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
    SYS_KMALLOC = 5,
    SYS_KFREE = 6,
    SYS_KCALLOC = 7,
    SYS_KREALLOC = 8,
    SYS_KSIZE = 9,
    SYS_KMALLOC_TOTAL = 10,
    SYS_KMALLOC_USED = 11,
    SYS_KMALLOC_REMAINING = 12,
    SYS_SHELL_EXEC = 13,
};

// Kernel-side syscall handler (called from int 0x90 stub)
extern "C" uint32_t syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Kernel-side implementations (NOT syscalls, direct kernel calls)
int32_t ksys_write(const char *buffer, uint32_t size);
uint32_t ksys_getchar();
int32_t ksys_putchar(char c);
void ksys_clear_screen();
void *kmalloc(uint32_t size);
void kfree(void *ptr);
void *kcalloc(uint32_t count, uint32_t size);
void *krealloc(void *ptr, uint32_t new_size);
uint32_t ksize(const void *ptr);
uint32_t kmalloc_total();
uint32_t kmalloc_used();
uint32_t kmalloc_remaining();
int32_t ksys_shell_exec(char *command);

// User-space syscall wrappers (invoke int 0x90)
uint32_t sys_get_ticks();
int32_t sys_write(const char *buffer, uint32_t size);
uint32_t sys_getchar();
int32_t sys_putchar(char c);
void sys_clear_screen();
void *sys_kmalloc(uint32_t size);
void sys_kfree(void *ptr);
void *sys_kcalloc(uint32_t count, uint32_t size);
void *sys_krealloc(void *ptr, uint32_t new_size);
uint32_t sys_ksize(const void *ptr);
uint32_t sys_kmalloc_total();
uint32_t sys_kmalloc_used();
uint32_t sys_kmalloc_remaining();
int32_t sys_shell_exec(char *command);

#endif
