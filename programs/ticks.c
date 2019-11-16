#include <stddef.h>
#include <stdint.h>

extern int printf(char *fmt, ...);
extern unsigned long long owmice_get_ticks(void);

int main() {
	printf("Ticks passed: %u", owmice_get_ticks());
	return (int)0xA55;
}

#include "../standard/owapi.h"
#include "../standard/stdio.h"
