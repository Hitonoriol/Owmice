MBALIGN  equ  1<<0              
MEMINFO  equ  1<<1              
FLAGS    equ  MBALIGN | MEMINFO 
MAGIC    equ  0x1BADB002        
CHECKSUM equ -(MAGIC + FLAGS)  
 
section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
 
section .text
global keyboard_handler
global read_port
global write_port
global load_idt

extern keyboard_handler_main

read_port:
	mov edx, [esp + 4]
	in al, dx
	ret

write_port:
	mov   edx, [esp + 4]    
	mov   al, [esp + 4 + 4]  
	out   dx, al  
	ret

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti
	ret

keyboard_handler:           
	pushad
	cld
	call keyboard_handler_main
	popad
	iretd

global _start
_start:
	mov esp, stack_space
	cli
	push ebx
	push eax
	extern kmain
	call kmain
	hlt

extern kcall_handle
global syscall
syscall:
	push ebp
	mov ebp, esp

	push dword [ebp + 16]
	push dword [ebp + 12]
	push dword [ebp + 8]	

	call kcall_handle

	add esp, 12

	pop ebp
	ret

;extern pit_handler_main
;global pit_handler
;pit_handler:
;	pushad
;	cld
;	call pit_handler_main
;	popad
;	iretd

;extern kcall_handle	
;global kcall_entry
;kcall_entry:
;	call kcall_handle
;	iretd

global create_gdt

gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

create_gdt:
    lgdt [gdt_descriptor]
    jmp CODE_SEG:.setcs              
.setcs:
    mov eax, DATA_SEG
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    ret
    
global dump_stack
extern printhex
dump_stack:
  push ebp
  mov ebp, esp
  call printhex
  pop ebp
  ret

global in_buffer
in_buffer: times 130 db 0

section .bss
resb 16192
stack_space:
