%define BIT(n) (1<<(n))
[BITS 32]
section .asm

global paging_load_directory
global enable_paging

paging_load_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov cr3, eax
    pop ebp
    ret

enable_paging:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, BIT(31)
    mov cr0, eax
    pop ebp
    ret