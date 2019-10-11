#ifndef _STANDARD_MEM_H
#define _STANDARD_MEM_H

#define BLOCKS_PER_BYTE 8
#define BLOCK_SIZE 4096
#define BLOCK_ALIGN BLOCK_SIZE
volatile uint32_t mem_size;
volatile uint32_t blocks_used = 0;
volatile uint32_t blocks_max;
volatile uint32_t mmap_len;
static uint32_t *memory_map;
volatile uint32_t mb_mmap_len;

volatile uint32_t mem_unused;	//end of memory pointer for offset alloc

typedef struct {
   uint32_t start;
   uint32_t end;
} mem_t;

mem_t *kernel_mem;

uint32_t malloc(uint32_t);
extern uint32_t end;

void mmap_set(int bit) {
	bit_set(memory_map, bit);
}

void mmap_unset(int bit) {
	bit_unset(memory_map, bit);
}

bool mmap_test(int bit) {
	return bit_test(memory_map, bit);
}

void mem_init_region (uint32_t base, size_t size) {
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;
	for (; blocks>0; blocks--) {
		uint32_t at = align;
		align++;
		if (mmap_test(at)) {
			mmap_unset (at);
			blocks_used--;
		}
	}
	mmap_set (0);
}

void mem_deinit_region (uint32_t base, size_t size) {
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;
	for ( ; blocks>0; blocks--) {
		uint32_t at = align;
		align++;
		if (!mmap_test(at)) {
			mmap_set (at);
			blocks_used++;
		}
	}
}

int find_free_block () {
	for (uint32_t i = 0; i < blocks_max / 32; i++)
		if (memory_map[i] != 0xffffffff)
			for (int j = 0; j<32; j++) {
				int bit = 1 << j;
				if (!(memory_map[i] & bit))
					return i*4*8+j;
			}
	return -1;
}

int mmap_first_free_s (size_t size) {
	if (size==0)
		return -1;
	if (size==1)
		return find_free_block();
	for (uint32_t i=0; i<blocks_max; i++)
		if (memory_map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {
				int bit = 1<<j;
				if (!(memory_map[i] & bit)) {
					int startingBit = i*32;
					startingBit+=bit;
					uint32_t free=0;
					for (uint32_t count=0; count<=size;count++) {
						if (! mmap_test (startingBit+count))
							free++;
						if (free==size)
							return i*4*8+j;
					}
				}
			}

	return -1;
}

void *alloc_block() {
	if (blocks_used >= blocks_max)
		die(STATUS_NOMEM+1);
	int frame = find_free_block();
	if (frame == -1)
		die(STATUS_NOMEM);
	mmap_set(frame);
	uint32_t addr = frame * BLOCK_SIZE;
	blocks_used++;
	return (void*)addr;
}

void free_block (void* p) {
	uint32_t addr = (uint32_t)p;
	int frame = addr / BLOCK_SIZE;
	mmap_unset (frame);
	blocks_used--;
}

void *alloc_blocks (size_t size) {
	printf("Allocating %u phys blocks...\n", size);
	if ((blocks_used - blocks_max) <= size)
		die(STATUS_NOMEM);
	int frame = mmap_first_free_s (size);

	if (frame == -1)
		die(STATUS_NOMEM);

	for (uint32_t i = 0; i < size; i++)
		mmap_set(frame + i);

	uint32_t addr = frame * BLOCK_SIZE;
	blocks_used += size;
	return (void*)addr;
}

void free_blocks (void* p, size_t size) {
	uint32_t addr = (uint32_t)p;
	int frame = addr / BLOCK_SIZE;
	for (uint32_t i = 0; i < size; i++)
		mmap_unset (frame+i);
	blocks_used -= size;
}

char* mem_regions[] = {
	"Available",	//0
	"Reserved",	//1
	"ACPI Reclaim",	//2
	"ACPI NVS Memory"//3
};

typedef struct _multiboot_memory_map {
	uint32_t size;
	uint32_t base_addr_low,base_addr_high;// or unsigned long long int base_addr
	uint32_t length_low,length_high;	// or unsigned long long int length
	uint32_t type;
} _multiboot_memory_map_t;

volatile _multiboot_memory_map_t *mb_mmap;

void pmem_init(multiboot_info_t* mbt_ptr) {
	mb_mmap_len = mbt_ptr->mmap_length;
        mem_size = mbt_ptr->mem_upper;
        mb_mmap = (_multiboot_memory_map_t*)mbt_ptr->mmap_addr;
        blocks_max = (mem_size * 1024) / 4096;
        printf("Total memory blocks: %u\n", blocks_max);
        mmap_len = blocks_max / BLOCKS_PER_BYTE;
        memory_map = (uint32_t*)malloc(mmap_len);
        memset (memory_map, 0xf, mmap_len);
        blocks_used = blocks_max;
        term_tempcolor(VGA_COLOR_LIGHT_BLUE);
        kprint("Memory map");
        while((uint32_t)mb_mmap < mbt_ptr->mmap_addr + mbt_ptr->mmap_length) {
        	printf("Addr: 0x%X%X Len: %uB Type: %s\n",
        		mb_mmap->base_addr_high, mb_mmap->base_addr_low,
        		mb_mmap->length_low,
        		mem_regions[mb_mmap->type]);
        	if (mb_mmap->type == 1)
			mem_init_region(mb_mmap->base_addr_low, mb_mmap->length_low);
		else {
			mem_deinit_region(mb_mmap->base_addr_low, mb_mmap->length_low);
		}
		mb_mmap = (_multiboot_memory_map_t*) ((unsigned int)mb_mmap + mb_mmap->size + sizeof(mb_mmap->size));
	}
	term_revertcolor();
	mem_size *= 1024;
}

void free(uint32_t sz) {
	if (kernel_mem == NULL) {
		mem_unused -= sz;
	} else {
		k_heapLCABFree(&kernel_heap, (void*)sz);
	}
}

uint32_t alloc(uint32_t sz, bool palign) {
	uint32_t tmp = mem_unused;
	if (palign && (mem_unused & 0xFFFFF000)) {
    		mem_unused &= 0xFFFFF000;
    		mem_unused += 0x1000;
	}
        tmp = mem_unused;
        mem_unused += sz;
	printf("Offset alloc %u at 0x%X\n", sz, tmp);
        return tmp;
}

uint32_t amalloc(uint32_t sz) {
	if (kernel_mem == NULL)
		return alloc(sz, true);
	else {
		return ((uint32_t)k_heapLCABAlloc(&kernel_heap, sz + 0x1000) & 0xFFFFF000);
	}
}

uint32_t malloc(uint32_t sz) {
	if (kernel_mem == NULL)
		return alloc(sz, false);
	else {
		return (uint32_t)k_heapLCABAlloc(&kernel_heap, sz);
	}
}

uint32_t get_free_mem() {
	return (blocks_max - blocks_used) * 0x1000;
}

void get_mem() {
	printf("Total physical memory: %uB\nPhysical blocks used: %u/%u (%u free)\n", mem_size, blocks_used, blocks_max, get_free_mem() / 0x1000);
	printf("Usable kernel vmem : 0x%X - 0x%X (%uB)\n", kernel_mem->start, kernel_mem->end, kernel_mem->end-kernel_mem->start);
	printf("Kernel heap: %uB/%uB (%uB free)", kernel_heap.mem_in_use, kernel_heap.size, kernel_heap.size - kernel_heap.mem_in_use);
}

#endif
