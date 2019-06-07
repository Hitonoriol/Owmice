#define FLAG_NOT_PRESENT 0x00000002

typedef struct pagetable {
	uint32_t page[1024];
	struct pagetable *next;
} pagetable_t;

typedef struct pagedir {
	uint32_t tables[1024];
	pagetable_t *table;
} pagedir_t;

pagedir_t *kernel_dir;

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
	kernel_dir = (pagedir_t*)amalloc(sizeof(pagedir_t));
	kernel_dir->table = (pagetable_t*)amalloc(sizeof(pagetable_t));
	for(size_t i = 0; i < 1024; i++)
		kernel_dir->tables[i] = FLAG_NOT_PRESENT;
	for(size_t i = 0; i < 1024; i++)
		kernel_dir->table->page[i] = (i * 0x1000) | 3; // rw/present/supervisor
	kernel_dir->tables[0] = ((unsigned int)kernel_dir->table->page) | 3;
	irq_map_handler(14, (unsigned long)pagefault);
	pagedir_load((unsigned int*)kernel_dir->tables);
	paging_enable();
	
}
