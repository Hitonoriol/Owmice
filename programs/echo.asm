bits 32
org 0x100060

_start:
	push ebp
	mov ebp, esp

	mov ecx, [ebp + 12]
	mov ebx, [ebp + 8]
	mov eax, CALL_WRITESTRING
	call syscall1

	pop ebp
	ret

%include "api.inc"
