#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



////////////////////////////////// Constants //////////////////////////////////

// Token matching rules
#define EQUALS      0
#define STARTS_WITH 1

// Option properties
#define DIALECT          1
#define HEAP_SIZE        2
#define NO_CRT0          3
#define NO_LTO           4
#define NO_STDINC        5
#define NO_STDLIB        6
#define OPTIMIZATION     7
#define OUTPUT           8
#define PREVIEW          9
#define PROHIBITED      10
#define STOP_ASSEMBLY   11
#define STOP_LINK       12
#define STOP_OBJECT     13
#define STOP_PREPROCESS 14



//////////////////////////////////// Types ////////////////////////////////////

// Argument list
typedef struct {
    char   *buffer;   // Allocated memory buffer
    size_t  capacity; // Size of list before a reallocation is needed
    size_t  length;   // Number of arguments in the list
    size_t *lengths;  // List of argument string lengths
    size_t  offset;   // Offset of next argument string in memory buffer
    size_t *offsets;  // List of argument string offsets in memory buffer
    size_t  size;     // Size of allocated memory buffer
} ArgList;

// Option definition
typedef struct {
    char   *text;       // Token template text
    uint8_t textLength; // Number of bytes in text
    uint8_t rule;       // Token matching rule
    uint8_t args;       // Number of additional arguments
    uint8_t property;   // Ancillary property
    uint8_t isCustom;   // Nonstandard option
} Option;



///////////////////////////// Platform Functions //////////////////////////////

// Resolve the parent directory path, including the trailing slash
static char* osParentDir(char *path, size_t length, int backslash) {

    // Locate the second-to-last path separator
    for (int remain = 2; length > 0; length--) {
        char c = path[length - 1];

        // Not a path separator
        if (c != '/' && (!backslash || c != '\\'))
            continue;

        // Advance to next path separator
        remain--;
        if (remain == 0)
            break;
    }

    // Fewer than 2 path separators were found
    if (length == 0)
        return NULL;

    // Truncate the string to the final trailing slash
    char *ret = malloc(length + 1);
    memcpy(ret, path, length);
    ret[length] = 0;
    return ret;
}

// Linux specific
#ifdef __linux__

#include <linux/limits.h>
#include <sys/wait.h>
#include <unistd.h>

// Determine the absolute path of the devkit base directory
static char* osBasePath() {
    char path[PATH_MAX + 1];

    // Read the self symbolic link
    ssize_t length = readlink("/proc/self/exe", path, sizeof path - 1);

    // Unable to read link
    if (length == -1)
        return NULL;

    // Resolve the parent directory
    path[length] = 0;
    return osParentDir(path, length, 0);
}

#endif // __linux__

#ifdef __APPLE__
#include <libproc.h>
#include <unistd.h>

// Determine the absolute path of the devkit base directory
static char* osBasePath() {
    char path[PROC_PIDPATHINFO_MAXSIZE];

    // Read the path from libproc
    pid_t pid = getpid();
    int length = proc_pidpath(pid, path, sizeof(path));

    // Unable to read path
    if (!length)
        return NULL;

    // Resolve the parent directory
    return osParentDir(path, length, 0);
}


#endif // __APPLE__

// Windows specific
#ifdef _WIN32

#include <windows.h>

// Determine the absolute path of the devkit base directory
static char* osBasePath() {
    char path[MAX_PATH + 1];

    // Retrieve the path of the current executable
    DWORD length = GetModuleFileNameA(NULL, path, sizeof path - 1);

    // Unable to read link
    if (GetLastError() != ERROR_SUCCESS || length >= sizeof path - 1)
        return NULL;

    // Resolve the parent directory
    path[length] = 0;
    return osParentDir(path, length, 1);
}

// On Windows, system() returns the exit code directly
#define osExec system

#endif

// Non-Windows specific
#ifndef _WIN32

// Execute a shell command and return its exit code
static int osExec(char *cmd) {
    int ret = system(cmd);
    return ret == -1 || !WIFEXITED(ret) ? -1 : WEXITSTATUS(ret);
}

#endif



/////////////////////////////// ArgList Methods ///////////////////////////////

// Delete resources used for an argument list
static void alDispose(ArgList *list) {
    free(list->buffer);
    free(list->lengths);
    free(list->offsets);
}

// Retrieve an argument from a list
static char* alGet(ArgList *list, size_t index) {
    return index >= list->size ? NULL :
        &list->buffer[list->offsets[index]];
}

// Initialize an argument list
static void alInit(ArgList *list) {
    list->capacity = 1;
    list->size     = 256;
    list->buffer   = malloc(list->size);
    list->length   = 0;
    list->lengths  = malloc(list->capacity * sizeof (size_t));
    list->offset   = 0;
    list->offsets  = malloc(list->capacity * sizeof (size_t));
}

// Escape and add an argument to the end of a list
static void alAppend(ArgList *list, char *arg, size_t argLength) {
    size_t addSize = 2 + argLength;

    // Count how many characters to escape
    for (size_t x = 0; x < argLength; x++) {
        char c = arg[x];
        addSize += c == '"' || c == '\\';
    }

    // Ensure there is size in the buffer for the new argument
    size_t newSize = list->offset + addSize + 1;
    if (newSize > list->size) {
        while (newSize > list->size)
            list->size *= 2;
        list->buffer = realloc(list->buffer, list->size);
    }

    // Encode the new argument, applying escapes where necessary
    char *newItem = &list->buffer[list->offset];
    newItem[          0] = '"';
    newItem[addSize - 1] = '"';
    newItem[addSize    ] = 0;
    for (size_t src = 0, dest = 1; src < argLength; src++) {
        char c = arg[src];
        if (c == '"' || c == '\\')
            newItem[dest++] = '\\';
        newItem[dest++] = c;
    }

    // Ensure there is size in the list for the new entry
    if (list->length == list->capacity) {
        size_t size     = list->capacity * 2 * sizeof (size_t);
        list->capacity *= 2;
        list->lengths   = realloc(list->lengths, size);
        list->offsets   = realloc(list->offsets, size);
    }

    // Add the new entry
    list->lengths[list->length] = addSize;
    list->offsets[list->length] = list->offset;
    list->offset               += addSize + 1;
    list->length++;
}

// Escape and add an argument with a prefix, path and suffix
static void alAppendCombo(ArgList *list, char *prefix, size_t prefixLength,
    char *path, size_t pathLength, char *suffix, size_t suffixLength) {
    size_t strLength  = prefixLength + pathLength + suffixLength;

    // Construct the combo argument string
    char *str = malloc(strLength + 1);
    memcpy(&str[0], prefix, prefixLength);
    memcpy(&str[prefixLength], path, pathLength);
    memcpy(&str[prefixLength + pathLength], suffix, suffixLength);
    str[strLength] = 0;

    // Append the argument
    alAppend(list, str, strLength);
    free(str);
}

// Append all arguments from another list to the end of this one
static void alConcat(ArgList *dest, ArgList *src) {

    // Process all arguments
    for (size_t x = 0; x < src->length; x++) {
        char  *arg       = alGet(src, x);
        size_t argLength = src->lengths[x];

        // Ensure there is size in the buffer for the new argument
        size_t newSize = dest->size + argLength + 1; // Includes terminator
        if (dest->size < newSize) {
            while (dest->size < newSize)
                dest->size *= 2;
            dest->buffer = realloc(dest->buffer, dest->size);
        }

        // Copy the argument text
        memcpy(&dest->buffer[dest->offset], arg, argLength + 1);

        // Ensure there is size in the list for the new entry
        if (dest->length == dest->capacity) {
            size_t size     = dest->capacity * 2 * sizeof (size_t);
            dest->capacity *= 2;
            dest->lengths   = realloc(dest->lengths, size);
            dest->offsets   = realloc(dest->offsets, size);
        }

        // Add the new entry
        dest->lengths[dest->length] = argLength;
        dest->offsets[dest->length] = dest->offset;
        dest->offset               += argLength + 1;
        dest->length++;
    }

}

// Format the argument list as a command
static char* alToCommand(ArgList *list,
    char *base, size_t baseLength, char *exec, size_t execLength) {

    // Determine the total number of bytes in the command
    size_t cmdLength = baseLength + execLength + 1; // Includes terminator
    for (size_t x = 0; x < list->length; x++)
        cmdLength += 1 + list->lengths[x]; // Space and argument

    // Allocate string memory and copy executable name
    char *ret = malloc(cmdLength);
    memcpy(&ret[0         ], base, baseLength);
    memcpy(&ret[baseLength], exec, execLength);

    // Output all arguments in the list
    size_t dest = baseLength + execLength;
    for (size_t x = 0; x < list->length; x++) {
        char  *arg       = alGet(list, x);
        size_t argLength = list->lengths[x];
        ret[dest++] = ' ';
        memcpy(&ret[dest], arg, argLength);
        dest += argLength;
    }

    // Finalize with a null terminator
    ret[dest] = 0;

    return ret;
}



////////////////////////////// Option Functions ///////////////////////////////

// Options
#include "options.c"

// Perform a binary search on the "equals" option lookup data
static Option* searchEquals(char *arg) {

    // Search bounds */
    int max = sizeof OPTIONS_EQUALS / sizeof (Option) - 1;
    int min = 0;

    // Search for the specified option
    for (;;) {
        int     cmp;   // Result of comparison
        Option *entry; // Current lookup entry
        int     mid;   // Index of current lookup

        // Select and compare with the entry half-way between min and max
        mid   = (max + min) >> 1;
        entry = (Option *) &OPTIONS_EQUALS[mid];
        cmp   = strcmp(arg, entry->text);

        // A match was found
        if (cmp == 0)
            return entry;

        // There are no more matches to be found
        if (min == max)
            break;

        // Eliminate half of the remaining search space
        if (cmp < 0)
            max = mid;
        else if (min == mid)
            min = max;
        else min = mid;
    }

    // No match was found
    return NULL;
}

// Perform a reverse linear search on the "starts with" option lookup data
static Option* searchStartsWith(char *arg) {
    for (
        int x = sizeof OPTIONS_STARTS_WITH / sizeof (Option) - 1;
        x >= 0;
        x--
    ) {
        Option *entry = (Option *) &OPTIONS_STARTS_WITH[x];
        if (strncmp(arg, entry->text, entry->textLength) == 0)
            return entry;
    }
    return NULL;
}

// Identify an option that matches a command line argument
static Option* findOption(char *arg) {
    Option *entry = searchEquals(arg);
    return entry != NULL ? entry : searchStartsWith(arg);
}



///////////////////////////// Filename Functions //////////////////////////////

/*
  Filename extensions checked by clang:
  C                             .c .h
  C++                           .cpp .cc .cxx .C .hpp
  Preprocessed C                .i
  Preprocessed C++              .ii
  Objective-C                   .m
  Objective-C++                 .mm
  Assembly                      .s
  Assembly with C preprocessor  .S
  Object                        .o .obj
  Library                       .a .so
*/

// Test if one string ends with another
static int endsWith(char *s, size_t sLength, char *t, size_t tLength) {
    return
        sLength >= tLength &&
        strcmp(&s[sLength - tLength], t) == 0
    ;
}

// Test whether an input file is compiled
static int isCompiledFile(char *arg, size_t length) {
    return
        endsWith(arg, length, ".c"  , 2) ||
        endsWith(arg, length, ".cpp", 4) ||
        endsWith(arg, length, ".cc" , 3) ||
        endsWith(arg, length, ".cxx", 4) ||
        endsWith(arg, length, ".C"  , 2) ||
        endsWith(arg, length, ".i"  , 2) ||
        endsWith(arg, length, ".ii" , 3) ||
        endsWith(arg, length, ".m"  , 2) ||
        endsWith(arg, length, ".mm" , 3)
    ;
}



////////////////////////////// Program Functions //////////////////////////////

// Primary processing of command driver
static int mainInternal(int argc, char **argv, char *base, size_t baseLength,
    ArgList *argsFiles, ArgList *argsOptions) {
    ArgList argsCmd;               // Command string argument list
    int     dashDash       = 0;    // Operating in remaining-arguments mode
    int     dialect        = 0;    // Specifying a C dialect
    char   *cmd;                   // Command string
    int     isCompiled     = 0;    // Contains a file to be compiled
    int     isLinked       = 0;    // Performing the linker step
    int     isObjCopying   = 0;    // Performing the objcopy step
    char   *heapSize       = NULL; // libc heap size value
    int     noCrt0         = 0;    // Not using provided crt0 and vb.ld
    int     noLTO          = 0;    // -fno-lto is present
    int     noStdInc       = 0;    // Not using standard includes
    int     noStdLib       = 0;    // Not using standard library
    int     optimization   = 0;    // An optimization option is present
    char   *outFilename    = NULL; // Output filename
    int     preview        = 0;    // Preview commands, but don't invoke them
    int     stopAssembly   = 0;    // -S is present
    int     stopBeforeLink = 0;    // Will stop before the linker step
    int     stopLink       = 0;    // -vb-no-rom is present
    int     stopObject     = 0;    // -c is present
    int     stopPreprocess = 0;    // -E is present

    // Initialize argument lists
    alInit(argsFiles);
    alInit(argsOptions);

    // Process all command line arguments
    for (int x = 1; x < argc; x++) {
        char  *arg       = argv[x];
        size_t argLength = strlen(arg);

        // Empty argument (shouldn't happen in a terminal)
        if (argLength == 0)
            continue;

        // Operating in remaining-arguments mode
        if (dashDash) {
            #ifdef DEBUG
            printf("File   %s\n", arg);
            #endif
            alAppend(argsFiles, arg, argLength);
            if (isCompiledFile(arg, argLength))
                isCompiled = 1;
            continue;
        }

        // Enter clang remaining-arguments mode
        if (strcmp(arg, "--") == 0) {
            dashDash = 1;
            continue;
        }

        // Find an option that matches the current token
        Option *entry = findOption(arg);

        // No option matched, may be a filename
        if (entry == NULL) {

            // Token is not a filename
            if (arg[0] == '-') {
                fprintf(stderr, "vbcc Error: Unsupported option %s\n", arg);
                return 1;
            }

            // Token is a filename
            #ifdef DEBUG
            printf("File   %s\n", arg);
            #endif
            alAppend(argsFiles, arg, argLength);
            if (isCompiledFile(arg, argLength))
                isCompiled = 1;
            continue;
        }

        // There aren't enough remaining arguments
        if (x + 1 + entry->args > argc) {
            fprintf(stderr, "Insufficient arguments for option %s\n", arg);
            return 1;
        }

        // Processing by option property
        switch (entry->property) {

            // Specifying heap size
            case HEAP_SIZE:
                heapSize = argv[++x];
                #ifdef DEBUG
                printf("Option -vb-heap-size %s\n", heapSize);
                #endif
                continue;

            // Specifying output filename
            case OUTPUT:
                outFilename = entry->rule == STARTS_WITH ?
                    &arg[2] : argv[++x];
                #ifdef DEBUG
                printf("Option -o%s%s\n",
                    entry->rule == STARTS_WITH ? "" : " ", outFilename);
                #endif
                continue;

            // Option is explicitly prohibited
            case PROHIBITED:
                fprintf(stderr, "vbcc Error: Unsupported option %s\n", arg);
                return 1;

            // Operational flags
            case DIALECT        : dialect        = 1; break;
            case NO_CRT0        : noCrt0         = 1; break;
            case NO_LTO         : noLTO          = 1; break;
            case NO_STDINC      : noStdInc       = 1; break;
            case NO_STDLIB      : noStdLib       = 1; break;
            case OPTIMIZATION   : optimization   = 1; break;
            case PREVIEW        : preview        = 1; break;
            case STOP_ASSEMBLY  : stopAssembly   = 1; break;
            case STOP_LINK      : stopLink       = 1; break;
            case STOP_OBJECT    : stopObject     = 1; break;
            case STOP_PREPROCESS: stopPreprocess = 1; break;
        }

        // Don't log custom options
        if (entry->isCustom) {
            x += entry->args;
            continue;
        }

        // Token is an option: log it and all of its arguments
        #ifdef DEBUG
        printf("Option %s", arg);
        #endif
        alAppend(argsOptions, arg, argLength);
        for (int y = 0; y < entry->args; y++) {
            arg = argv[++x];
            #ifdef DEBUG
            printf(" %s", arg);
            #endif
            alAppend(argsOptions, arg, strlen(arg));
        }
        #ifdef DEBUG
        printf("\n");
        #endif
    }

    // Debugging output
    #ifdef DEBUG
    printf("\n");

    printf("Options: %d\n", argsOptions->length);
    for (int x = 0; x < argsOptions->length; x++)
        printf("  %s\n", alGet(argsOptions, x));

    printf("Source files: %d\n", argsFiles->length);
    for (int x = 0; x < argsFiles->length; x++)
        printf("  %s\n", alGet(argsFiles, x));

    if (outFilename == NULL)
        printf("Output file is not specified\n");
    else printf("Output file:\n  \"%s\"\n", outFilename);

    printf("\n");
    #endif

    // No input files
    if (argsFiles->length == 0) {
        fprintf(stderr, "vbcc Error: No input files.\n");
        return 1;
    }

    // Enable only the earliest stop flag
    if (stopPreprocess)
        stopAssembly   = stopObject   = stopLink   = 0;
    else if (stopAssembly)
        stopPreprocess = stopObject   = stopLink   = 0;
    else if (stopObject)
        stopPreprocess = stopAssembly = stopLink   = 0;
    else if (stopLink)
        stopPreprocess = stopAssembly = stopObject = 0;
    stopBeforeLink = stopPreprocess || stopAssembly || stopObject;

    // Incompatible flag combination
    if (stopBeforeLink && outFilename != NULL && argsFiles->length > 1) {
        fprintf(stderr, "vbcc Error: "
            "Cannot use -o with -c, -E, or -S and multiple files.\n");
        return 1;
    }

    // Performing the link step
    if (!stopBeforeLink) {
        isCompiled   = !noStdLib; // Will append vb.c
        isLinked     = 1;
        isObjCopying = !stopLink;
        if (outFilename == NULL)
            outFilename = isObjCopying ? "a.vb" : "a.out";
    }

    // Compose v810-clang command
    alInit(&argsCmd);

    // Prepend implicit options
    alAppend(&argsCmd, "--target=v810-unknown-vb", 24);
    if (isCompiled) {
        alAppend(&argsCmd, "-ffreestanding" , 14);
        alAppend(&argsCmd, "-fno-exceptions", 15); // C++
        alAppend(&argsCmd, "-fno-rtti"      ,  9); // C++
        alAppend(&argsCmd, "-mgprel"        ,  7);
        if (!dialect)
            alAppend(&argsCmd, "-std=c23", 8);
        if (!optimization)
            alAppend(&argsCmd, "-O2"     , 3);
    }

    // User options
    alConcat(&argsCmd, argsOptions);

    // Append implicit options
    if (!noStdInc)
        alAppendCombo(&argsCmd, "-I",2, base,baseLength, "lib/vb/include",14);
    if (!noLTO && (isCompiled || isLinked))
        alAppend(&argsCmd, "-flto", 5);
    if (isLinked) {
        if (noStdLib)
            alAppend(&argsCmd, "-nostdlib", 9);
        else {
            alAppendCombo(&argsCmd, "-L", 2, base, baseLength, "lib/vb", 6);
            alAppend     (&argsCmd, "-lc", 3);
        }
        if (!noStdLib && heapSize != NULL) {
            alAppendCombo(&argsCmd, "-D__VB_HEAP_SIZE=", 17,
                heapSize, strlen(heapSize), "", 0);
        }
        if (!noCrt0) {
            alAppendCombo(&argsCmd,
                "--ld-path=", 10, base, baseLength, "clang/bin/ld.lld", 16);
            alAppendCombo(&argsCmd,
                "-T"        ,  2, base, baseLength, "lib/vb/vb.ld", 12);
        }
    }

    // Output filename
    if (outFilename != NULL) {
        alAppend(&argsCmd, "-o", 2);
        alAppend(&argsCmd, outFilename, strlen(outFilename));
    }

    // Begin files
    alAppend(&argsCmd, "--", 2);

    // User files
    alConcat(&argsCmd, argsFiles);

    // Append implicit files
    if (isLinked) {
        if (!noStdLib)
            alAppendCombo(&argsCmd, "",0, base,baseLength, "lib/vb/vb.c"  ,11);
        if (!noCrt0)
            alAppendCombo(&argsCmd, "",0, base,baseLength, "lib/vb/crt0.s",13);
    }

    // Invoke command
    cmd = alToCommand(&argsCmd, base, baseLength, "clang/bin/v810-clang", 20);
    alDispose(&argsCmd);
    if (preview) {
        printf("%s\n", cmd);
        free(cmd);
    } else {
        int exitCode = osExec(cmd);
        free(cmd);
        if (exitCode != 0)
            return exitCode;
    }

    // Stopping after the linker step
    if (!isObjCopying)
        return 0;

    // llvm-objcopy command invocation
    alInit(&argsCmd);
    alAppend(&argsCmd, "-S"    , 2);
    alAppend(&argsCmd, "-O"    , 2);
    alAppend(&argsCmd, "binary", 6);
    alAppend(&argsCmd, outFilename, strlen(outFilename));
    cmd = alToCommand(&argsCmd, base, baseLength, "clang/bin/llvm-objcopy", 22);
    alDispose(&argsCmd);
    if (preview) {
        printf("%s\n", cmd);
        free(cmd);
    } else {
        int exitCode = osExec(cmd);
        free(cmd);
        if (exitCode != 0)
            return exitCode;
    }

    return 0;
}

// Entry point
int main(int argc, char **argv) {

    // Check for shell support
    if (system(NULL) == 0) {
        fprintf(stderr, "Error: No shell available.\n");
        return 1;
    }

    // Resolve the absolute base path for the toolkit
    char *base = osBasePath();
    if (base == NULL) {
        fprintf(stderr, "Error: Could not resolve absolute path of binary.\n");
        return 1;
    }

    // Process the command line
    ArgList argsFiles, argsOptions;
    int ret = mainInternal(argc, argv, base, strlen(base),
        &argsFiles, &argsOptions);
    alDispose(&argsFiles);
    alDispose(&argsOptions);
    free(base);
    return ret;
}
