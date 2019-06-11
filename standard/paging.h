#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024
#define PAGEDIR_IDX(x) (((x) >> 22) & 0x3ff)
#define PAGETABLE_IDX(x) (((x) >> 12) & 0x3ff)
#define PAGE_ADDR(x) (*x & ~0xfff)
#define PTABLE_ADDR_SPACE_SIZE 0x400000
#define DTABLE_ADDR_SPACE_SIZE 0x100000000
#define PAGE_SIZE 4096
#define KERNEL_VADDR_START 0xC0000000
#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x10000
#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x123890AB
#define HEAP_MIN_SIZE     0x10000
#define KERNEL_PHYS_START 0x100000
enum PAGE_FLAGS {
	PAGE_PRESENT = 1,
	PAGE_WRITABLE = 2,
	PAGE_USER = 4,
	PAGE_WRITETHOUGH = 8,
	PAGE_NOT_CACHEABLE = 0x10,
	PAGE_ACCESSED = 0x20,
	PAGE_DIRTY = 0x40,
	PAGE_PAT	= 0x80,	
	PAGE_CPU_GLOBAL = 0x100,
	PAGE_LV4_GLOBAL = 0x200,
   	PAGE_FRAME = 0x7FFFF000
};

enum PDE_FLAGS {
	PDE_PRESENT = 1,
	PDE_WRITABLE = 2,
	PDE_USER = 4,
	PDE_PWT = 8,
	PDE_PCD = 0x10,
	PDE_ACCESSED = 0x20,
	PDE_DIRTY = 0x40,
	PDE_4MB = 0x80,
	PDE_CPU_GLOBAL = 0x100,
	PDE_LV4_GLOBAL = 0x200,
   	PDE_FRAME = 0x7FFFF000
};

inline void page_set_property (uint32_t* e, uint32_t property) {
	*e |= property;
}

inline void page_del_property (uint32_t* e, uint32_t property) {
	*e &= ~property;
}

inline void page_set_frame (uint32_t* e, uint32_t addr) {
	*e = (*e & ~PAGE_FRAME) | addr;
}

inline bool page_is_present (uint32_t e) {
	return e & PAGE_PRESENT;
}

inline bool page_is_writable (uint32_t e) {
	return e & PAGE_WRITABLE;
}

inline uint32_t page_pfn (uint32_t e) {
	return e & PAGE_FRAME;
}

inline void table_add_property (uint32_t* e, uint32_t property) {
	*e |= property;
}

inline void table_del_property (uint32_t* e, uint32_t property) {
	*e &= ~property;
}

inline void table_set_frame (uint32_t* e, uint32_t addr) {
	*e = (*e & ~PDE_FRAME) | addr;
}

inline bool table_is_present (uint32_t e) {
	return e & PDE_PRESENT;
}

inline bool table_is_writable (uint32_t e) {
	return e & PDE_WRITABLE;
}

inline uint32_t table_pfn (uint32_t e) {
	return e & PDE_FRAME;
}

inline bool table_is_user (uint32_t e) {
	return e & PDE_USER;
}

inline bool table_is_4mb (uint32_t e) {
	return e & PDE_4MB;
}

typedef struct pagetable_s {
	uint32_t entry[PAGES_PER_TABLE];
} pagetable_t;
 
typedef struct pagedir_s {
	uint32_t entry[PAGES_PER_DIR];
} pagedir_t;

pagedir_t *dir_current;
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

void pagedir_load(uint32_t *ptr) {
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
	printf("\nPagefault at 0x%X\n", addr);
	die(STATUS_PAGEFAULT);
}

uint32_t* get_page (pagetable_t* p, uint32_t addr) {
	if (p)
		return &p->entry[PAGETABLE_IDX(addr)];
	return 0;
}

pagetable_t* get_pagetable (pagedir_t* p, uint32_t addr) {
	if (p)
		return (pagetable_t*)PAGE_ADDR(&p->entry[PAGEDIR_IDX(addr)]);
	return 0;
}

uint32_t* _get_page (uint32_t addr) {
	return get_page(get_pagetable(dir_current, addr), addr);
}

inline bool switch_directory (pagedir_t* dir) {
	if (!dir)
		return false;
	dir_current = dir;
	pagedir_load((uint32_t*)dir);
	return true;
}
 
pagedir_t* get_directory() {
	return dir_current;
}

void map_page (void* phys, void* virt) {
	pagedir_t* tempdir = get_directory();
	uint32_t* e = &tempdir->entry [PAGEDIR_IDX ((uint32_t) virt)];
	if ((*e & PAGE_PRESENT) != PAGE_PRESENT) {
		pagetable_t* table = (pagetable_t*) alloc_block();
		if (!table)
			die(STATUS_NOMEM);
		memset (table, 0, sizeof(pagetable_t));
		uint32_t* entry = &tempdir->entry [PAGEDIR_IDX((uint32_t)virt)];
		table_add_property (entry, PDE_PRESENT);
		table_add_property (entry, PDE_WRITABLE);
		table_set_frame (entry, (uint32_t)table);
	}
	pagetable_t* table = (pagetable_t*) PAGE_ADDR(e);
	uint32_t* page = &table->entry [PAGETABLE_IDX((uint32_t)virt)];
	page_set_frame (page, (uint32_t) phys);
	page_set_property (page, PAGE_PRESENT);
}



bool alloc_page (uint32_t* e) {
	void* p = alloc_block();
	if (!p)
		return false;
	page_set_frame (e, (uint32_t)p);
	page_set_property (e, PAGE_PRESENT);
	return true;
}

void free_page (uint32_t* e) {
	void* p = (void*)page_pfn (*e);
	if (p)
		free_block (p);
	page_del_property (e, PAGE_PRESENT);
}


heap_t *heap_kernel;
extern heap_t *create_heap(uint32_t, uint32_t, uint32_t, uint8_t, uint8_t);
void paging_init() {
	printf("Initializing paging... ");
	pagetable_t* table = (pagetable_t*) alloc_block();
	if (!table)
		die(STATUS_NOMEM);

	pagetable_t* mem_start_table = (pagetable_t*) alloc_block();
	if (!mem_start_table)
		die(STATUS_NOMEM);

	memset (table, 0, sizeof (pagetable_t));

	for (uint32_t i = 0, frame = 0x0, virt = 0x00000000; i < 1024; i++, frame += PAGE_SIZE, virt += PAGE_SIZE) {	//the first 4MB are identity mapped
		uint32_t page = 0;
		page_set_property(&page, PAGE_PRESENT);
 		page_set_frame(&page, frame);
		mem_start_table->entry [PAGETABLE_IDX (virt)] = page;
	}
	for (uint32_t i = 0, frame = KERNEL_PHYS_START, virt = KERNEL_VADDR_START; i < 4096; i++, frame += PAGE_SIZE, virt += PAGE_SIZE) { // 1MB -> 3GB
		uint32_t page = 0;
		page_set_property(&page, PAGE_PRESENT);
		page_set_frame(&page, frame);
		table->entry[PAGETABLE_IDX(virt)] = page;
	}
	pagedir_t* dir = (pagedir_t*) alloc_blocks(3);
	kernel_dir = dir;
	if (!dir)
		die(0x366);
	memset (dir, 0, sizeof (pagedir_t));
	uint32_t* entry = &dir->entry [PAGEDIR_IDX(KERNEL_VADDR_START)];
	table_add_property (entry, PDE_PRESENT);
	table_add_property (entry, PDE_WRITABLE);
	table_set_frame (entry, (uint32_t)table);
	uint32_t* entry2 = &dir->entry[PAGEDIR_IDX(0x00000000)];
	table_add_property (entry2, PDE_PRESENT);
	table_add_property (entry2, PDE_WRITABLE);
	table_set_frame (entry2, (uint32_t)mem_start_table);
	irq_map_handler(14, (unsigned long)pagefault);
	switch_directory(dir);
	heap_kernel = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);
	paging_enable();
	printf("Done!\n");
}
