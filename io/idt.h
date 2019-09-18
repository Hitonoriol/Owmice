#ifndef _IO_IDT_H
#define _IO_IDT_H

#define IRQ_TIMER 0x20
#define IRQ_KEYBOARD 0x21

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#define ENTER_KEY_CODE 0x1C
extern void create_gdt(void);
extern char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

void cli(void) { asm volatile("cli"); }
void sti(void) { asm volatile ("sti"); }

uint16_t read_port16(uint16_t port) {
	uint16_t ret;
	asm volatile("inw %1, %0": "=a" (ret) : "dN" (port));
	return ret;
}

void port_write16(uint16_t port, uint16_t value) {
	asm volatile("outw %1, %0" : : "dN" (port), "a" (value));
}

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void EOI() {
	write_port(0x20, 0x20);
	write_port(0xA0, 0x20);
}

void wait_int() {
	asm("hlt");
}

void int_off() {
	write_port(0x21, 0xFF);
}

void irq_map_handler(uint8_t num, unsigned long base) {
	IDT[num].offset_lowerbits = (base & 0xFFFF);
	IDT[num].offset_higherbits = (base & 0xffff0000) >> 16;    
	IDT[num].selector = KERNEL_CODE_SEGMENT_OFFSET;    
	IDT[num].zero = 0;
	IDT[num].type_attr = INTERRUPT_GATE;   
}

void ports_remap() {
	write_port(0x20, 0x11);
	write_port(0xA0, 0x11);
	write_port(0x21, 0x20);
	write_port(0xA1, 0x28);
	write_port(0x21, 0x00);
	write_port(0xA1, 0x00);
	write_port(0x21, 0x01);
	write_port(0xA1, 0x01);
	write_port(0x21, 0xff);
	write_port(0xA1, 0xff);
}

void idt_init(void) {
	unsigned long idt_addr, idt_ptr[2];
	ports_remap();
	idt_addr = (unsigned long)IDT;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_addr & 0xffff) << 16);
	idt_ptr[1] = idt_addr >> 16 ;
	load_idt(idt_ptr);
}

#endif
