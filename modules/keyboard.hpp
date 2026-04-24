#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <stdint.h>

void keyboard_init();
void keyboard_handle_scancode(uint8_t scancode);
bool keyboard_has_char();
char keyboard_getchar();

#endif
