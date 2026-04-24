#include "modules/vga_buffer.hpp"
#include "modules/keyboard.hpp"
#include "modules/string.hpp"

extern "C" void kernel_main()
{
    vga_init();
    keyboard_init();

    vga_buffer.clear();
    vga_buffer.print("=== Kernix v0.4 ===\n");
    vga_buffer.print("Kernel loaded successfully!\n");
    vga_buffer.print("Running in 32-bit mode\n");
    vga_buffer.print("Type help to get some help:\n> ");

    char input[512];
    int length = 0;

    while (true)
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            input[length] = '\0';
            vga_buffer.putchar('\n');

            char *command = input;
            char *args = nullptr;

            char *space = find_space(input);

            if (space)
            {
                *space = '\0';
                args = space + 1;
            }

            if (strcmp(command, "help") == 0)
            {
                vga_buffer.print("Available commands:\n");
                vga_buffer.print("help - Show this help message\n");
                vga_buffer.print("clear - Clear the screen\n");
                vga_buffer.print("about - About project\n");
                vga_buffer.print("version - Show kernel version\n");
                vga_buffer.print("echo <text>\n");
            }
            else if (strcmp(command, "clear") == 0)
            {
                vga_buffer.clear();
            }
            else if (strcmp(command, "about") == 0)
            {
                vga_buffer.print("Kernix it is the simple representation of monolith kernel.\n");
            }
            else if (strcmp(command, "version") == 0)
            {
                vga_buffer.print("v0.4\n");
            }
            else if (strcmp(command, "echo") == 0)
            {
                if (args)
                    vga_buffer.print(args);

                vga_buffer.putchar('\n');
            }
            else
            {
                vga_buffer.print("Unknown command: ");
                vga_buffer.print(command);
                vga_buffer.putchar('\n');
            }

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
                vga_buffer.putchar(' ');
                vga_buffer.putchar('\b');
            }
            continue;
        }

        if (c >= 32 && c <= 126 && length < 511)
        {
            input[length++] = c;
            vga_buffer.putchar(c);
        }
    }
}