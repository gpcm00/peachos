[BITS 32]
global _start
extern kernel_main

_start:
    ; init PIC
    mov al, 0x11    ; PIC init | ICW4
    out 0x20, al
    mov al, 0x20    ; master ISR maps start at int 20h
    out 0x21, al
    mov al, 0x04
    out 0x21, al
    mov al, 0x01
    out 0x21, al

    call kernel_main
    jmp $


times 512 - ($ - $$) db 0