#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#define brk() stop(__FILE__, __LINE__);
#include "io/kbdmap.h"
#include "standard/bitmap.h"
#include "standard/strings.h"
#include "screen/terminal.h"
#include "standard/stdio.h"
#include "io/idt.h"
#include "standard/task.h"
#include "io/timer.h"
#include "standard/cmos.h"
#include "owshell/owshell.h"
#include "io/keyboard.h"
#include "misc/multiboot.h"
#include "standard/mem.h"
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
	multiboot_info_t *mboot_ptr = verify_multiboot(magic, addr);
	pmem_init(mboot_ptr);
        assume(mboot_ptr->mods_count > 0);
   	uint32_t initrd_location = *((uint32_t*)mboot_ptr->mods_addr);
   	uint32_t initrd_end = *(uint32_t*)(mboot_ptr->mods_addr+4);
   	uint32_t kernel_size = ((initrd_end - initrd_location) + (&end - &k_entry)) + 1;
   	mem_deinit_region((uint32_t)&k_entry, &end - &k_entry);
   	mem_deinit_region(initrd_location, initrd_end - initrd_location);
	mem_free -= kernel_size;
   	mem_unused = initrd_end + 1;
   	fs_root = initrd_init(initrd_location);
   	kernel_calls_init();
	kbd_init();
	paging_init();
	tasking_init();
	timer_init(PIT_10MSEC);
	mem_deinit_region((uint32_t)&k_entry, mem_unused - (uint32_t)&k_entry);
	//at this point this shitty offset malloc should be disabled
	printf("Available to kernel: %uB\n", mem_free);
        cprint("Ready!", VGA_COLOR_MAGENTA);
	task_spawn(&owshell, owshell_main, task_current->regs.eflags);
	draw_clock();
	while(1) {
		task_sleep(&task_main, 100);
		draw_clock();
		wait_int();
	}
}
