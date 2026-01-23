section .asm

global idt_load
global no_interrupt

global enable_interrupts
global disable_interrupts

extern no_interrupt_handler

%macro INT_DEFINE 1
global int%{1}h_isr
extern int%{1}h_handler
int%{1}h_isr:
    pushad
    call int%{1}h_handler
    popad
    iret
%endmacro


idt_load:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]        ; get frame pointer from stack
    lidt [eax]
    pop ebp
    ret

no_interrupt:
    pushad
    call no_interrupt_handler
    popad
    iret

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

; %assign r 20
; %rep 15
;     INT_DEFINE r
;     %assign r r+1
; %endrep
INT_DEFINE 21