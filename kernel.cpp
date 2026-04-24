#include "modules/vga_buffer.hpp"
#include "modules/keyboard.hpp"

extern "C" void kernel_main()
{
    vga_init();
    keyboard_init();

    vga_buffer.clear();
    vga_buffer.print("=== Kernix v0.3 ===\n");
    vga_buffer.print("Kernel loaded successfully!\n");
    vga_buffer.print("Running in 32-bit mode\n");
    vga_buffer.print("Type something and press Enter:\n> ");

    char input[64];
    int length = 0;

    while (true)
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            input[length] = '\0';
            vga_buffer.putchar('\n');
            vga_buffer.print("You typed: ");
            vga_buffer.print(input);
            vga_buffer.print("\n> ");
            length = 0;
            continue;
        }

        if (c == '\b')
        {
            if (length > 0)
            {
                length--;
                vga_buffer.putchar('\b');
            }
            continue;
        }

        if (c >= 32 && c <= 126 && length < 63)
        {
            input[length++] = c;
            vga_buffer.putchar(c);
        }
    }
}