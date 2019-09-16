typedef unsigned int uint32;
typedef unsigned char uint8;
typedef unsigned int uintptr;

#define KHEAPFLAG_USED			0x80000000

typedef struct _KHEAPHDRLCAB {
	uint32				prevsize;
	uint32				flagsize;
} KHEAPHDRLCAB;

typedef struct _KHEAPBLOCKLCAB {
	uint32					size;
	uint32					used;
	struct _KHEAPBLOCKLCAB	                *next;
	uint32					lastdsize;
	KHEAPHDRLCAB			        *lastdhdr;
} KHEAPBLOCKLCAB;

typedef struct _KHEAPLCAB {
	KHEAPBLOCKLCAB		       *fblock;
	uint32				bcnt;
	uint32				mem_in_use;
	uint32				size;
} KHEAPLCAB;

void k_heapLCABInit(KHEAPLCAB *heap) {
		heap->fblock = 0;
		heap->bcnt = 0;
		heap->mem_in_use = 0;
}

int k_heapLCABAddBlock(KHEAPLCAB *heap, uintptr addr, uint32 size) {
	KHEAPBLOCKLCAB			*hb;
	KHEAPHDRLCAB			*hdr;
	
	//printf("add block addr:%p size:%x\n", addr, size);
	heap->size += size;
	hb = (KHEAPBLOCKLCAB*)addr;
	hb->size = size;
	hb->used = 0;
	hb->next = heap->fblock;
	heap->fblock = hb;
	
	hdr = (KHEAPHDRLCAB*)&hb[1];
	hdr->flagsize = hb->size - (sizeof(KHEAPBLOCKLCAB) + 32);
	
	++heap->bcnt;
	
	hb->lastdsize = 0;
	hb->lastdhdr = 0;
	
	//printf("added block; block-count:%u\n", heap->bcnt);
	
	//printf("added block hb:%p hb->next:%p\n", hb, hb->next);
	return 1;
}

/*
	Look behind and forward to see if we can merge back into some chunks.
*/
void k_heapLCABFree(KHEAPLCAB *heap, void *ptr) {
	KHEAPHDRLCAB				*hdr, *phdr, *nhdr;
	KHEAPBLOCKLCAB				*hb;
	uint32						sz = 0;
	uint8						fg = 0;
	
	if (fg || sz)
		fg = fg;
	//printf("lab pre-free ptr:%p\n", ptr);
	
	hdr = (KHEAPHDRLCAB*)ptr;
	//printf("GGGG\n");
	hdr[-1].flagsize &= ~0x80000000;
		
	//printf("lcab free\n");
		
	phdr = 0;
	/* find the block we are located in */
	for (hb = heap->fblock; hb; hb = hb->next) {
		//printf("lcab free looking at block:%p next:%p ptr:%p end:%p\n", hb, hb->next, ptr, (uintptr)hb + hb->size);
		if (((uintptr)ptr > (uintptr)hb) && ((uintptr)ptr < (uintptr)hb + hb->size)) {
			/* we have found the heap block */
			
			/* get header */
			hdr = (KHEAPHDRLCAB*)((uintptr)ptr - sizeof(KHEAPHDRLCAB));
			//printf("hdr 0x%X\n", (uint32_t*)hdr);
			/* set to free */
			hdr->flagsize &= ~0x80000000;
			printf("free %u\n", hdr->flagsize);
			heap->mem_in_use -= hdr->flagsize;
			hb->used -= hdr->flagsize;
			
			/* get previous header if it exists */
			//printf("prevsize %u\n", hdr->prevsize);
			if (hdr->prevsize) {
				phdr = (KHEAPHDRLCAB*)((uintptr)&hdr - (sizeof(KHEAPHDRLCAB) + hdr->prevsize));
			} else {
				phdr = 0;
			}
			
			//printf("hdr:%p hdr->flagsize:%x hdr->prevsize:%x\n", hdr, hdr->flagsize, hdr->prevsize);
			/* get next header */
			nhdr = (KHEAPHDRLCAB*)((uintptr)&hdr[1] + hdr->flagsize);
			if ((uintptr)nhdr >= ((uintptr)hb + hb->size)) {
				nhdr = 0;
			}
						
			//fg = hdr->flagsize >> 31;
			//sz = hdr->flagsize & 0x7fffffff;			
			
			//printf("hdr:%p phdr:%p nhdr:%p phdr->flagsize:%x hdr->flagsize:%x\n", hdr, phdr, nhdr, phdr->flagsize, hdr->flagsize);
			if (nhdr) {
				if (!(nhdr->flagsize & 0x80000000)) {
					/* combine with it */
					hdr->flagsize += sizeof(KHEAPHDRLCAB) + nhdr->flagsize;
					hb->used -= sizeof(KHEAPHDRLCAB);
					heap->mem_in_use -= sizeof(KHEAPHDRLCAB);
					/* set next header prevsize */
					nhdr = (KHEAPHDRLCAB*)((uintptr)&hdr[1] + hdr->flagsize);
					nhdr->prevsize = hdr->flagsize;
				}
			}
			
			//printf("here hdr:%p prevsize:%x\n", hdr, hdr->prevsize);
			
			/* can we combine with previous header? */
			if (phdr) {
				//printf("phdr 0x%X\n", (uint32_t*)phdr);
				if (!(phdr->flagsize & 0x80000000)) {	
					//printf("combine backward\n");
					phdr->flagsize += sizeof(KHEAPHDRLCAB) + hdr->flagsize;
					hb->used -= sizeof(KHEAPHDRLCAB);
					heap->mem_in_use -= sizeof(KHEAPHDRLCAB);
					hdr = phdr;
					/* set next header prevsize */
					nhdr = (KHEAPHDRLCAB*)((uintptr)&hdr[1] + hdr->flagsize);
					if ((uintptr)nhdr < (uintptr)hb + sizeof(KHEAPBLOCKLCAB) + hb->size) {
						nhdr->prevsize = hdr->flagsize;
					}
				}
			}
			
			/* optimization */
			if (hdr->flagsize > hb->lastdsize) {
				hb->lastdsize = hdr->flagsize;
				hb->lastdhdr = hdr;
			}
			
			return;
		}
	}
	
	printf("uhoh ptr:0x%X\n", ptr);
	for (hb = heap->fblock; hb; hb = hb->next) {
		printf("lcab free looking at block:0x%X next:0x%X ptr:0x%X end:0x%X\n", hb, hb->next, ptr, (uintptr)hb + hb->size);
		if (((uintptr)ptr > (uintptr)hb)) {
			printf("above\n");
			if (((uintptr)ptr < ((uintptr)hb + hb->size))) {
				printf("found\n");
			}
		}
	}
	for (;;);
	/* uhoh.. this should not happen.. */
	return;
}

/*
	This will allocate a chunk of memory by the specified size, and if
	no memory chunk can be found it will return zero.
*/
void* k_heapLCABAlloc(KHEAPLCAB *heap, uint32 size) {
	KHEAPBLOCKLCAB		*hb;
	KHEAPHDRLCAB		*hdr, *_hdr, *phdr;
	uint32				sz;
	uint8				fg;
	uint32				checks;
	uint32				bc;
	
	checks = 0;
	bc =0;
	
	//printf("lcab alloc\n");
	/* first find heap block with enough space */
	for (hb = heap->fblock; hb; hb = hb->next) {
		if ((hb->size - hb->used) >= (size + sizeof(KHEAPHDRLCAB))) {
			++bc;
			/* optimization */
			//if (hb->lastdsize >= size) {
				/* optimization (use block) */
				//hdr = hb->lastdhdr;
				//hb->lastdhdr = 0;
				//hb->lastdsize = 0;
			//} else {
			hdr = (KHEAPHDRLCAB*)&hb[1];
			//}
			
			//printf("enter loop\n");
			phdr = 0;
			if (phdr)
				phdr = phdr;
			while ((uintptr)hdr < ((uintptr)hb + hb->size)) {
				++checks;
				//printf("lcab alloc found\n");
				fg = hdr->flagsize >> 31;
				sz = hdr->flagsize & 0x7fffffff;
				//printf("hdr:%p fg:%x sz:%x\n", hdr, fg, sz);
				if (!fg) {
					//printf("lcab alloc got chunk\n");
					if (sz >= size) {
						//printf("lcab alloc thinking of splitting\n");
						// else take whole chunk
						if (sz > (size + sizeof(KHEAPHDRLCAB) + 16)) {
							//printf("lcab alloc splitting\n");
							/* has enough left over (break into two) */
							_hdr = (KHEAPHDRLCAB*)((uintptr)&hdr[1] + size);
							//printf("AA\n");
							/* take out data size and header size */
							_hdr->flagsize = sz - (size + sizeof(KHEAPHDRLCAB));
							_hdr->prevsize = size;
							//printf("BB\n");
							/* set to used and set new size */
							hdr->flagsize = 0x80000000 | size;
							//printf("CC\n");
							hb->used += sizeof(KHEAPHDRLCAB);
							heap->mem_in_use += sizeof(KHEAPHDRLCAB);
							//printf("DD\n");
						} else {
							/* simply set to used */
							hdr->flagsize |= 0x80000000;
						}
						hb->used += size;
						
						//printf("ptr:%p\n", &hdr[1]);
						//printf("alloced checks:%u blocks-checked:%u\n", checks, bc);
						heap->mem_in_use += size;
						//printf("heap alloc 0x%X\n", &hdr[1]);
						return &hdr[1];
					}
				}
				phdr = hdr;
				hdr = (KHEAPHDRLCAB*)((uintptr)&hdr[1] + sz);
			}
			//printf("exit loop\n");
		}
	}
	
	//printf("return null\n");
	return 0;
}

KHEAPLCAB kernel_heap;
