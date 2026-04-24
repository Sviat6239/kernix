#include "shell.hpp"
#include "../vga_buffer/vga_buffer.hpp"
#include "../string/string.hpp"
#include "../syscalls/syscalls.hpp"
#include "../interrupts/interrupts.hpp"
#include "../memory/memory.hpp"

void print_uint32(uint32_t value)
{
    char buffer[11];
    int index = 0;

    if (value == 0)
    {
        ksys_putchar('0');
        return;
    }

    while (value > 0 && index < 10)
    {
        buffer[index++] = static_cast<char>('0' + (value % 10));
        value /= 10;
    }

    while (index > 0)
        ksys_putchar(buffer[--index]);
}

void shell_init()
{
    ksys_print("Shell initialized.\n");
}

void shell_handle_input(char *input)
{
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
        ksys_print("Available commands:\n");
        ksys_print("help - Show this help message\n");
        ksys_print("clear - Clear the screen\n");
        ksys_print("about - About project\n");
        ksys_print("version - Show kernel version\n");
        ksys_print("ticks - Show timer ticks (kernel call)\n");
        ksys_print("systicks - Show ticks via syscall (int 0x90)\n");
        ksys_print("syswrite <text> - Write text via syscall\n");
        ksys_print("mem - Show allocator status\n");
        ksys_print("panic - Trigger a test exception\n");
        ksys_print("echo <text> - Echo text\n");
    }
    else if (strcmp(command, "clear") == 0)
    {
        ksys_clear_screen();
    }
    else if (strcmp(command, "about") == 0)
    {
        ksys_print("Kernix it is the simple representation of monolith kernel.\n");
    }
    else if (strcmp(command, "version") == 0)
    {
        ksys_print("Kenel v0.0.7\n");
        ksys_print("Shell v0.0.2\n");
    }
    else if (strcmp(command, "ticks") == 0)
    {
        ksys_print("ticks: ");
        print_uint32(interrupts_get_ticks());
        ksys_putchar('\n');
    }
    else if (strcmp(command, "systicks") == 0)
    {
        ksys_print("systicks: ");
        print_uint32(sys_get_ticks());
        ksys_putchar('\n');
    }
    else if (strcmp(command, "syswrite") == 0)
    {
        if (args)
        {
            unsigned int len = 0;
            const char *p = args;
            while (*p)
            {
                len++;
                p++;
            }

            ksys_print("[DEBUG] len=");
            print_uint32(len);
            ksys_print(" text=");
            ksys_print(args);
            ksys_print(" -> ");

            ksys_print("syscall result: ");
            print_uint32(sys_write(args, len));
            ksys_putchar('\n');
        }
        else
        {
            ksys_print("Usage: syswrite <text>\n");
        }
    }
    else if (strcmp(command, "mem") == 0)
    {
        ksys_print("heap total: ");
        print_uint32(kmalloc_total());
        ksys_print(" bytes\n");
        ksys_print("heap used: ");
        print_uint32(kmalloc_used());
        ksys_print(" bytes\n");
        ksys_print("heap free: ");
        print_uint32(kmalloc_remaining());
        ksys_print(" bytes\n");
    }
    else if (strcmp(command, "panic") == 0)
    {
        ksys_print("Triggering divide-by-zero exception...\n");
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
            ksys_print(args);

        ksys_putchar('\n');
    }
    else
    {
        ksys_print("Unknown command: ");
        ksys_print(command);
        ksys_putchar('\n');
    }
}