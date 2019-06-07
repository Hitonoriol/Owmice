#define SYSCALL0(fn, num) \
int syscall_##fn() { \
 int a; \
 asm volatile("int $0x99" : "=a" (a) : "0" (num)); \
 return a; \
}

#define SYSCALL1(fn, num, P1) \
int syscall_##fn(P1 p1) { \
 int a; \
 asm volatile("int $0x99" : "=a" (a) : "0" (num), "b" ((int)p1)); \
 return a; \
}

#define SYSCALL2(fn, num, P1, P2) \
int syscall_##fn(P1 p1, P2 p2) { \
 int a; \
 asm volatile("int $0x99" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2)); \
 return a; \
}

#define CALLS 7

SYSCALL1(die, 0, uint32_t);
SYSCALL1(term_writestring, 1, char *);
SYSCALL0(now, 2);
SYSCALL0(get_mem, 3);
SYSCALL1(kbd_get_string, 4, char *);
SYSCALL1(term_putchar, 5, char);
SYSCALL0(term_cls, 6);

static void *syscalls[CALLS] = {
	&die,
	&term_writestring,
	&now,
	&get_mem,
	&kbd_get_string,
	&term_putchar,
	&term_cls
};

extern void kcall_entry(void);

void kernel_calls_init() {
	irq_map_handler(0x99, (unsigned long)kcall_entry);
}

extern volatile registers_t regdump;
void kcall_handle(registers_t *regs) {
	get_regs();
	if (regdump.eax >= CALLS){
		dump_registers();
		die(0xBADBAD);
	}
	void *location = syscalls[regdump.eax];
	int ret;
	asm volatile (" \
	push %1; \
	push %2; \
	push %3; \
	push %4; \
	push %5; \
	call *%6; \
	pop %%ebx; \
	pop %%ebx; \
	pop %%ebx; \
	pop %%ebx; \
	pop %%ebx; "
	: "=a" (ret)
	: "r" (regdump.edi), "r" (regdump.esi), "r" (regdump.edx), "r" (regdump.ecx), "r" (regdump.ebx), "r" (location));
	EOI();
}

