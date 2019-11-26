#ifndef _SCREEN_TERMINAL_H
#define _SCREEN_TERMINAL_H
#include "vga.h"
#include "ver.h"
#include "../io/idt.h"
#define COLOR_DEFAULT VGA_COLOR_LIGHT_GREY
#define STATUSBAR_BG VGA_COLOR_LIGHT_GREY
#define SCREEN_START 1
uint8_t input_counter = 0;
size_t term_row;
size_t term_column;
uint8_t term_color, term_lastcolor;
volatile uint16_t* term_buffer = (uint16_t*) 0xB8000;
size_t pmtlen = 1;
char* pmt = ">\0";

unsigned int BACKSPACE_OFFSET = 2;
unsigned int CURSOR_STEPBACK = 1;

static const size_t VGA_WIDTH = 90;
static const size_t VGA_HEIGHT = 30 - SCREEN_START;

void status_cls() {
	for (size_t x = 0; x < VGA_WIDTH; x++) {
		term_buffer[x] = vga_entry(' ', vga_entry_color(STATUSBAR_BG, STATUSBAR_BG));
	}
}

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
	term_row = SCREEN_START;
	term_column = 0;
	for (size_t y = SCREEN_START; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			term_buffer[index] = vga_entry(' ', term_color);
		}
	}
}

void term_scroll() {
    for(size_t ii = SCREEN_START; ii < VGA_HEIGHT; ii++){
        for (size_t i = 0; i < VGA_WIDTH; i++){
            term_buffer[ii * VGA_WIDTH + i] = term_buffer[(ii + 1) * VGA_WIDTH + i];
        }
    }
}

void term_initialize(void) {
	write_regs(g_90x30_text);
	write_font(g_8x16_font, 16);
	term_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	term_cls();
	status_cls();
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
	if (y != 0)
		term_movecur(x + 1, y);
}
void term_putchar(char c) {
	term_putentryat(c, term_color, term_column, term_row);
	if (++term_column == VGA_WIDTH) {
		term_column = 0;
		++term_row;
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

uint32_t lastcol, lastrow;

void draw(uint32_t x, uint32_t y, int color) {
	lastcol = term_column;
	lastrow = term_row;
	term_row = y;
	term_column = x;
	uint8_t tcolor = term_color;
	uint8_t dcol = vga_entry_color((uint8_t)color, (uint8_t)color);
	term_color = dcol;
	term_putchar(' ');
	term_color = tcolor;
	term_column = lastcol;
	term_row = lastrow;
}

void write_statusbar(uint32_t x) {
	lastcol = term_column;
	lastrow = term_row;
	term_row = 0;
	term_column = x;
}

void statusbar_rev() {
	term_row = lastrow;
	term_column = lastcol;
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
	write_statusbar(0);
	cprint(BUILD_STRING, vga_entry_color(VGA_COLOR_MAGENTA, STATUSBAR_BG));
	statusbar_rev();
}

#define CLOCK_OFFSET 67
extern int today();
void draw_clock() {
	write_statusbar(CLOCK_OFFSET);
	uint8_t color = vga_entry_color(VGA_COLOR_BLACK, STATUSBAR_BG);
	uint8_t tmp = term_color;
	term_color = color;
	today();
	term_color = tmp;
	statusbar_rev();
}

void term_set_title(char* str) {
	write_statusbar(VGA_WIDTH - strlen(str));
	uint8_t color = vga_entry_color(VGA_COLOR_BLACK, STATUSBAR_BG);
	uint8_t tmp = term_color;
	term_color = color;
	term_writestring(str);
	term_color = tmp;
	statusbar_rev();
}

void prompt() {
	term_writestring("\n");
	term_writestring(pmt);
}
#endif
