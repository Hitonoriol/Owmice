#define CALLS 12

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
	&prompt
};

extern volatile registers_t regdump;
typedef int func(int arg1, int arg2);

int kcall_handle(uint32_t call, int arg1, int arg2) {
	if (call >= CALLS) {
		printf("[Invalid syscall received: %u]\n", call);
		return -1;
	}
	//tasking_pause();
	//printf("[Syscall received: %u %u %u]\n", call, arg1, arg2);
	//return 123;

	void *location = syscalls[call];
	int ret = ((func *)location)(arg1, arg2);
	//dump_registers();
	//printf("[Syscall to 0x%X returned: %u]\n", (uint32_t)location, ret);
	return ret;
}

extern void syscall(void);
void kernel_calls_init() {
	printf("Syscall entrypoint: 0x%X\n", (uint32_t)syscall);
}
