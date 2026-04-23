#ifndef __cplusplus
#error "This file must be compiled as C++"
#endif

using uint8_t = unsigned char;
using uint16_t = unsigned short;

// VGA Buffer
#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x0F // White on black

struct VGABuffer
{
    volatile uint8_t *buffer;
    uint16_t x, y;

    VGABuffer() : buffer((uint8_t *)VGA_BUFFER), x(0), y(0) {}

    void putchar(char c)
    {
        if (c == '\n')
        {
            x = 0;
            y++;
            if (y >= VGA_HEIGHT)
                y = VGA_HEIGHT - 1;
            return;
        }

        uint16_t offset = (y * VGA_WIDTH + x) * 2;
        buffer[offset] = (uint8_t)c;
        buffer[offset + 1] = VGA_COLOR;

        x++;
        if (x >= VGA_WIDTH)
        {
            x = 0;
            y++;
            if (y >= VGA_HEIGHT)
                y = VGA_HEIGHT - 1;
        }
    }

    void print(const char *str)
    {
        while (*str)
        {
            putchar(*str++);
        }
    }

    void clear()
    {
        for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2)
        {
            buffer[i] = ' ';
            buffer[i + 1] = VGA_COLOR;
        }
        x = 0;
        y = 0;
    }
};

// Global VGA buffer
VGABuffer vga_buffer;

extern "C" void kernel_main()
{
    vga_buffer.buffer = (volatile uint8_t *)VGA_BUFFER;
    vga_buffer.x = 0;
    vga_buffer.y = 0;

    vga_buffer.clear();
    vga_buffer.print("=== Kernix v0.1 ===\n");
    vga_buffer.print("Kernel loaded successfully!\n");
    vga_buffer.print("Running in 32-bit mode\n");

    // Kernel loop
    while (true)
    {
        __asm__("hlt");
    }
}

// Prevent name mangling for global destructors
extern "C" void __cxa_pure_virtual()
{
    vga_buffer.print("Pure virtual function called!\n");
    while (true)
        ;
}
