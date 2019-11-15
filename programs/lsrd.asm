bits 32
org 0x100060

_start:
	mov eax, CALL_LS_INITRD
	call syscall0
	ret

%include "api.inc"
