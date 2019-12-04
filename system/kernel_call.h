#define CALLS 24

void *syscalls[CALLS] = {
	&die,
	&term_putchar,
	&term_writestring,
	&term_cls,
	&kbd_get_string,
	&today,
	&get_mem,
	&malloc,
	&free,
	&ls_initrd,
	&cat_initrd,
	&prompt,
	&term_set_title,
	&exec_initrd,
	&kbd_get_char,
	&get_ticks,
	&read_initrd,
	&draw,
	&term_disable_scroll,
	&term_enable_scroll,
	&now,
	&status_cls,
	&sleep,
	&kbd_current_char
};

extern volatile registers_t regdump;
typedef int func(int arg1, int arg2, int arg3);

int kcall_handle(uint32_t call, int arg1, int arg2, int arg3) {
	if (call >= CALLS) {
		printf("[Invalid syscall received: %u]\n", call);
		return -1;
	}

	void *location = syscalls[call];
	int ret = ((func *)location)(arg1, arg2, arg3);
	return ret;
}

extern void syscall(void);
void kernel_calls_init() {
	printf("Syscall entrypoint: 0x%X\n", (uint32_t)syscall);
	irq_map_handler(0x99, (uint32_t)syscall);
}
