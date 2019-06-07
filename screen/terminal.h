#ifndef _SCREEN_TERMINAL_H
#define _SCREEN_TERMINAL_H
#include "vga.h"
#include "../io/idt.h"
#define COLOR_DEFAULT VGA_COLOR_LIGHT_GREY
uint8_t input_counter = 0;
size_t term_row;
size_t term_column;
uint8_t term_color, term_lastcolor;
volatile uint16_t* term_buffer = (uint16_t*) 0xB8000;
size_t pmtlen = 1;
char* pmt = ">\0";

unsigned int BACKSPACE_OFFSET = 2;
unsigned int CURSOR_STEPBACK = 1;

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

void setcolor(uint8_t color) {
	term_color = color;
}

void term_tempcolor(uint8_t color) {
	term_lastcolor = term_color;
	setcolor(color);
}

void term_revertcolor() {
	setcolor(term_lastcolor);
}

void setprompt(char* p){
	justincase(p);
	pmtlen = strlen(p);
	strcpy(pmt, p);
}

void showcur(uint8_t cursor_start, uint8_t cursor_end) {
	write_port(0x3D4, 0x0A);
	write_port(0x3D5, (read_port(0x3D5) & 0xC0) | cursor_start);
	write_port(0x3D4, 0x0B);
	write_port(0x3D5, (read_port(0x3D5) & 0xE0) | cursor_end);
}

void default_cursor(){
	showcur(0, 15);
}

void hidecur() {
	write_port(0x3D4, 0x0A);
	write_port(0x3D5, 0x20);
}

void term_movecur(int x, int y) {
	uint16_t pos = y * VGA_WIDTH + x;
 
	write_port(0x3D4, 0x0F);
	write_port(0x3D5, (uint8_t) (pos & 0xFF));
	write_port(0x3D4, 0x0E);
	write_port(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void term_undo_nl() {
	term_row -= 1;
	term_column = 0;
	term_movecur(term_row, term_column);
}

uint16_t getcurpos(void) {
    uint16_t pos = 0;
    write_port(0x3D4, 0x0F);
    pos |= read_port(0x3D5);
    write_port(0x3D4, 0x0E);
    pos |= ((uint16_t)read_port(0x3D5)) << 8;
    return pos;
}

void term_cls() {
	term_row = 0;
	term_column = 0;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			term_buffer[index] = vga_entry(' ', term_color);
		}
	}
}

void term_scroll() {
    for(size_t ii = 0; ii < VGA_HEIGHT; ii++){
        for (size_t i = 0; i < VGA_WIDTH; i++){
            term_buffer[ii * VGA_WIDTH + i] = term_buffer[(ii + 1) * VGA_WIDTH + i];
        }
    }
}

void term_initialize(void) {
	term_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	term_cls();
}
void term_putentryat(char c, uint8_t color, size_t x, size_t y) {
	if (c == '\n'){
		term_row++;
		term_column = -CURSOR_STEPBACK;
		return;
	}
	if (c == '\b') {
		if (term_column > pmtlen){
			term_column -= BACKSPACE_OFFSET;
			x -= CURSOR_STEPBACK;
			c = '\0';
		} else return;
	}

	const size_t index = y * VGA_WIDTH + x;
	term_buffer[index] = vga_entry(c, color);
	if (c == '\0')
		x--;
	term_movecur(x + 1, y);
}
void term_putchar(char c) {
	term_putentryat(c, term_color, term_column, term_row);
	if (++term_column == VGA_WIDTH) {
		term_column = 0;
		if (++term_row == VGA_HEIGHT)
			term_row = 0;
	}
	if (term_row > (VGA_HEIGHT - 1)) {
		term_row--;
		term_scroll();
	}
}
void term_write(char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		term_putchar(data[i]);
}
void term_writestring(char* data) {
	term_write(data, strlen(data));
}

void kprint (char* s) {
	term_writestring(s);
	term_writestring(&linebreak);
}

void cprint (char* s, uint8_t newc) {
	uint8_t last = term_color;
	term_color = newc;
	term_writestring(s);
	term_writestring(&linebreak);
	term_color = last;
}

void console_init() {
	term_initialize();
	cprint("Owmice [v]\n", VGA_COLOR_MAGENTA);
}

void prompt() {
	term_writestring("\n");
	term_writestring(pmt);
}
#endif
