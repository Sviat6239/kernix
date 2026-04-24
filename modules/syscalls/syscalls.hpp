#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

#include <stdint.h>

// Syscall numbers
enum SyscallNumber : uint32_t
{
    SYS_GET_TICKS = 0,
    SYS_WRITE = 1,
    SYS_GETCHAR = 2,
    SYS_PUTCHAR = 3,
    SYS_CLEAR_SCREEN = 4,
};

// Kernel-side syscall handler (called from int 0x90 stub)
extern "C" uint32_t syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Kernel-side implementations (NOT syscalls, direct kernel calls)
int32_t ksys_write(const char *buffer, uint32_t size);
uint32_t ksys_getchar();
int32_t ksys_putchar(char c);
void ksys_clear_screen();

// User-space syscall wrappers (invoke int 0x90)
uint32_t sys_get_ticks();
int32_t sys_write(const char *buffer, uint32_t size);
uint32_t sys_getchar();
int32_t sys_putchar(char c);
void sys_clear_screen();

#endif
