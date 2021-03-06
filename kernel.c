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
#include "system/system.h"
#include "io/ATA.h"
#include "io/idt.h"
//#include "standard/task.h"
#include "io/timer.h"
#include "io/cmos.h"
#include "io/keyboard.h"
#include "misc/multiboot.h"
#include "mem/heap.h"
#include "mem/mem.h"
#include "io/vfs.h"
#include "io/initrd.h"
#include "mem/paging.h"
#include "system/kernel_call.h"
#define assume(b) ((b) ? (void)0 : _die(0xBADA55, __FILE__, __LINE__))
extern uint32_t k_entry;
extern uint32_t exec_space;

extern unsigned long long kticks;

uint32_t kernel_size;
//task_t owshell;

char* autostart_fname = "autostart.dat";
char* shell_fname;

void test_disk() {
	master = newATA(1, 0x1F0);
	if(ATA_identify(master) != 0) {
		printf("Can't access the hard disk.\n");
		return;
	}
	printf("*Found master hard disk!\n");
	return;	
	printf("Would you like to do a write test? [y/n]\n");
	if (kbd_get_char() != 'y')
		return;
	printf("Write test...\n");
	if(ATA_write28(master, 0, (uint8_t*)"Yo wassup\n\0") != 0) {
		printf("Write test failed.\n");
	}
	char* firstSector = (char*)ATA_read28(master, 0);
	printf("First sector: %s\n", firstSector);
	free((uint32_t)firstSector);
}

uint32_t shell_buf, shell_size;

void kmain(unsigned long magic, unsigned long addr) {
	create_gdt();
	idt_init();
	console_init();
	multiboot_info_t *mboot_ptr = verify_multiboot(magic, addr);
        assume(mboot_ptr->mods_count > 0);
   	uint32_t initrd_location = *((uint32_t*)mboot_ptr->mods_addr);
   	uint32_t initrd_end = *(uint32_t*)(mboot_ptr->mods_addr+4);
   	kernel_size = ((uint32_t)&end - (uint32_t)&k_entry);
   	mem_unused = initrd_end + 1;
   	fs_root = initrd_init(initrd_location);
   	pmem_init(mboot_ptr);
   	mem_deinit_region(initrd_location, (initrd_end - initrd_location));
   	printf("Kernel: 0x%X 0x%X\n", (uint32_t)&k_entry, (uint32_t)(&end));
   	mem_deinit_region((uint32_t)&k_entry, (uint32_t)(&end - &k_entry));
   	kernel_calls_init();
	kbd_init();
	paging_init();
	read_buffer = (char*)malloc(INITRD_BUFFER_SIZE);
	printf("Exec space: 0x%X\n", (uint32_t)&exec_space);
	printf("Kernel end: 0x%X\n", &end);
	//tasking_init();
	timer_init(PIT_10MSEC);
        cprint("Ready!", VGA_COLOR_MAGENTA);
        test_disk();
	//task_spawn(&owshell, owshell_main, task_current->regs.eflags);

	//Reading autostart file for the autostart binary
	fs_node_t *start_node = finddir_fs(fs_root, autostart_fname);
	if (start_node == NULL) {
		printf("%s must contain a valid name of the binary existing on the ramdisk\n", autostart_fname);
		die(0);
	}
	printf("Reading %s...\n", autostart_fname);
	shell_fname = (char*)malloc(128);	
	clearchar(shell_fname);
	read_fs(start_node, 0, 128, (uint8_t*)shell_fname);
	shell_fname = trim(shell_fname);

	printf("Loading %s...\n", shell_fname);
	fs_node_t *fsnode = finddir_fs(fs_root, shell_fname);
	if (fsnode == NULL) {
		printf("%s doesn't exist.\n", shell_fname);
		die(0);
	}
	char* buf = (char*)malloc(INITRD_BUFFER_SIZE);
	uint32_t sz = read_fs(fsnode, 0, INITRD_BUFFER_SIZE, (uint8_t*)buf);
	free((uint32_t)buf);
	shell_size = sz;
	shell_buf = (uint32_t)malloc(shell_size);
	exec_initrd(shell_fname, 0, 0);
	
	die(0);
}
