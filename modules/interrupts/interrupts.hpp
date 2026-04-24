#ifndef INTERRUPTS_HPP
#define INTERRUPTS_HPP

#include <stdint.h>

void interrupts_init();
void interrupts_enable();

uint32_t interrupts_get_ticks();

extern "C" void cpu_exception_handler(uint32_t vector, uint32_t error_code);

extern "C" uint32_t irq0_handler(uint32_t current_esp);
extern "C" void irq1_handler();

#endif
