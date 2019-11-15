#include <stddef.h>
#include <stdint.h>

extern void owmice_writestring(char*);
extern int printf(char *fmt, ...);

int main(char* arg) {
	owmice_writestring("Hello from test C binary!\n");
	if (arg == NULL)
		printf("No arguments today? Lame!\n");
	else
		printf("Here's your argument: 0x%X -> %s\n", (uint32_t)arg, arg);
	return 0;
}

#include "../standard/stdio.h"
#include "../standard/owapi.h"
