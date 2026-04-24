#include <stdint.h>

#include "memory.hpp"

extern "C" uint8_t kernel_end;

static constexpr uint32_t PAGE_SIZE = 4096;
static constexpr uint32_t PAGE_ENTRIES = 1024;
static constexpr uint32_t PAGE_FRAME_MASK = 0xFFFFF000;
static constexpr uint32_t PAGE_FLAG_MASK = 0x00000FFF;
static constexpr uint32_t IDENTITY_MAP_LIMIT = 0x00400000;
static constexpr uint32_t PHYS_ALLOC_START = IDENTITY_MAP_LIMIT;
static constexpr uint32_t HEAP_VIRT_LIMIT = 0x04000000;
static constexpr uint32_t PAGE_TABLE_POOL_COUNT = 32;

static constexpr uint32_t ALLOC_ALIGN = 8;
static constexpr uint32_t BLOCK_MAGIC = 0x4B484541; // KHEA

struct HeapBlock
{
    uint32_t magic;
    uint32_t size;
    uint32_t free;
    HeapBlock *prev;
    HeapBlock *next;
};

alignas(PAGE_SIZE) static uint32_t page_directory[PAGE_ENTRIES];
alignas(PAGE_SIZE) static uint32_t page_table_pool[PAGE_TABLE_POOL_COUNT][PAGE_ENTRIES];
static bool page_table_used[PAGE_TABLE_POOL_COUNT];

static uint32_t g_next_free_phys = PHYS_ALLOC_START;
static uint32_t g_heap_start = 0;
static uint32_t g_heap_mapped_end = 0;
static HeapBlock *g_heap_head = nullptr;

static inline uint32_t align_up(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

static inline uint32_t align_down(uint32_t value, uint32_t alignment)
{
    return value & ~(alignment - 1);
}

static inline void invlpg(uint32_t addr)
{
    __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

static uint32_t *alloc_page_table(uint32_t &phys_out)
{
    for (uint32_t i = 0; i < PAGE_TABLE_POOL_COUNT; ++i)
    {
        if (page_table_used[i])
            continue;

        page_table_used[i] = true;

        uint32_t *table = page_table_pool[i];
        for (uint32_t j = 0; j < PAGE_ENTRIES; ++j)
            table[j] = 0;

        phys_out = reinterpret_cast<uint32_t>(table);
        return table;
    }

    phys_out = 0;
    return nullptr;
}

static uint32_t *get_page_table(uint32_t dir_index)
{
    uint32_t pde = page_directory[dir_index];
    if ((pde & PAGE_FLAG_PRESENT) == 0)
        return nullptr;

    return reinterpret_cast<uint32_t *>(pde & PAGE_FRAME_MASK);
}

static uint32_t *get_or_create_page_table(uint32_t dir_index)
{
    uint32_t *table = get_page_table(dir_index);
    if (table)
        return table;

    uint32_t table_phys;
    table = alloc_page_table(table_phys);
    if (!table)
        return nullptr;

    page_directory[dir_index] = (table_phys & PAGE_FRAME_MASK) | PAGE_FLAG_PRESENT | PAGE_FLAG_RW;
    return table;
}

void map_page(uint32_t virt, uint32_t phys, uint32_t flags)
{
    uint32_t vaddr = align_down(virt, PAGE_SIZE);
    uint32_t paddr = align_down(phys, PAGE_SIZE);

    uint32_t dir_index = (vaddr >> 22) & 0x3FF;
    uint32_t table_index = (vaddr >> 12) & 0x3FF;

    uint32_t *table = get_or_create_page_table(dir_index);
    if (!table)
        return;

    uint32_t page_flags = (flags & PAGE_FLAG_MASK) | PAGE_FLAG_PRESENT;
    table[table_index] = (paddr & PAGE_FRAME_MASK) | page_flags;
    invlpg(vaddr);
}

void unmap_page(uint32_t virt)
{
    uint32_t vaddr = align_down(virt, PAGE_SIZE);
    uint32_t dir_index = (vaddr >> 22) & 0x3FF;
    uint32_t table_index = (vaddr >> 12) & 0x3FF;

    uint32_t *table = get_page_table(dir_index);
    if (!table)
        return;

    table[table_index] = 0;
    invlpg(vaddr);
}

static inline uint32_t header_size()
{
    return align_up(static_cast<uint32_t>(sizeof(HeapBlock)), ALLOC_ALIGN);
}

static inline uint32_t min_split_payload()
{
    return ALLOC_ALIGN;
}

static inline uint8_t *block_payload(HeapBlock *block)
{
    return reinterpret_cast<uint8_t *>(block) + header_size();
}

static inline bool ptr_in_heap(const void *ptr)
{
    uint32_t p = reinterpret_cast<uint32_t>(ptr);
    return p >= g_heap_start && p < g_heap_mapped_end;
}

static HeapBlock *last_block()
{
    HeapBlock *block = g_heap_head;
    if (!block)
        return nullptr;

    while (block->next)
        block = block->next;

    return block;
}

static HeapBlock *find_free_block(uint32_t required_size)
{
    HeapBlock *block = g_heap_head;
    while (block)
    {
        if (block->magic == BLOCK_MAGIC && block->free && block->size >= required_size)
            return block;
        block = block->next;
    }

    return nullptr;
}

static void split_block(HeapBlock *block, uint32_t required_size)
{
    uint32_t hsize = header_size();
    if (block->size < required_size + hsize + min_split_payload())
        return;

    uint8_t *new_block_addr = block_payload(block) + required_size;
    HeapBlock *new_block = reinterpret_cast<HeapBlock *>(new_block_addr);

    new_block->magic = BLOCK_MAGIC;
    new_block->size = block->size - required_size - hsize;
    new_block->free = 1;
    new_block->prev = block;
    new_block->next = block->next;

    if (new_block->next)
        new_block->next->prev = new_block;

    block->size = required_size;
    block->next = new_block;
}

static void coalesce_with_next(HeapBlock *block)
{
    HeapBlock *next = block->next;
    if (!next || !next->free || next->magic != BLOCK_MAGIC)
        return;

    block->size += header_size() + next->size;
    block->next = next->next;
    if (block->next)
        block->next->prev = block;
}

static HeapBlock *block_from_ptr(const void *ptr)
{
    if (!ptr || !ptr_in_heap(ptr))
        return nullptr;

    const uint8_t *raw = reinterpret_cast<const uint8_t *>(ptr);
    if (reinterpret_cast<uint32_t>(raw) < g_heap_start + header_size())
        return nullptr;

    HeapBlock *block = reinterpret_cast<HeapBlock *>(const_cast<uint8_t *>(raw - header_size()));
    if (!ptr_in_heap(block) || block->magic != BLOCK_MAGIC)
        return nullptr;

    return block;
}

static bool extend_heap_to_fit(uint32_t required_size)
{
    if (!g_heap_head)
        return false;

    uint32_t hsize = header_size();
    HeapBlock *tail = last_block();
    if (!tail)
        return false;

    uint32_t needed_payload = required_size;
    if (tail->free)
    {
        if (tail->size >= needed_payload)
            return true;
        needed_payload -= tail->size;
    }
    else
    {
        needed_payload += hsize;
    }

    uint32_t bytes_needed = align_up(needed_payload, PAGE_SIZE);
    uint32_t pages_needed = bytes_needed / PAGE_SIZE;

    for (uint32_t i = 0; i < pages_needed; ++i)
    {
        if (g_heap_mapped_end >= HEAP_VIRT_LIMIT)
            return false;

        map_page(g_heap_mapped_end, g_next_free_phys, PAGE_FLAG_RW);
        g_heap_mapped_end += PAGE_SIZE;
        g_next_free_phys += PAGE_SIZE;
    }

    uint32_t added_bytes = pages_needed * PAGE_SIZE;
    if (tail->free)
    {
        tail->size += added_bytes;
    }
    else
    {
        uint8_t *new_block_addr = reinterpret_cast<uint8_t *>(tail) + hsize + tail->size;
        HeapBlock *new_block = reinterpret_cast<HeapBlock *>(new_block_addr);
        new_block->magic = BLOCK_MAGIC;
        new_block->size = added_bytes - hsize;
        new_block->free = 1;
        new_block->prev = tail;
        new_block->next = nullptr;
        tail->next = new_block;
    }

    return true;
}

void paging_init()
{
    for (uint32_t i = 0; i < PAGE_ENTRIES; ++i)
        page_directory[i] = 0;

    for (uint32_t i = 0; i < PAGE_TABLE_POOL_COUNT; ++i)
        page_table_used[i] = false;

    uint32_t first_table_phys;
    uint32_t *first_table = alloc_page_table(first_table_phys);
    if (!first_table)
        return;

    for (uint32_t i = 0; i < PAGE_ENTRIES; ++i)
        first_table[i] = (i * PAGE_SIZE) | PAGE_FLAG_PRESENT | PAGE_FLAG_RW;

    page_directory[0] = (first_table_phys & PAGE_FRAME_MASK) | PAGE_FLAG_PRESENT | PAGE_FLAG_RW;

    uint32_t pd_phys = reinterpret_cast<uint32_t>(page_directory);
    __asm__ volatile("mov %0, %%cr3" : : "r"(pd_phys) : "memory");

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

void kmalloc_init()
{
    uint32_t hsize = header_size();
    uint32_t heap_start = align_up(reinterpret_cast<uint32_t>(&kernel_end), ALLOC_ALIGN);

    if (heap_start + hsize + min_split_payload() >= IDENTITY_MAP_LIMIT)
    {
        g_heap_start = 0;
        g_heap_mapped_end = 0;
        g_heap_head = nullptr;
        return;
    }

    g_next_free_phys = align_up(PHYS_ALLOC_START, PAGE_SIZE);
    g_heap_start = heap_start;
    g_heap_mapped_end = IDENTITY_MAP_LIMIT;

    g_heap_head = reinterpret_cast<HeapBlock *>(g_heap_start);
    g_heap_head->magic = BLOCK_MAGIC;
    g_heap_head->size = g_heap_mapped_end - g_heap_start - hsize;
    g_heap_head->free = 1;
    g_heap_head->prev = nullptr;
    g_heap_head->next = nullptr;
}

void *kmalloc(uint32_t size)
{
    if (size == 0 || !g_heap_head)
        return nullptr;

    uint32_t aligned_size = align_up(size, ALLOC_ALIGN);
    HeapBlock *block = find_free_block(aligned_size);

    if (!block)
    {
        if (!extend_heap_to_fit(aligned_size))
            return nullptr;

        block = find_free_block(aligned_size);
        if (!block)
            return nullptr;
    }

    split_block(block, aligned_size);
    block->free = 0;
    return block_payload(block);
}

void kfree(void *ptr)
{
    HeapBlock *block = block_from_ptr(ptr);
    if (!block || block->free)
        return;

    block->free = 1;
    coalesce_with_next(block);

    if (block->prev && block->prev->free)
        coalesce_with_next(block->prev);
}

void *kcalloc(uint32_t count, uint32_t size)
{
    if (count == 0 || size == 0)
        return nullptr;

    uint64_t total64 = static_cast<uint64_t>(count) * static_cast<uint64_t>(size);
    if (total64 > 0xFFFFFFFFULL)
        return nullptr;

    uint32_t total = static_cast<uint32_t>(total64);
    void *ptr = kmalloc(total);
    if (!ptr)
        return nullptr;

    uint8_t *bytes = reinterpret_cast<uint8_t *>(ptr);
    for (uint32_t i = 0; i < total; ++i)
        bytes[i] = 0;

    return ptr;
}

uint32_t ksize(const void *ptr)
{
    HeapBlock *block = block_from_ptr(ptr);
    if (!block || block->free)
        return 0;

    return block->size;
}

void *krealloc(void *ptr, uint32_t new_size)
{
    if (!ptr)
        return kmalloc(new_size);

    if (new_size == 0)
    {
        kfree(ptr);
        return nullptr;
    }

    HeapBlock *block = block_from_ptr(ptr);
    if (!block || block->free)
        return nullptr;

    uint32_t aligned_size = align_up(new_size, ALLOC_ALIGN);
    if (aligned_size <= block->size)
    {
        split_block(block, aligned_size);
        return ptr;
    }

    if (block->next && block->next->free && block->next->magic == BLOCK_MAGIC)
    {
        uint32_t combined = block->size + header_size() + block->next->size;
        if (combined >= aligned_size)
        {
            coalesce_with_next(block);
            split_block(block, aligned_size);
            return ptr;
        }
    }

    void *new_ptr = kmalloc(aligned_size);
    if (!new_ptr)
        return nullptr;

    uint8_t *dst = reinterpret_cast<uint8_t *>(new_ptr);
    uint8_t *src = reinterpret_cast<uint8_t *>(ptr);
    for (uint32_t i = 0; i < block->size; ++i)
        dst[i] = src[i];

    kfree(ptr);
    return new_ptr;
}

uint32_t kmalloc_total()
{
    if (!g_heap_head)
        return 0;

    uint32_t total = 0;
    HeapBlock *block = g_heap_head;
    while (block)
    {
        if (block->magic != BLOCK_MAGIC)
            break;

        total += block->size;
        block = block->next;
    }

    return total;
}

uint32_t kmalloc_remaining()
{
    if (!g_heap_head)
        return 0;

    uint32_t total_free = 0;
    HeapBlock *block = g_heap_head;
    while (block)
    {
        if (block->magic != BLOCK_MAGIC)
            break;

        if (block->free)
            total_free += block->size;

        block = block->next;
    }

    return total_free;
}

uint32_t kmalloc_used()
{
    if (!g_heap_head)
        return 0;

    uint32_t total_used = 0;
    HeapBlock *block = g_heap_head;
    while (block)
    {
        if (block->magic != BLOCK_MAGIC)
            break;

        if (!block->free)
            total_used += block->size;

        block = block->next;
    }

    return total_used;
}
