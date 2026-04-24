#ifndef VGA_BUFFER_HPP
#define VGA_BUFFER_HPP

using uint8_t = unsigned char;
using uint16_t = unsigned short;

struct VGABuffer
{
    volatile uint8_t *buffer;
    uint16_t x, y;

    void putchar(char c);
    void print(const char *str);
    void clear();
};

extern VGABuffer vga_buffer;

void vga_init();

#endif