#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "../standard/colors.inc"
#include "../standard/owapi.inc"
#include "../standard/stdio.inc"

int main() {
	owmice_disable_scroll();
	owmice_cls();

	for (int i = 0; i < 90; ++i) {
		for(int j = 0; j < 30; ++j) {
			owmice_draw(i, j, rand_range(0, 15));
		}
	}

	owmice_get_char();
	owmice_enable_scroll();
	owmice_cls();
	return 0;
}

#include "../standard/owapi.h"
#include "../standard/stdio.h"
