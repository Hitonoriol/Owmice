/*
	This is an example of Owmice C program
	Notice that origin point must be at 0x100060
	because I'm a lazy fuck and it's just fun and easy, you know
*/

#include <stddef.h>
#include <stdint.h>

#include "../standard/owapi.inc"
#include "../standard/stdio.inc"

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
