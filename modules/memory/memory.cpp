#include <stdint.h>

#include "memory.hpp"

extern "C" uint8_t kernel_end;

static uint32_t g_kmalloc_cursor = 0;

static constexpr uint32_t PAGE_SIZE = 4096;
static constexpr uint32_t PAGE_ENTRIES = 1024;
static constexpr uint32_t PAGE_PRESENT_RW = 0x3;
static constexpr uint32_t IDENTITY_MAP_LIMIT = 0x00400000;

alignas(PAGE_SIZE) static uint32_t page_directory[PAGE_ENTRIES];
alignas(PAGE_SIZE) static uint32_t first_page_table[PAGE_ENTRIES];

static inline uint32_t align_up(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void paging_init()
{
    for (uint32_t i = 0; i < PAGE_ENTRIES; ++i)
    {
        page_directory[i] = 0;
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT_RW;
    }

    page_directory[0] = reinterpret_cast<uint32_t>(first_page_table) | PAGE_PRESENT_RW;

    uint32_t pd_phys = reinterpret_cast<uint32_t>(page_directory);
    __asm__ volatile("mov %0, %%cr3" : : "r"(pd_phys) : "memory");

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

void kmalloc_init()
{
    uint32_t heap_start = align_up(reinterpret_cast<uint32_t>(&kernel_end), 8);

    if (heap_start < IDENTITY_MAP_LIMIT)
        g_kmalloc_cursor = heap_start;
    else
        g_kmalloc_cursor = 0;
}

void *kmalloc(uint32_t size)
{
    if (size == 0 || g_kmalloc_cursor == 0)
        return nullptr;

    uint32_t aligned_size = align_up(size, 8);
    uint32_t next = g_kmalloc_cursor + aligned_size;

    if (next > IDENTITY_MAP_LIMIT)
        return nullptr;

    void *result = reinterpret_cast<void *>(g_kmalloc_cursor);
    g_kmalloc_cursor = next;
    return result;
}
