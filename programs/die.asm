bits 32
org 0x100060

_start:
	mov eax, CALL_DIE
	mov ebx, 0xDEAD
	call syscall1
	ret

%include "api.inc"
