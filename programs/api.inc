syscall0:
	push dword 0
	push dword 0
	push eax
	call [syscall_base]
	add esp, 12
	ret

syscall1:
	push 0
	push ebx
	push eax
	call [syscall_base]
	add esp, 12
	ret

syscall2:
	push ecx
	push ebx
	push eax
	call [syscall_base]
	add esp, 12
	ret



syscall_base: dd 0x100040

;****************************************

CALL_DIE: equ 0			;void
CALL_PUTCHAR: equ 1		;uint32_t
CALL_WRITESTRING: equ 2		;char*
CALL_CLS: equ 3			;void
CALL_KBD_GET_STRING: equ 4	;char*
CALL_PRINT_DATE: equ 5		;void
CALL_MEM_INFO: equ 6		;void
CALL_MALLOC: equ 7		;uint32_t
CALL_FREE: equ 8		;uint32_t
CALL_LS_INITRD: equ 9		;void
CALL_CAT_INITRD: equ 10		;char*
CALL_PROMPT: equ 11		;void
CALL_TERM_SET_TITLE: equ 12	;char*
CALL_EXEC_INITRD: equ 13	;char*

;****************************************
