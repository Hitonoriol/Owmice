#ifndef _STANDARD_MEM_H
#define _STANDARD_MEM_H

volatile uint32_t mem_unused;
volatile uint32_t mem_free = 0;
extern uint32_t end;

void free(uint32_t sz) {		//only one malloc at a time can be freed... obviously
	mem_unused -= sz;
	mem_free += sz;
}

uint32_t alloc(uint32_t sz, bool palign) {
	uint32_t tmp = mem_unused;
	if (sz > mem_free)
		die(STATUS_NOMEM);
	if (palign && (mem_unused & 0xFFFFF000)) {
    		mem_unused &= 0xFFFFF000;
    		mem_unused += 0x1000;
    		mem_free -= 0x1000 + (tmp - mem_unused);
	}
        tmp = mem_unused;
        mem_unused += sz;
	mem_free -= sz;
        return tmp;
}

uint32_t amalloc(uint32_t sz) {
	return alloc(sz, true);
}

uint32_t malloc(uint32_t sz) {
	return alloc(sz, false);
}

uint32_t get_mem() {
	return mem_free;
}

#endif
