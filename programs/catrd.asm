bits 32
org 0x100060

_start:
	push ebp
	mov ebp, esp

	mov ebx, [ebp + 8]
	mov eax, CALL_CAT_INITRD
	call syscall1

	pop ebp
	ret

%include "api.inc"
