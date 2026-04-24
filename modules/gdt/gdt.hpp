#ifndef GDT_HPP
#define GDT_HPP

#include <stdint.h>

static constexpr uint16_t KERNEL_CS_SELECTOR = 0x08;
static constexpr uint16_t KERNEL_DS_SELECTOR = 0x10;
static constexpr uint16_t USER_CS_SELECTOR = 0x1B;
static constexpr uint16_t USER_DS_SELECTOR = 0x23;
static constexpr uint16_t TSS_SELECTOR = 0x28;

void gdt_tss_init(uint32_t kernel_stack_top);
void tss_set_kernel_stack(uint32_t kernel_stack_top);

#endif