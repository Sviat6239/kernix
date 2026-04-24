#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

void keyboard_init();
void keyboard_poll();
bool keyboard_has_char();
char keyboard_getchar();

#endif
