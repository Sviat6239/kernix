#include "modules/vga_buffer/vga_buffer.hpp"
#include "modules/keyboard/keyboard.hpp"
#include "modules/string/string.hpp"
#include "modules/interrupts/interrupts.hpp"
#include "modules/memory/memory.hpp"
#include "modules/syscalls/syscalls.hpp"
#include "modules/gdt/gdt.hpp"
#include "modules/shell/shell.hpp"
#include "modules/string/string.hpp"

[[noreturn]] static void enter_ring3(uint32_t entry, uint32_t user_stack_top)
{
    __asm__ volatile(
        "cli\n\t"
        "movw %2, %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "pushl %2\n\t"
        "pushl %0\n\t"
        "pushfl\n\t"
        "popl %%eax\n\t"
        "orl $0x200, %%eax\n\t"
        "pushl %%eax\n\t"
        "pushl %3\n\t"
        "pushl %1\n\t"
        "iretl\n\t"
        :
        : "r"(user_stack_top), "r"(entry), "i"(USER_DS_SELECTOR), "i"(USER_CS_SELECTOR)
        : "eax", "memory");

    while (true)
        __asm__ volatile("hlt");
}

static void user_mode_main()
{
    char input[1024];
    uint32_t length = 0;

    while (true)
    {
        uint32_t c = sys_getchar();
        if (c == 0xFFFFFFFFu)
            continue;

        char ch = static_cast<char>(c);

        if (ch == '\r')
            ch = '\n';

        if (ch == '\n')
        {
            input[length] = '\0';
            sys_putchar('\n');

            if (input[0] != '\0')
                sys_shell_exec(input);

            const char prompt[] = "\n> ";
            sys_write(prompt, sizeof(prompt) - 1);
            length = 0;
            continue;
        }

        if (ch == '\b')
        {
            if (length > 0)
            {
                length--;
                sys_putchar('\b');
                sys_putchar(' ');
                sys_putchar('\b');
            }
            continue;
        }

        if (ch >= 32 && ch <= 126 && length < 1023)
        {
            input[length++] = ch;
            sys_putchar(ch);
        }
    }
}

static void kernel_shell_loop()
{
    char input[1024];
    uint32_t length = 0;

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

extern "C" void kernel_main()
{
    vga_init();
    paging_init();
    kmalloc_init();

    uint8_t *ring0_stack = reinterpret_cast<uint8_t *>(kmalloc(8 * 1024));
    if (!ring0_stack)
    {
        ksys_print("Failed to allocate ring0 stack for TSS\n");
        while (true)
            __asm__ volatile("hlt");
    }

    gdt_tss_init(reinterpret_cast<uint32_t>(ring0_stack + (8 * 1024)));

    interrupts_init();
    keyboard_init();

    ksys_clear_screen();
    ksys_print("=== Kernix v0.0.9 ===\n");
    ksys_print("Kernel loaded successfully!\n");
    ksys_print("Running in 32-bit mode, ring3 enabled\n");
    shell_init_message();

    uint8_t *user_stack = reinterpret_cast<uint8_t *>(kmalloc(16 * 1024));
    if (!user_stack)
    {
        ksys_print("Failed to allocate user stack\n");
        while (true)
            __asm__ volatile("hlt");
    }

    interrupts_enable();
    kernel_shell_loop();
}