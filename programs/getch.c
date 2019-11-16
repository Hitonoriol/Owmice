#include <stddef.h>
#include <stdint.h>

#include "../standard/owapi.inc"
#include "../standard/stdio.inc"

int main() {
	printf("Press any key...\n");
	char kb = owmice_get_char();
	printf("Pressed keychar: [%u | %c]\n", (uint32_t)kb, kb);
	return 0;
}

#include "../standard/stdio.h"
#include "../standard/owapi.h"
