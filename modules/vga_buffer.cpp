#include "vga_buffer.hpp"

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x0F

VGABuffer vga_buffer;

void vga_init()
{
    vga_buffer.buffer = (volatile uint8_t *)VGA_BUFFER;
    vga_buffer.x = 0;
    vga_buffer.y = 0;
}

void VGABuffer::putchar(char c)
{
    if (c == '\b')
    {
        if (x > 0)
        {
            x--;
        }
        else if (y > 0)
        {
            y--;
            x = VGA_WIDTH - 1;
        }

        uint16_t offset = (y * VGA_WIDTH + x) * 2;
        buffer[offset] = ' ';
        buffer[offset + 1] = VGA_COLOR;
        return;
    }

    if (c == '\n')
    {
        x = 0;
        y++;
        if (y >= VGA_HEIGHT)
            y = VGA_HEIGHT - 1;
        return;
    }

    uint16_t offset = (y * VGA_WIDTH + x) * 2;
    buffer[offset] = c;
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

void VGABuffer::print(const char *str)
{
    while (*str)
        putchar(*str++);
}

void VGABuffer::clear()
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2)
    {
        buffer[i] = ' ';
        buffer[i + 1] = VGA_COLOR;
    }
    x = 0;
    y = 0;
}