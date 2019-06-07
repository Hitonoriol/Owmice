#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#define brk() stop(__FILE__, __LINE__);
#include "io/kbdmap.h"
#include "standard/strings.h"
#include "screen/terminal.h"
#include "standard/stdio.h"
#include "io/idt.h"
#include "standard/task.h"
#include "io/timer.h"
#include "standard/cmos.h"
#include "owshell/owshell.h"
#include "io/keyboard.h"
#include "standard/mem.h"
#include "misc/multiboot.h"
#include "io/vfs.h"
#include "io/initrd.h"
#include "standard/paging.h"
#include "standard/kernel_call.h"
#define assume(b) ((b) ? (void)0 : die(0xBADA55))
extern uint32_t k_entry;
extern volatile uint32_t kticks;
task_t owshell;
void kmain(unsigned long magic, unsigned long addr) {
	create_gdt();
	idt_init();
	console_init();
	term_tempcolor(VGA_COLOR_GREEN);
	multiboot_info_t *mboot_ptr = verify_multiboot(magic, addr);
        assume(mboot_ptr->mods_count > 0);
   	uint32_t initrd_location = *((uint32_t*)mboot_ptr->mods_addr);
   	uint32_t initrd_end = *(uint32_t*)(mboot_ptr->mods_addr+4);
	mem_free -= ((initrd_end - initrd_location) + (&end - &k_entry)) + 1;
   	mem_unused = initrd_end + 1;
   	fs_root = initrd_init(initrd_location);
   	kernel_calls_init();
	kbd_init();
	paging_init();
	tasking_init();

	timer_init(PIT_10MSEC);
	printf("Available to kernel: %uB\n", mem_free);
	term_revertcolor();
        cprint("Ready!", VGA_COLOR_MAGENTA);
	task_spawn(&owshell, owshell_main, task_current->regs.eflags);
	while(1) {
		wait_int();
	}
}
