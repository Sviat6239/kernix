#ifndef INTERRUPTS_HPP
#define INTERRUPTS_HPP

#include <stdint.h>

void interrupts_init();
void interrupts_enable();

uint32_t interrupts_get_ticks();

extern "C" void irq0_handler();
extern "C" void irq1_handler();

#endif
