#ifndef VGA_BUFFER_HPP
#define VGA_BUFFER_HPP

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;

struct VGABuffer
{
    volatile uint8_t *buffer;
    uint16_t x, y;
    bool cursor_visible;
    uint32_t last_blink_tick;

    void putchar(char c);
    void print(const char *str);
    void clear();
    void sync_cursor();
    void set_cursor_visible(bool visible);
    void tick(uint32_t ticks);
};

extern VGABuffer vga_buffer;

void vga_init();

#endif