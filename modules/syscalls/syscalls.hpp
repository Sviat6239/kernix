#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

#include <stdint.h>

enum SyscallNumber : uint32_t
{
    SYSCALL_GET_TICKS = 0,
};

// Called by the interrupt stub for int 0x90.
extern "C" uint32_t syscall_handler(uint32_t syscall_number);

// Kernel-side example wrapper that invokes int 0x90.
uint32_t sys_get_ticks();

#endif
