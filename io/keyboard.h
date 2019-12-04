#ifndef _IO_KEYBOARD_H
#define _IO_KEYBOARD_H
#define INPUT_MAXLEN 128
#define KBD_STRING 1
#define KBD_CHAR 2
extern char in_buffer;	//Reserved in asm bootstrap, 129 bytes
char *con_input = &in_buffer;
volatile int bufpos = 0;
volatile uint8_t request = 0;
char *charbuffer = "a";
char keychar;
char keycode;

volatile bool shift_down = false;

void kbd_init(void) {
	printf("Setting up keyboard driver... ");
	irq_map_handler(IRQ_KEYBOARD, (unsigned long)keyboard_handler);
	write_port(0x21, 0xFD);
	write_port(0xA1, 0xFF);
	printf("Done!\n");
}

char* kbd_get_string(char* buf) {
	if (request)
		return NULL;
	default_cursor();
	request = KBD_STRING;
	while(request)
		wait_int();
	strcpy(buf, con_input);
	clearchar(con_input);
	hidecur();
	return buf;
}

char kbd_get_char() {
	if (request)
		return 0;
	hidecur();
	request = KBD_CHAR;
	while(request)
		wait_int();
	return keychar;
}

char kbd_current_char() {
	return keychar;
}

void keyboard_handler_main(void) {
	EOI();
	unsigned char status;
	status = read_port(KEYBOARD_STATUS_PORT);
		if (status & 0x01) {
			keycode = read_port(KEYBOARD_DATA_PORT);
			keychar = keyboard_map[(unsigned char) keycode];
			if (keychar == '\b'){
					size_t comlen = strlen(con_input);
					if (comlen > 0){
						con_input[comlen - 1] = '\0';
						bufpos++;
						term_putchar(keychar);
					}
					return;
			}
			if ((keycode & 0x80)) {	//key up
				if (keychar == KEY_SHIFT)
					shift_down = false;
				keychar = 0;
				return;
			} else {	//key down
				if (keychar == KEY_SHIFT) {
					shift_down = true;
					return;
				}
				else if (keychar == KEY_UP ||
					keychar == KEY_DOWN ||
					keychar == KEY_LEFT ||
					keychar == KEY_RIGHT)
					return;
				else if(keycode == ENTER_KEY_CODE) {
					term_putchar('\n');
					request = 0;
					bufpos = 0;
					return;
				} else {
					keychar = keyboard_map[(unsigned char)keycode];
					if (keychar == '\0')
						return;
					if (bufpos >= INPUT_MAXLEN)
						return;
					bufpos++;
					if (request == KBD_CHAR) {
						request = 0;
						bufpos = 0;
						return;
					}
					memcpy(charbuffer, &keychar, 1);
					con_input = strcat(con_input, charbuffer);
					term_putchar(keychar);
				}
			}
		}
		return;
}
#endif
