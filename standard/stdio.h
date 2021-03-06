#ifndef STANDARD_STDIO_H
#define STANDARD_STDIO_H
#include "owapi.h"
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include "strings.h"
#define EOF (-1)

void* memmove(void* dstptr, const void* srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

int memcmp(const void* aptr, const void* bptr, size_t size) {
	const unsigned char* a = (const unsigned char*) aptr;
	const unsigned char* b = (const unsigned char*) bptr;
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

void* memset(void* bufptr, int value, size_t size) {
	unsigned char* buf = (unsigned char*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (unsigned char) value;
	return bufptr;
}

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

void ft_putchar(int c) {
    owmice_putchar((char)c);
}

int ft_printf_aux(const char *fmt, va_list ap, int len);

int ft_print_c(const char *fmt, va_list ap, int len) {
    int c = va_arg(ap, int);
    ft_putchar(c);
    return ft_printf_aux(fmt, ap, len + 1);
}

int ft_putnum(unsigned long n, unsigned int base, const char *digits) {
    int res = 1;
    if (n >= base)
        res += ft_putnum(n / base, base, digits);
    ft_putchar(digits[n % base]);
    return res;
}

char* nums_decimal = "0123456789";
int ft_print_d(const char *fmt, va_list ap, int len) {
    int n = va_arg(ap, int);
    unsigned long long u;
    if (n < 0) {
        ft_putchar('-');
        len++;
        u = -(unsigned)n;
    } else {
        u = n;
    }
    len += ft_putnum(u, 10, nums_decimal);
    return ft_printf_aux(fmt, ap, len);
}

char* nums_octal = "01234567";
int ft_print_o(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 8, nums_octal);
    return ft_printf_aux(fmt, ap, len);
}

int ft_print_u(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 10, nums_decimal);
    return ft_printf_aux(fmt, ap, len);
}

char* nums_hexs = "0123456789abcdef";
int ft_print_x(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 16, nums_hexs);
    return ft_printf_aux(fmt, ap, len);
}

char* nums_hexl = "0123456789ABCDEF";
int ft_print_X(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 16, nums_hexl);
    return ft_printf_aux(fmt, ap, len);
}

int ft_print_s(const char *fmt, va_list ap, int len) {
    const char *s = va_arg(ap, const char *);
    if (s == NULL) {
        s = "(null)";
    }
    while (*s) {
        ft_putchar(*s++);
        len++;
    }
    return ft_printf_aux(fmt, ap, len);
}

typedef int (*ft_print_dispatch_f)(const char *fmt, va_list ap, int len);

ft_print_dispatch_f ft_print_dispatch[256] = {
    ['c'] = ft_print_c,
    ['d'] = ft_print_d,
    ['i'] = ft_print_d,
    ['o'] = ft_print_o,
    ['u'] = ft_print_u,
    ['x'] = ft_print_x,
    ['X'] = ft_print_X,
    ['s'] = ft_print_s,
};

int ft_printf_aux(const char *fmt, va_list ap, int len) {
    int c;
    while (*fmt) {
        c = (unsigned char)*fmt++;
        if (c != '%') {
            ft_putchar(c);
            len++;
        } else {
            c = (unsigned char)*fmt++;
            if (ft_print_dispatch[c] == NULL) {
                if (c == '\0')
                    break;
                ft_putchar(c);
                len++;
            } else {
                return ft_print_dispatch[c](fmt, ap, len);
            }
        }
    }
    return len;
}

int ft_vprintf(const char *fmt, va_list ap) {
    return ft_printf_aux(fmt, ap, 0);
}

int printf(char *fmt, ...) {
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = ft_printf_aux(fmt, ap, 0);
    va_end(ap);
    return n;
}

void printhex(uint32_t val) {
	printf("0x%X\n", val);
}

static unsigned long int next = 1;
 
int rand() {
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) {
    next = seed;
}

int rand_range(int min, int max) {
	return (rand() % (max - min + 1)) + min;
}

#endif
