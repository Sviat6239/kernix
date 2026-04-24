#include "syscalls.hpp"

#include "../interrupts/interrupts.hpp"

extern "C" uint32_t syscall_handler(uint32_t syscall_number)
{
    if (syscall_number == SYSCALL_GET_TICKS)
    {
        return interrupts_get_ticks();
    }

    return 0xFFFFFFFFu;
}

uint32_t sys_get_ticks()
{
    uint32_t result;
    __asm__ volatile(
        "int $0x90"
        : "=a"(result)
        : "a"(SYSCALL_GET_TICKS)
        : "memory");
    return result;
}
