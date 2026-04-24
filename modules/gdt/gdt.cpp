#include "gdt.hpp"

struct __attribute__((packed)) gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

struct __attribute__((packed)) gdt_ptr
{
    uint16_t limit;
    uint32_t base;
};

struct __attribute__((packed)) tss_entry
{
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
};

static constexpr uint32_t GDT_ENTRY_COUNT = 6;

static gdt_entry g_gdt[GDT_ENTRY_COUNT];
static gdt_ptr g_gdt_ptr;
static tss_entry g_tss;

static void gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    g_gdt[index].limit_low = static_cast<uint16_t>(limit & 0xFFFF);
    g_gdt[index].base_low = static_cast<uint16_t>(base & 0xFFFF);
    g_gdt[index].base_mid = static_cast<uint8_t>((base >> 16) & 0xFF);
    g_gdt[index].access = access;
    g_gdt[index].granularity = static_cast<uint8_t>(((limit >> 16) & 0x0F) | (flags & 0xF0));
    g_gdt[index].base_high = static_cast<uint8_t>((base >> 24) & 0xFF);
}

static void write_tss(uint32_t kernel_stack_top)
{
    const uint32_t base = reinterpret_cast<uint32_t>(&g_tss);
    const uint32_t limit = sizeof(tss_entry) - 1;

    gdt_set_entry(5, base, limit, 0x89, 0x00);

    uint8_t *bytes = reinterpret_cast<uint8_t *>(&g_tss);
    for (uint32_t i = 0; i < sizeof(tss_entry); ++i)
        bytes[i] = 0;

    g_tss.ss0 = KERNEL_DS_SELECTOR;
    g_tss.esp0 = kernel_stack_top;
    g_tss.cs = USER_CS_SELECTOR;
    g_tss.ss = USER_DS_SELECTOR;
    g_tss.ds = USER_DS_SELECTOR;
    g_tss.es = USER_DS_SELECTOR;
    g_tss.fs = USER_DS_SELECTOR;
    g_tss.gs = USER_DS_SELECTOR;
    g_tss.iomap_base = static_cast<uint16_t>(sizeof(tss_entry));
}

void tss_set_kernel_stack(uint32_t kernel_stack_top)
{
    g_tss.esp0 = kernel_stack_top;
}

void gdt_tss_init(uint32_t kernel_stack_top)
{
    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC0);
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC0);
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xC0);
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xC0);
    write_tss(kernel_stack_top);

    g_gdt_ptr.limit = static_cast<uint16_t>(sizeof(g_gdt) - 1);
    g_gdt_ptr.base = reinterpret_cast<uint32_t>(&g_gdt[0]);

    __asm__ volatile("lgdt (%0)" : : "r"(&g_gdt_ptr) : "memory");

    __asm__ volatile(
        "movw %0, %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "movw %%ax, %%ss\n\t"
        "ljmp %1, $1f\n\t"
        "1:\n\t"
        :
        : "i"(KERNEL_DS_SELECTOR), "i"(KERNEL_CS_SELECTOR)
        : "ax", "memory");

    __asm__ volatile(
        "movw %0, %%ax\n\t"
        "ltr %%ax\n\t"
        :
        : "i"(TSS_SELECTOR)
        : "ax", "memory");
}