[BITS 32]

[GLOBAL irq0_stub]
[GLOBAL irq1_stub]

[EXTERN irq0_handler]
[EXTERN irq1_handler]

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
