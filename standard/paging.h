#define FLAG_NOT_PRESENT 0x00000002
uint32_t root_pagedir[1024] __attribute__((aligned(4096)));
uint32_t root_pagetable[1024] __attribute__((aligned(4096)));	//first 4 MB

void paging_enable() {
	asm volatile ("\
	push %ebp;\
	mov %esp, %ebp;\
	mov %cr0, %eax;\
	or $0x80000000, %eax;\
	mov %eax, %cr0;\
	mov %ebp, %esp;\
	pop %ebp;\
	");
}

void pagedir_load(unsigned int *ptr) {
	asm volatile ("\
	push %ebp;\
	mov %esp, %ebp;\
	mov 8(%esp), %eax;\
	mov %eax, %cr3;\
	mov %ebp, %esp;\
	pop %ebp;\
	");
}

void pagefault() {
	uint32_t addr;
	asm volatile("mov %%cr2, %0" : "=r" (addr));
	setcolor(VGA_COLOR_LIGHT_CYAN);
	cprint("Pagefault!", VGA_COLOR_RED);
	printf("At %X\n", addr);
	die(0);
}

void paging_init() {
	for(size_t i = 0; i < 1024; i++)
		root_pagedir[i] = FLAG_NOT_PRESENT;
	for(size_t i = 0; i < 1024; i++)
		root_pagetable[i] = (i * 0x1000) | 3; // rw/present/supervisor
	root_pagedir[0] = ((unsigned int)root_pagetable) | 3;
	irq_map_handler(14, (unsigned long)pagefault);
	pagedir_load((unsigned int*)root_pagedir);
	paging_enable();
	
}
