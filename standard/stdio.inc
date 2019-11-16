extern void* memmove(void*, const void*, size_t);
extern int memcmp(const void* aptr, const void* bptr, size_t size);
extern void* memset(void* bufptr, int value, size_t size);
extern void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);

extern int printf(char *fmt, ...);

extern int rand();
extern void srand(unsigned int seed);
