#ifndef OWAPI_H
#define OWAPI_H

#define CALL_BASE 0x100040

#define CALL_DIE 0

#define CALL_PUTCHAR 1		//uint32_t
#define CALL_WRITESTRING 2	//char*
#define CALL_CLS 3		//void

#define CALL_KBD_GET_STRING 4	//char*

#define CALL_PRINT_DATE 5	//void
#define CALL_MEM_INFO 6		//void

#define CALL_MALLOC 7		//uint32_t
#define CALL_FREE 8		//uint32_t

#define CALL_LS_INITRD 9	//void
#define CALL_CAT_INITRD 10	//char*

#define CALL_PROMPT 11		//void
#define CALL_TERM_SET_TITLE 12	//char*

#define CALL_EXEC_INITRD 13	//char*

#define CALL_GET_CHAR 14	//void

#define CALL_GET_TICKS 15	//void | returns unsigned long long*
#define CALL_READ_INITRD 16	//char*

typedef int sys_call(uint32_t call, int32_t arg1, int32_t arg2, int32_t arg3);

int owmice_call0(uint32_t call) {
	return ((sys_call *)CALL_BASE)(call, 0, 0, 0);
}

int owmice_call1(uint32_t call, int arg1) {
	return ((sys_call *)CALL_BASE)(call, arg1, 0, 0);
}

int owmice_call2(uint32_t call, int arg1, int arg2) {
	return ((sys_call *)CALL_BASE)(call, arg1, arg2, 0);
}

int owmice_call3(uint32_t call, int arg1, int arg2, int arg3) {
	return ((sys_call *)CALL_BASE)(call, arg1, arg2, arg3);
}

/********************************************************/

void owmice_die(uint32_t status) {
	owmice_call1(CALL_DIE, (int)status);
}

void owmice_putchar(uint32_t chr) {
	owmice_call1(CALL_PUTCHAR, (int)chr);
}

void owmice_writestring(char* str_ptr) {
	owmice_call1(CALL_WRITESTRING, (int)str_ptr);
}

void owmice_cls() {
	owmice_call0(CALL_CLS);
}

void owmice_kbd_get_string(char* buf) {
	owmice_call1(CALL_KBD_GET_STRING, (int)buf);
}

void owmice_print_date() {
	owmice_call0(CALL_PRINT_DATE);
}

void owmice_print_meminfo() {
	owmice_call0(CALL_MEM_INFO);
}

void* owmice_malloc(uint32_t sz) {
	return (void*)owmice_call1(CALL_MALLOC, sz);
}

void owmice_free(void* addr) {
	owmice_call1(CALL_FREE, (int)addr);
}

void owmice_ls_initrd() {
	owmice_call0(CALL_LS_INITRD);
}

void owmice_cat_initrd(char* fname) {
	owmice_call1(CALL_CAT_INITRD, (int)fname);
}

void owmice_prompt() {
	owmice_call0(CALL_PROMPT);
}

void owmice_set_title(char* str) {
	owmice_call1(CALL_TERM_SET_TITLE, (int)str);
}

int owmice_exec_initrd(char* fname, int arg1, int arg2) {
	return owmice_call3(CALL_EXEC_INITRD, (int)fname, arg1, arg2);
}

char owmice_get_char() {
	return owmice_call0(CALL_GET_CHAR);
}

unsigned long long owmice_get_ticks() {
	return *((unsigned long long*)owmice_call0(CALL_GET_TICKS));
}

char* owmice_read_initrd(char* fname) {
	return (char*)owmice_call1(CALL_READ_INITRD, (int)fname);
}
/********************************************************/
#endif
