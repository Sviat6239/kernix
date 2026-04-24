#include "modules/vga_buffer.hpp"

extern "C" void kernel_main()
{
    vga_init();

    vga_buffer.clear();
    vga_buffer.print("=== Kernix v0.2 ===\n");
    vga_buffer.print("Kernel loaded successfully!\n");

    while (true)
        __asm__("hlt");
}