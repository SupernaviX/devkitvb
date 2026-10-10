CFLAGS := -Wall -Wextra -Werror -O2 -flto

.PHONY: vbcc
vbcc: options.c
	$(CC) $(CFLAGS) main.c -o vbcc

.PHONY: windows
windows: options.c
	@x86_64-w64-mingw32-gcc -Wall -Wextra -Werror main.c -O2 -flto -s -o vbcc

.PHONY: options.c
options.c:
	@deno --allow-read --allow-write options.js

.PHONY: clean
clean:
	@rm -f vbcc options.c

.PHONY: debug
debug: options.c
	@$(CC) $(CFLAGS) -DDEBUG main.c -o vbcc
