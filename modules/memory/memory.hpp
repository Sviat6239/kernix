#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stdint.h>

static constexpr uint32_t PAGE_FLAG_PRESENT = 0x001;
static constexpr uint32_t PAGE_FLAG_RW = 0x002;
static constexpr uint32_t PAGE_FLAG_USER = 0x004;

void paging_init();
void map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void unmap_page(uint32_t virt);

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
