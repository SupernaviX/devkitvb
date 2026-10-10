#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <vb.h>

// Shorthand assembly marker
#define ASM __asm__ volatile



// ////////////////////////////// CPU Helpers ////////////////////////////// //

// Helper for CPU instruction CLI
__attribute__((always_inline)) inline void _cli() {
    ASM("cli");
}

// Helper for CPU instruction HALT
__attribute__((always_inline)) inline void _halt() {
    ASM("halt");
}

// Helper for CPU instruction LDSR
void _ldsr(int id, uint32_t value) {
    switch (id) {
        case VB_EIPC : ASM("ldsr %[value],  0" :: [value] "r" (value)); break;
        case VB_EIPSW: ASM("ldsr %[value],  1" :: [value] "r" (value)); break;
        case VB_FEPC : ASM("ldsr %[value],  2" :: [value] "r" (value)); break;
        case VB_FEPSW: ASM("ldsr %[value],  3" :: [value] "r" (value)); break;
        case VB_PSW  : ASM("ldsr %[value],  5" :: [value] "r" (value)); break;
        case VB_CHCW : ASM("ldsr %[value], 24" :: [value] "r" (value)); break;
        case VB_ADTRE: ASM("ldsr %[value], 25" :: [value] "r" (value)); break;
        case 29      : ASM("ldsr %[value], 29" :: [value] "r" (value)); break;
        case 31      : ASM("ldsr %[value], 31" :: [value] "r" (value)); break;
    }
}

// Helper for CPU instruction SEI
__attribute__((always_inline)) inline void _sei() {
    ASM("sei");
}

// Helper for CPU instruction STSR
uint32_t _stsr(int id) {
    uint32_t ret = 0;
    switch (id) {
        case VB_EIPC : ASM("stsr  0, %[ret]" : [ret] "=r" (ret)); break;
        case VB_EIPSW: ASM("stsr  1, %[ret]" : [ret] "=r" (ret)); break;
        case VB_FEPC : ASM("stsr  2, %[ret]" : [ret] "=r" (ret)); break;
        case VB_FEPSW: ASM("stsr  3, %[ret]" : [ret] "=r" (ret)); break;
        case VB_ECR  : ASM("stsr  4, %[ret]" : [ret] "=r" (ret)); break;
        case VB_PSW  : ASM("stsr  5, %[ret]" : [ret] "=r" (ret)); break;
        case VB_PIR  : ASM("stsr  6, %[ret]" : [ret] "=r" (ret)); break;
        case VB_TKCW : ASM("stsr  7, %[ret]" : [ret] "=r" (ret)); break;
        case VB_CHCW : ASM("stsr 24, %[ret]" : [ret] "=r" (ret)); break;
        case VB_ADTRE: ASM("stsr 25, %[ret]" : [ret] "=r" (ret)); break;
        case 29      : ASM("stsr 29, %[ret]" : [ret] "=r" (ret)); break;
        case 30      : ASM("stsr 30, %[ret]" : [ret] "=r" (ret)); break;
        case 31      : ASM("stsr 31, %[ret]" : [ret] "=r" (ret)); break;
    }
    return ret;
}



// ///////////////////////////// API Commands ////////////////////////////// //

// Initialize the column table with the stock reference template
void vbInitColumnTable() {

    // Stock reference values
    static const uint8_t COLUMNS[] = {
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xE0, 0xBC,
        0xA6, 0x96, 0x8A, 0x82, 0x7A, 0x74, 0x6E, 0x6A,
        0x66, 0x62, 0x60, 0x5C, 0x5A, 0x58, 0x56, 0x54,
        0x52, 0x50, 0x50, 0x4E, 0x4C, 0x4C, 0x4A, 0x4A,
        0x48, 0x48, 0x46, 0x46, 0x46, 0x44, 0x44, 0x44,
        0x42, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40, 0x40,
        0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3C,
        0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
        0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C
    };

    // Store the template across the entire column table
    for (int x = 0; x < 128; x++) {
        *(uint16_t *) &VB_COLSL[      x] =
        *(uint16_t *) &VB_COLSR[      x] =
        *(uint16_t *) &VB_COLSL[255 - x] =
        *(uint16_t *) &VB_COLSR[255 - x] = COLUMNS[x];
    }

}

// Read the current state of the game pad
uint16_t vbReadGamePad() {
    VB_SCR = VB_PARASI;
    for (int x = 16; x > 0; x--) {
        ASM("rev r0, r0");
        VB_SCR = VB_SOFTCK;
        VB_SCR = 0;
    }
    return VB_SDHR << 8 | VB_SDLR;
}



// /////////////////////////////// C Runtime /////////////////////////////// //

// Read an input byte from the environment
static int getc_terminal(FILE *file) {
    (void) file;
    volatile char *IN = (volatile char *)0x0200002c;
    int c = *IN;
    return (int) c;
}

// Write an output byte to the environment
static int putc_terminal(char c, FILE *file) {
    (void) file;
    volatile char *OUT = (volatile char *)0x02000030;
    *OUT = c;
    return c;
}

// Establish stdio streams
static FILE __stdio = FDEV_SETUP_STREAM(
    putc_terminal, getc_terminal, NULL, _FDEV_SETUP_RW);
FILE *const stdin  = &__stdio;
__strong_reference(stdin, stdout);
__strong_reference(stdin, stderr);

// Allocate heap
#ifndef __VB_HEAP_SIZE
#define __VB_HEAP_SIZE 0
#endif
#define STR(s) #s
#define XSTR(s) STR(s)
__asm__(
    ".balign 4\n"
    ".global __heap_start\n"
    ".fill " XSTR(__VB_HEAP_SIZE) "\n"
    ".global __heap_end"
);

// Shim for libc
void _exit(int status) {
    (void) status;
    ASM("sei\nhalt");
}
