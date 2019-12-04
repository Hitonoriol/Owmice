#include <stddef.h>
#include <stdint.h>

#include "../standard/owapi.inc"
#include "../standard/colors.inc"
#include "../standard/stdio.inc"

int main() {
	char kb;
	int x = 10, y = 10;
	srand(owmice_now());
	uint8_t color = rand_range(0, 15);
	owmice_cls();
	printf("<arrows> move\n<enter> quit\n<space> change color");
	owmice_sleep(1000);
	while (kb != '\n') {
		kb = owmice_kbd_current_char();
		owmice_draw(x, y, color);
		if (kb != 0) {
			switch(kb) {
				case KEY_UP:
					--y;
					break;
				case KEY_DOWN:
					++y;
					break;
				case KEY_LEFT:
					--x;
					break;
				case KEY_RIGHT:
					++x;
					break;
				case ' ':
					color = rand_range(0, 15);
					break;
				default:
					break;
			}
		}
		owmice_sleep(80);
	}
	owmice_cls();
	owmice_status_cls();
	return 0;
}

#include "../standard/stdio.h"
#include "../standard/owapi.h"
