bits 32
org 0x100060

_start:
	mov eax, CALL_CLS
	call syscall0
	ret

%include "api.inc"
