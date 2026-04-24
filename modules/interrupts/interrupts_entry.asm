[BITS 32]

[GLOBAL isr0_stub]
[GLOBAL isr13_stub]
[GLOBAL isr14_stub]
[GLOBAL irq0_stub]
[GLOBAL irq1_stub]

[EXTERN cpu_exception_handler]
[EXTERN irq0_handler]
[EXTERN irq1_handler]

isr0_stub:
    pushad
    push dword 0
    push dword 0
    call cpu_exception_handler
    add esp, 8
    popad
    iretd

isr13_stub:
    pushad
    push dword [esp + 32]
    push dword 13
    call cpu_exception_handler
    add esp, 8
    popad
    add esp, 4
    iretd

isr14_stub:
    pushad
    push dword [esp + 32]
    push dword 14
    call cpu_exception_handler
    add esp, 8
    popad
    add esp, 4
    iretd

irq0_stub:
    pushad
    call irq0_handler
    popad
    iretd

irq1_stub:
    pushad
    call irq1_handler
    popad
    iretd
