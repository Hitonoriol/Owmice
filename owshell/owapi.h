#define SYSCALL0(fn, num) \
int owmice_##fn() { \
 int a; \
 asm volatile("int $0x99" : "=a" (a) : "0" (num)); \
 return a; \
}

#define SYSCALL1(fn, num, P1) \
int owmice_##fn(P1 p1) { \
 int a; \
 asm volatile("int $0x99" : "=a" (a) : "0" (num), "b" ((int)p1)); \
 return a; \
}

#define SYSCALL2(fn, num, P1, P2) \
int owmice_##fn(P1 p1, P2 p2) { \
 int a; \
 asm volatile("int $0x99" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2)); \
 return a; \
}

#define SYSCALL3(fn, num, P1, P2, P3) \
int owmice_##fn(P1 p1, P2 p2, P3 p3) { \
 int a; \
 asm volatile("int $0x99" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3)); \
 return a; \
}

#define SYSCALL4(fn, num, P1, P2, P3, P4) \
int owmice_##fn(P1 p1, P2 p2, P3 p3, P4 p4) \
{ \
  int a; \
  asm volatile("int $0x99" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4)); \
  return a; \
}

#define SYSCALL5(fn, num) \
int owmice_##fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) \
{ \
  int a; \
  asm volatile("int $0x99" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4), "D" ((int)p5)); \
  return a; \
}

SYSCALL1(die, 0, uint32_t);
SYSCALL1(term_writestring, 1, char *);
SYSCALL0(now, 2);
SYSCALL0(get_mem, 3);
SYSCALL1(kbd_get_string, 4, char *);
SYSCALL1(term_putchar, 5, char);
SYSCALL0(term_cls, 6);
