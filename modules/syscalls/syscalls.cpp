#include "syscalls.hpp"
#include "../interrupts/interrupts.hpp"
#include "../vga_buffer/vga_buffer.hpp"
#include "../keyboard/keyboard.hpp"
#include "../memory/memory.hpp"

// ============================================================================
// KERNEL-SIDE IMPLEMENTATIONS (direct kernel functions, not syscalls)
// ============================================================================

int32_t ksys_write(const char *buffer, uint32_t size)
{
    if (!buffer)
        return -1;

    for (uint32_t i = 0; i < size; ++i)
        vga_buffer.putchar(buffer[i]);

    return static_cast<int32_t>(size);
}

uint32_t ksys_getchar()
{
    if (keyboard_available())
        return keyboard_getchar();

    return 0xFFFFFFFFu;
}

int32_t ksys_putchar(char c)
{
    vga_buffer.putchar(c);
    return 0;
}

void ksys_clear_screen()
{
    vga_buffer.clear();
}

// ============================================================================
// SYSCALL DISPATCHER (called by int 0x90 handler from ASM)
// ============================================================================

extern "C" uint32_t syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx [[maybe_unused]])
{
    switch (eax)
    {
    case SYS_GET_TICKS:
        return interrupts_get_ticks();

    case SYS_WRITE:
        return static_cast<uint32_t>(ksys_write(reinterpret_cast<const char *>(ebx), ecx));

    case SYS_GETCHAR:
        return ksys_getchar();

    case SYS_PUTCHAR:
        ksys_putchar(static_cast<char>(ebx));
        return 0;

    case SYS_CLEAR_SCREEN:
        ksys_clear_screen();
        return 0;

    case SYS_KMALLOC:
        return reinterpret_cast<uint32_t>(kmalloc(ebx));

    case SYS_KFREE:
        kfree(reinterpret_cast<void *>(ebx));
        return 0;

    case SYS_KCALLOC:
        return reinterpret_cast<uint32_t>(kcalloc(ebx, ecx));

    case SYS_KREALLOC:
        return reinterpret_cast<uint32_t>(krealloc(reinterpret_cast<void *>(ebx), ecx));

    case SYS_KSIZE:
        return ksize(reinterpret_cast<const void *>(ebx));

    case SYS_KMALLOC_TOTAL:
        return kmalloc_total();

    case SYS_KMALLOC_USED:
        return kmalloc_used();

    case SYS_KMALLOC_REMAINING:
        return kmalloc_remaining();

    default:
        return 0xFFFFFFFFu;
    }
}

// ============================================================================
// USER-SPACE SYSCALL WRAPPERS (invoke int 0x90)
// ============================================================================

uint32_t sys_get_ticks()
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_GET_TICKS)
        : "memory");
    return result;
}

int32_t sys_write(const char *buffer, uint32_t size)
{
    int32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_WRITE), "b"(buffer), "c"(size)
        : "memory", "cc");
    return result;
}

uint32_t sys_getchar()
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_GETCHAR)
        : "memory");
    return result;
}

int32_t sys_putchar(char c)
{
    int32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_PUTCHAR), "b"(c)
        : "memory", "cc");
    return result;
}

void sys_clear_screen()
{
    __asm__ volatile(
        "int $0x90"
        :
        : "a"(SYS_CLEAR_SCREEN)
        : "memory");
}

void *sys_kmalloc(uint32_t size)
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_KMALLOC), "b"(size)
        : "memory", "cc");
    return reinterpret_cast<void *>(result);
}

void sys_kfree(void *ptr)
{
    __asm__ volatile(
        "int $0x90"
        :
        : "a"(SYS_KFREE), "b"(ptr)
        : "memory", "cc");
}

void *sys_kcalloc(uint32_t count, uint32_t size)
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_KCALLOC), "b"(count), "c"(size)
        : "memory", "cc");
    return reinterpret_cast<void *>(result);
}

void *sys_krealloc(void *ptr, uint32_t new_size)
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_KREALLOC), "b"(ptr), "c"(new_size)
        : "memory", "cc");
    return reinterpret_cast<void *>(result);
}

uint32_t sys_ksize(const void *ptr)
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_KSIZE), "b"(ptr)
        : "memory", "cc");
    return result;
}

uint32_t sys_kmalloc_total()
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_KMALLOC_TOTAL)
        : "memory", "cc");
    return result;
}

uint32_t sys_kmalloc_used()
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_KMALLOC_USED)
        : "memory", "cc");
    return result;
}

uint32_t sys_kmalloc_remaining()
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYS_KMALLOC_REMAINING)
        : "memory", "cc");
    return result;
}