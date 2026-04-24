#include "modules/vga_buffer/vga_buffer.hpp"
#include "modules/keyboard/keyboard.hpp"
#include "modules/string/string.hpp"
#include "modules/interrupts/interrupts.hpp"
#include "modules/memory/memory.hpp"

static void print_uint32(uint32_t value)
{
    char buffer[11];
    int index = 0;

    if (value == 0)
    {
        vga_buffer.putchar('0');
        return;
    }

    while (value > 0 && index < 10)
    {
        buffer[index++] = static_cast<char>('0' + (value % 10));
        value /= 10;
    }

    while (index > 0)
        vga_buffer.putchar(buffer[--index]);
}

extern "C" void kernel_main()
{
    vga_init();
    paging_init();
    kmalloc_init();
    keyboard_init();
    interrupts_init();
    interrupts_enable();

    vga_buffer.clear();
    vga_buffer.print("=== Kernix v0.0.4 ===\n");
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
                vga_buffer.print("ticks - Show timer ticks\n");
                vga_buffer.print("mem - Show allocator status\n");
                vga_buffer.print("panic - Trigger a test exception\n");
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
                vga_buffer.print("Kenel v0.0.5\n");
                vga_buffer.print("Shell v0.0.1\n");
            }
            else if (strcmp(command, "ticks") == 0)
            {
                vga_buffer.print("ticks: ");
                print_uint32(interrupts_get_ticks());
                vga_buffer.putchar('\n');
            }
            else if (strcmp(command, "mem") == 0)
            {
                vga_buffer.print("kmalloc remaining: ");
                print_uint32(kmalloc_remaining());
                vga_buffer.print(" bytes\n");
            }
            else if (strcmp(command, "panic") == 0)
            {
                vga_buffer.print("Triggering divide-by-zero exception...\n");
                __asm__ volatile(
                    "xor %%edx, %%edx\n\t"
                    "mov $1, %%eax\n\t"
                    "div %%edx\n\t"
                    :
                    :
                    : "eax", "edx");
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