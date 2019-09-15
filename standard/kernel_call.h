#define CALLS 7

static void *syscalls[CALLS] = {
	&die,
	&term_writestring,
	&today,
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
	printf("[Kernel call #%u]\n", regdump.eax);
	if (regdump.eax >= CALLS)
		die(0xBADCA11);
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

