#include <stdint.h>

#include "keyboard.hpp"

static char key_buffer[256];
static int buf_head = 0;
static int buf_tail = 0;
static bool shift_pressed = false;
static bool extended_scancode = false;

static const char keymap[128] = {
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b',
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
};

static const char keymap_shift[128] = {
    0,
    27,
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    '\b',
    '\t',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    '\n',
    0,
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    0,
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    0,
    '*',
    0,
    ' ',
};

static void push_char(char c)
{
    int next = (buf_tail + 1) % 256;
    if (next == buf_head)
        return;

    key_buffer[buf_tail] = c;
    buf_tail = next;
}

static char pop_char()
{
    char c = key_buffer[buf_head];
    buf_head = (buf_head + 1) % 256;
    return c;
}

void keyboard_init()
{
    buf_head = 0;
    buf_tail = 0;
    shift_pressed = false;
    extended_scancode = false;
}

void keyboard_handle_scancode(uint8_t scancode)
{
    if (scancode == 0xE0)
    {
        extended_scancode = true;
        return;
    }

    if (extended_scancode)
    {
        extended_scancode = false;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36)
    {
        shift_pressed = true;
        return;
    }

    if (scancode == 0xAA || scancode == 0xB6)
    {
        shift_pressed = false;
        return;
    }

    if (scancode & 0x80)
        return;

    char c = shift_pressed ? keymap_shift[scancode] : keymap[scancode];
    if (c != 0)
        push_char(c);
}

bool keyboard_available()
{
    return buf_head != buf_tail;
}

char keyboard_getchar()
{
    while (!keyboard_available())
        __asm__ volatile("hlt");

    return pop_char();
}