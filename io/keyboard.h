#ifndef _IO_KEYBOARD_H
#define _IO_KEYBOARD_H
#define INPUT_MAXLEN 128
extern char in_buffer;	//Reserved in asm bootstrap, 129 bytes
char *con_input = &in_buffer;
volatile int bufpos = 0;
volatile bool request = false;
char *charbuffer = "a";
void kbd_init(void) {
	irq_map_handler(IRQ_KEYBOARD, (unsigned long)keyboard_handler);
	write_port(0x21, 0xFD);
	write_port(0xA1, 0xFF);
}

char* kbd_get_string(char* buf) {
	default_cursor();
	request = true;
	while(request)
		wait_int();
	strcpy(buf, con_input);
	clearchar(con_input);
	hidecur();
	return buf;
}

void keyboard_handler_main(void) {
	EOI();
	unsigned char status;
	char keycode;
	char actualkey;
	if (!request) return;
	status = read_port(KEYBOARD_STATUS_PORT);
		if (status & 0x01) {
			keycode = read_port(KEYBOARD_DATA_PORT);
			actualkey = keyboard_map[(unsigned char) keycode];
			if ((status & 0x20)) return;
			if (keycode < 0) return;
			else if(keycode == ENTER_KEY_CODE) {
				term_putchar('\n');
				request = false;
				bufpos = 0;
				return;
			} else if (actualkey == '\b'){
				size_t comlen = strlen(con_input);
				if (comlen > 0){
					con_input[comlen - 1] = '\0';
					bufpos++;
					term_putchar(actualkey);
				} else return;
			} else {
				actualkey = keyboard_map[(unsigned char)keycode];
				if (actualkey == '\0')
					return;
				if (bufpos >= INPUT_MAXLEN)
					return;
				bufpos++;
				memcpy(charbuffer, &actualkey, 1);
				con_input = strcat(con_input, charbuffer);
				term_putchar(actualkey);
			}
		}
		return;
}
#endif
