#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stdint.h>

void paging_init();
void kmalloc_init();
void *kmalloc(uint32_t size);
void kfree(void *ptr);
void *kcalloc(uint32_t count, uint32_t size);
void *krealloc(void *ptr, uint32_t new_size);
uint32_t ksize(const void *ptr);
uint32_t kmalloc_remaining();
uint32_t kmalloc_used();
uint32_t kmalloc_total();

#endif
