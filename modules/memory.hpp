#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stdint.h>

void paging_init();
void kmalloc_init();
void *kmalloc(uint32_t size);

#endif
