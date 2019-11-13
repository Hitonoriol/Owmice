#ifndef SYSTEM_H
#define SYSTEM_H

typedef struct registers {
    uint32_t ds;                  
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    uint32_t int_no, err_code;    
    uint32_t eip, cs, eflags, useresp, ss; 
} registers_t;

volatile registers_t regdump;

void get_regs() {
	asm volatile("movl %%eax, %0":"=m"(regdump.eax));
	asm volatile("movl %%edi, %0":"=m"(regdump.edi));
	asm volatile("movl %%esi, %0":"=m"(regdump.esi));
	asm volatile("movl %%ebx, %0":"=m"(regdump.ebx));
	asm volatile("movl %%ecx, %0":"=m"(regdump.ecx));
	asm volatile("movl %%edx, %0":"=m"(regdump.edx));
	asm volatile("movl %%esp, %0":"=m"(regdump.esp));
	asm volatile("1: lea 1b, %0;": "=a"(regdump.eip));
	asm volatile("movl %%ebp, %0":"=m"(regdump.ebp));
}

void dump_registers() {
	get_regs();
	uint32_t cr3;
	asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
	setcolor(VGA_COLOR_RED);
	printf("CR3: 0x%X EIP: 0x%X\n", cr3, regdump.eip);
	printf("EAX: 0x%X EBX: 0x%X ECX: 0x%X EDX: 0x%X\n", regdump.eax, regdump.ebx, regdump.ecx, regdump.edx);
	printf("ESI: 0x%X EDI: 0x%X\n", regdump.esi, regdump.edi);
	printf("ESP: 0x%X\n", regdump.esp);
	setcolor(VGA_COLOR_LIGHT_GREY);
}

#define STATUS_BAD_MULTIBOOT 0x0BADB007
#define STATUS_NOMEM 0xBADB00B5
#define STATUS_GENERAL 0xBADDEAD
#define STATUS_PAGEFAULT 0x546EDEAD
#define STATUS_BREAKPOINT 0xDB6

extern void wait_int();
char* deadmsg = "\nOwmice died.";
void die(uint32_t stat) {
	int_off();
	hidecur();
	setcolor(VGA_COLOR_RED);
	kprint(deadmsg);
	if (!(stat == STATUS_BREAKPOINT)) {
		printf("Status: 0x%X\n", stat);
	}
	dump_registers();
	wait_int();
}

void _die(uint32_t stat, const char *file, uint32_t line) {
	printf("Assertion at %s:%u failed.\n", file, line);
	die(stat);
}

void stop(const char *file, uint32_t line) {
	setcolor(VGA_COLOR_RED);
	printf("Breakpoint at %s:%u reached. Bye.\n", file, line);
	die(STATUS_BREAKPOINT);
}

#endif
