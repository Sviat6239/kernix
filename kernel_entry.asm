[BITS 32]
[GLOBAL _start]
[EXTERN kernel_main]

; Multiboot header
MAGIC equ 0x1BADB002
FLAGS equ 0x00000003
CHECKSUM equ -(MAGIC + FLAGS)

ALIGN 4
dd MAGIC
dd FLAGS
dd CHECKSUM

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Clear BSS section
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb
    
    ; Jump to C++ kernel
    call kernel_main
    
    ; Hang if kernel returns
    cli
    hlt
    jmp $

SECTION .bss
    ALIGN 16
    stack_bottom: resq 16
    stack_top:

    bss_start:
    bss_end:
