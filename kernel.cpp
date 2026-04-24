#include "modules/vga_buffer/vga_buffer.hpp"
#include "modules/keyboard/keyboard.hpp"
#include "modules/string/string.hpp"
#include "modules/interrupts/interrupts.hpp"
#include "modules/memory/memory.hpp"
#include "modules/syscalls/syscalls.hpp"
#include "modules/shell/shell.hpp"
#include "modules/string/string.hpp"

extern "C" void kernel_main()
{
    vga_init();
    paging_init();
    kmalloc_init();
    keyboard_init();
    interrupts_init();
    interrupts_enable();

    ksys_clear_screen();
    ksys_print("=== Kernix v0.0.8 ===\n");
    ksys_print("Kernel loaded successfully!\n");
    ksys_print("Running in 32-bit mode\n");
    shell_init_message();

    char input[1024];
    int length = 0;

    while (true)
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            input[length] = '\0';
            ksys_putchar('\n');

            shell_handle_input(input);

            ksys_print("\n> ");
            length = 0;
            continue;
        }

        if (c == '\b')
        {
            if (length > 0)
            {
                length--;
                ksys_putchar('\b');
                ksys_putchar(' ');
                ksys_putchar('\b');
            }
            continue;
        }

        if (c >= 32 && c <= 126 && length < 1023)
        {
            input[length++] = c;
            ksys_putchar(c);
        }
    }
}