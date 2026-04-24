#include <stdint.h>

#include "interrupts.hpp"
#include "keyboard.hpp"

#define IDT_ENTRIES 256

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI 0x20

#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10
#define ICW4_8086 0x01

#define IRQ0_VECTOR 32
#define IRQ1_VECTOR 33

struct __attribute__((packed)) idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
};

struct __attribute__((packed)) idt_ptr
{
    uint16_t limit;
    uint32_t base;
};

static idt_entry g_idt[IDT_ENTRIES];
static idt_ptr g_idt_ptr;
static volatile uint32_t g_ticks = 0;

extern "C" void irq0_stub();
extern "C" void irq1_stub();

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void io_wait()
{
    outb(0x80, 0);
}

static void idt_set_gate(uint8_t vector, uint32_t handler)
{
    g_idt[vector].offset_low = static_cast<uint16_t>(handler & 0xFFFF);
    g_idt[vector].selector = 0x08;
    g_idt[vector].zero = 0;
    g_idt[vector].type_attr = 0x8E;
    g_idt[vector].offset_high = static_cast<uint16_t>((handler >> 16) & 0xFFFF);
}

static void idt_load()
{
    g_idt_ptr.limit = static_cast<uint16_t>(sizeof(g_idt) - 1);
    g_idt_ptr.base = reinterpret_cast<uint32_t>(&g_idt[0]);
    __asm__ volatile("lidt %0" : : "m"(g_idt_ptr));
}

static void pic_remap(uint8_t offset1, uint8_t offset2)
{
    uint8_t mask2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Unmask only IRQ0 (timer) and IRQ1 (keyboard) on the master PIC.
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, mask2);
}

void interrupts_init()
{
    for (int i = 0; i < IDT_ENTRIES; ++i)
    {
        g_idt[i].offset_low = 0;
        g_idt[i].selector = 0;
        g_idt[i].zero = 0;
        g_idt[i].type_attr = 0;
        g_idt[i].offset_high = 0;
    }

    pic_remap(IRQ0_VECTOR, 40);

    idt_set_gate(IRQ0_VECTOR, reinterpret_cast<uint32_t>(irq0_stub));
    idt_set_gate(IRQ1_VECTOR, reinterpret_cast<uint32_t>(irq1_stub));

    idt_load();
}

void interrupts_enable()
{
    __asm__ volatile("sti");
}

uint32_t interrupts_get_ticks()
{
    return g_ticks;
}

extern "C" void irq0_handler()
{
    ++g_ticks;
    outb(PIC1_COMMAND, PIC_EOI);
}

extern "C" void irq1_handler()
{
    uint8_t scancode = inb(0x60);
    keyboard_handle_scancode(scancode);
    outb(PIC1_COMMAND, PIC_EOI);
}
