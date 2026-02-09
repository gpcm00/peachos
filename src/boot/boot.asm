ORG 0x7c00
[BITS 16]

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

KERNEL_ADDR equ 0x0100000
KERNEL_SECT equ 1

entry:
    jmp short start
    nop


; [Reserved]    = 200
; [FAT1]        = 256
; [FAT2]        = 256
; [ROOT]        = 4
; [DATA]        = 8388608
; times 33 db 0
; FAT16 Header
OEMName:                db  'PeachOS '
BytesPerSector:         dw  512             ; 0x0200
SectorsPerCluster:      db  128             ; 0x80
ReservedSectorCount:    dw  200             ; 0xC8
NumFATs:                db  2               ; 0x02
RootEntryCount:         dw  64              ; 0x40
TotalSectors16:         dw  0               ; 0x0000
Media:                  db  0xF8            ; 0xF8
SectorsPerFAT:          dw  256             ; 0x0100
SectorsPerTrack:        dw  32              ; 0x0020
NumHeads:               dw  64              ; 0x0040
HiddenSectors:          dd  0               ; 0x00000000
TotalSectors32:         dd  8387914         ; 0x007FFD4A
                            
; FAT16 Extended BPB
DriveNumber:            db  0x80
Reserved1:              db  0x00
BootSignature:          db  0x29
VolumeID:               dd  0xD105
VolumeLabel:            db  'POS BOOT   '
FileSystemType:         db  'FAT16   '
; FAT16 Header
; OEMIdentifier           db 'PEACHOS '
; BytesPerSector          dw 0x200
; SectorsPerCluster       db 0x80
; ReservedSectors         dw 200
; FATCopies               db 0x02
; RootDirEntries          dw 0x40
; NumSectors              dw 0x00
; MediaType               db 0xF8
; SectorsPerFat           dw 0x100
; SectorsPerTrack         dw 0x20
; NumberOfHeads           dw 0x40
; HiddenSectors           dd 0x00
; SectorsBig              dd 0x773594

; ; Extended BPB (Dos 4.0)
; DriveNumber             db 0x80
; WinNTBit                db 0x00
; Signature               db 0x29
; VolumeID                dd 0xD105
; VolumeIDString          db 'PEACHOS BOO'
; SystemIDString          db 'FAT16   '

start:
    jmp 0x0000:.init_segments

.init_segments:
    cli              ; clear interrupt flag
    mov ax, 0x0000   ; using offsets 
    mov ds, ax
    mov es, ax      
    xor ax, ax       ; zero stack segment 
    mov ss, ax      
    mov sp, 0x7c00   ; stack starts

.load_protected:
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:load32

    sti
    jmp $


gdt_start:
gdt_null:
    dd 0
    dd 0


gdt_code:           ; CS as flat addressing mode
    dw 0xFFFF       ; code segment limit
    dw 0x0000       ; base 0-15 bits
    db 0x00         ; base 16-23 bits
    db 0x9A         ; access byes
    db 0xCF         ; flags 4 bits, limit 4bits
    db 0x00         ; base 24-31 bits

gdt_data:           ; DS, SS, ES, FS, GS as flat addressing mode
    dw 0xFFFF       ; code segment limit
    dw 0x0000       ; base 0-15 bits
    db 0x00         ; base 16-23 bits
    db 0x93         ; access byes
    db 0xCF         ; flags 4 bits, limit 4bits
    db 0x00         ; base 24-31 bits

gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start-1
    dd gdt_start

[BITS 32]
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; enable a20 line
    in al, 0x92
    or al, 0x2
    out 0x92, al

.load_kernel:
    mov eax, KERNEL_SECT
    push eax

    shr eax, 24
    or eax, 0xE0    ; master drive lba mode + lba bits 24-27
    mov dx, 0x1F6   ; ata head port
    out dx, al

    ; total sectors to read
    mov ecx, 100
    mov eax, ecx
    mov dx, 0x1F2   ; ata sector count
    out dx, al

    ; lba low bytes
    pop eax
    mov dx, 0x1F3
    out dx, al

    ; lba mid bytes
    shr eax, 8
    mov dx, 0x1F4
    out dx, al

    ; lba high bytes
    shr eax, 8
    mov dx, 0x1F5
    out dx, al

    ; send read sector command
    mov eax, 0x20
    mov dx, 0x1F7
    out dx, al

    mov edi, KERNEL_ADDR  ; load address

.next_sector:
    push ecx

.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 0x08   ; disk seek complete?
    jz .try_again   ; loop while not complete

    ; read 256 words (512 bytes)
    mov ecx, 256
    mov dx, 0x1F0
    rep insw        ; repeat insw 256 times
    pop ecx
    loop .next_sector   ; loop ecx times

    jmp CODE_SEG:KERNEL_ADDR

    jmp $

times 510 - ($ - $$) db 0
dw 0xAA55