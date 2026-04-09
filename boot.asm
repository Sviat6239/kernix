format binary as 'img'
org 7C00h

        xor     ax, ax
        mov     ds, ax
        mov     es, ax
        mov     ss, ax
        mov     sp, 7C00h

        ; Normalize CS to 0000h via far return.
        push    ax
        push    start
        retf

start:
        mov     al, 'H'
        call    print
        mov     al, 'i'
        call    print

hang:
        jmp     hang

print:
        pusha
        mov     bx, 0007h    ; Text attribute: light gray on black
        mov     ah, 0Eh      ; BIOS teletype output
        int     10h
        popa
        ret