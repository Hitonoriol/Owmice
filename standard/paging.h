#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR	1024
#define PAGEDIR_IDX(x) (((x) >> 22) & 0x3ff)
#define PAGETABLE_IDX(x) (((x) >> 12) & 0x3ff)
#define PAGE_ADDR(x) (*x & ~0xfff)
#define PTABLE_ADDR_SPACE_SIZE 0x400000
#define DTABLE_ADDR_SPACE_SIZE 0x100000000
#define PAGE_SIZE 4096
#define KERNEL_VADDR_START 0xC0000000
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
	uint32_t entry[TABLES_PER_DIR];
} pagedir_t;

pagedir_t *dir_current;

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


void pagefault(registers_t regs) {
	uint32_t addr;
	asm volatile("mov %%cr2, %0" : "=r" (addr));
	int present   = !(regs.err_code & 0x1);
	int rw = regs.err_code & 0x2;
	int us = regs.err_code & 0x4;
	int reserved = regs.err_code & 0x8;
	int id = regs.err_code & 0x10;

	setcolor(VGA_COLOR_LIGHT_CYAN);
	printf("\nPagefault at 0x%X\n", addr);
	if (present) printf("Not present\n");
   	if (rw) printf("RO\n");
   	if (us) printf("Usermode\n");
   	if (reserved) printf("Reserved\n");
    	if (id) printf("Instruction fetch\n");
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

bool switch_directory (pagedir_t* dir) {
	if (!dir)
		return false;
	dir_current = dir;
	pagedir_load((uint32_t*)dir);
	return true;
}
 
pagedir_t* get_directory() {
	return dir_current;
}

void map_table (uint32_t phys, uint32_t virt) {
	pagedir_t* tempdir = get_directory();
	uint32_t* e = &tempdir->entry [PAGEDIR_IDX ((uint32_t) virt)];
	if ((*e & PAGE_PRESENT) != PAGE_PRESENT) {
		pagetable_t* table = (pagetable_t*) alloc_block();
		if (!table)
			die(STATUS_NOMEM);
		memset (table, 0, sizeof(pagetable_t));
		for (uint32_t i = 0, frame = phys, _virt = virt; i < 1024; i++, frame += PAGE_SIZE, _virt += PAGE_SIZE){
			uint32_t page = 0;
			page_set_property(&page, PAGE_PRESENT);
			page_set_property(&page, PAGE_WRITABLE);
			page_set_frame(&page, frame);
			table->entry[PAGETABLE_IDX(_virt)] = page;
		}
		uint32_t* entry = &tempdir->entry [PAGEDIR_IDX((uint32_t)virt)];
		table_add_property (entry, PDE_PRESENT);
		table_add_property (entry, PDE_WRITABLE);
		table_set_frame (entry, (uint32_t)table);
	}
	return;
}

void map_tables(uint32_t phys, uint32_t virt, uint32_t amt) {
	uint32_t step = PAGE_SIZE * PAGES_PER_TABLE;
	while(amt > 0) {
		map_table(phys, virt);
		phys += step;
		virt += step;
		--amt;
	}
}

bool alloc_page (uint32_t* e) {
	void* p = alloc_block();
	if (!p)
		return false;
	page_set_frame (e, (uint32_t)p);
	page_set_property (e, PAGE_PRESENT);
	page_set_property (e, PAGE_WRITABLE);
	return true;
}

void free_page (uint32_t* e) {
	void* p = (void*)page_pfn (*e);
	if (p)
		free_block (p);
	page_del_property (e, PAGE_PRESENT);
}

#define MEM_INIT_SIZE 4
extern uint32_t kernel_size;
void paging_init() {
	kernel_mem = (mem_t*)malloc(sizeof(mem_t));
	kernel_mem->start = KERNEL_VADDR_START + kernel_size;
	kernel_mem->end = kernel_mem->start + (MEM_INIT_SIZE * 0x100000);
	printf("Initializing paging... ");
	pagetable_t* table = (pagetable_t*) alloc_block();
	if (!table)
		die(STATUS_NOMEM);

	pagetable_t* mem_start_table = (pagetable_t*) alloc_block();
	if (!mem_start_table)
		die(STATUS_NOMEM);
	memset(table, 0, sizeof (pagetable_t));

	for (uint32_t i = 0, frame = 0x0, virt = 0x00000000; i < PAGES_PER_TABLE; i++, frame += PAGE_SIZE, virt += PAGE_SIZE) {	//identity mapping 0-4MB
		uint32_t page = 0;
		page_set_property(&page, PAGE_PRESENT);
		page_set_property(&page, PAGE_WRITABLE);
 		page_set_frame(&page, frame);
		mem_start_table->entry [PAGETABLE_IDX (virt)] = page;
	}
	for (uint32_t i = 0, frame = KERNEL_PHYS_START, virt = KERNEL_VADDR_START; i < PAGES_PER_TABLE; i++, frame += PAGE_SIZE, virt += PAGE_SIZE) { // phys_start -> vaddr_start
		uint32_t page = 0;
		page_set_property(&page, PAGE_PRESENT);
		page_set_property(&page, PAGE_WRITABLE);
		page_set_frame(&page, frame);
		table->entry[PAGETABLE_IDX(virt)] = page;
	}
	pagedir_t* dir = (pagedir_t*) alloc_blocks(3);
	printf("Dir addr: 0x%X\n", (uint32_t*)dir);
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
	if (!switch_directory(dir))
		die(0xD1);
	printf("%u kernel pages\n", ((0x1000+(kernel_size&0xFFFFF000))/1024));
	map_tables(KERNEL_PHYS_START, KERNEL_VADDR_START, ((0x1000+(kernel_size&0xFFFFF000))/1024)+MEM_INIT_SIZE + 1);
	paging_enable();
	printf("%X %X %u\n", kernel_mem->start, kernel_mem->end, kernel_mem->end-kernel_mem->start);
	mem_unused = kernel_mem->start;
	printf("Done!\n");
}
