section .asm

global insb
global insw
global outb
global outw

insb:
    push ebp
    mov ebp, esp
    mov edx, [ebp+8]
    xor eax, eax
    in al, dx
    pop ebp
    ret

insw:
    push ebp
    mov ebp, esp
    mov edx, [ebp+8]
    xor eax, eax
    in ax, dx
    pop ebp
    ret

outb:
    push ebp
    mov ebp, esp
    mov edx, [ebp+8]
    mov eax, [ebp+12]
    out dx, al
    pop ebp
    ret

outw:
    push ebp
    mov ebp, esp
    mov edx, [ebp+8]
    mov eax, [ebp+12]
    out dx, ax
    pop ebp
    ret
