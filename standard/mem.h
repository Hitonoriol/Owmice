#ifndef _STANDARD_MEM_H
#define _STANDARD_MEM_H

volatile uint32_t mem_unused;
volatile uint32_t mem_free = 0;
extern uint32_t end;

void free(uint32_t sz) {		//only one malloc at a time can be freed... obviously
	mem_unused -= sz;
}

uint32_t malloc(uint32_t sz) {
	if (sz > mem_free)
		die(STATUS_NOMEM);
        uint32_t tmp = mem_unused;
        mem_unused += sz;
	mem_free -= sz;
        return tmp;
}

uint32_t get_mem() {
	return mem_free;
}

#endif
