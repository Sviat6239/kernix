#include "vga_buffer.hpp"

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x0F
#define VGA_CURSOR_COMMAND 0x3D4
#define VGA_CURSOR_DATA 0x3D5
#define VGA_CURSOR_DISABLE 0x20
#define VGA_CURSOR_BLINK_PERIOD 100

VGABuffer vga_buffer;

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void vga_init()
{
    vga_buffer.buffer = (volatile uint8_t *)VGA_BUFFER;
    vga_buffer.x = 0;
    vga_buffer.y = 0;
    vga_buffer.cursor_visible = true;
    vga_buffer.last_blink_tick = 0;
    vga_buffer.sync_cursor();
}

void VGABuffer::sync_cursor()
{
    uint16_t position = static_cast<uint16_t>(y * VGA_WIDTH + x);

    outb(VGA_CURSOR_COMMAND, 0x0F);
    outb(VGA_CURSOR_DATA, static_cast<uint8_t>(position & 0xFF));
    outb(VGA_CURSOR_COMMAND, 0x0E);
    outb(VGA_CURSOR_DATA, static_cast<uint8_t>((position >> 8) & 0xFF));

    set_cursor_visible(cursor_visible);
}

void VGABuffer::set_cursor_visible(bool visible)
{
    uint8_t cursor_start;

    outb(VGA_CURSOR_COMMAND, 0x0A);
    cursor_start = inb(VGA_CURSOR_DATA);

    if (visible)
    {
        outb(VGA_CURSOR_COMMAND, 0x0A);
        outb(VGA_CURSOR_DATA, static_cast<uint8_t>(cursor_start & ~VGA_CURSOR_DISABLE));
    }
    else
    {
        outb(VGA_CURSOR_COMMAND, 0x0A);
        outb(VGA_CURSOR_DATA, static_cast<uint8_t>(cursor_start | VGA_CURSOR_DISABLE));
    }
}

void VGABuffer::tick(uint32_t ticks)
{
    if (ticks - last_blink_tick < VGA_CURSOR_BLINK_PERIOD)
        return;

    last_blink_tick = ticks;
    cursor_visible = !cursor_visible;
    set_cursor_visible(cursor_visible);
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
        sync_cursor();
        return;
    }

    if (c == '\n')
    {
        x = 0;
        y++;
        if (y >= VGA_HEIGHT)
            y = VGA_HEIGHT - 1;
        sync_cursor();
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

    sync_cursor();
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
    cursor_visible = true;
    last_blink_tick = 0;
    sync_cursor();
}