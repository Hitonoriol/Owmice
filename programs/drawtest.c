#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "../standard/colors.inc"
#include "../standard/owapi.inc"

int main() {
	owmice_draw(10, 10, VGA_COLOR_MAGENTA);
	return 0;
}

#include "../standard/owapi.h"
