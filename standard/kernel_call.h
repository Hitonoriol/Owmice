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

extern volatile registers_t regdump;
void kcall_handle() {
	get_regs();
	if (regdump.eax >= CALLS)
		return;	
	tasking_pause();
	printf("[Kernel call #%u]\n", regdump.eax);
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
	printf("%u end\n", regdump.eax);
	tasking_resume();
	EOI();
}

void kernel_calls_init() {
	irq_map_handler(0x99, (uint32_t)kcall_handle);
}
