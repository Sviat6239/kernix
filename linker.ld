ENTRY(_start)

SECTIONS {
    . = 0x100000;
    
    .text ALIGN(0x1000) : {
        *(.text*)
        *(.rodata*)
    }
    
    .data ALIGN(0x1000) : {
        *(.data*)
    }
    
    .bss ALIGN(0x1000) : {
        bss_start = .;
        *(.bss*)
        *(COMMON)
        bss_end = .;
    }

    kernel_end = .;
    
    /DISCARD/ : {
        *(.note*)
        *(.comment*)
    }
}
