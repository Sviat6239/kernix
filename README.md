# kernix

Minimal 32-bit C++ kernel with VGA text output, interrupts, paging, heap allocator and a tiny command shell.

Current runtime banner: Kernix v0.0.7.

## Build Requirements

- nasm
- qemu-system-i386 (for run target)
- i686-elf toolchain is preferred (gcc, g++, ld, objcopy)
- fallback to host gcc/g++/ld/objcopy is enabled in Makefile when i686-elf-g++ is not found

## Build And Run

```bash
make
make run
```

## Clean

```bash
make clean
```

## Project Layout

- kernel_entry.asm: multiboot header, _start entry, stack setup, .bss zeroing, call to kernel_main
- kernel.cpp: core initialization flow and main input loop
- linker.ld: links kernel at 0x100000
- Makefile: build, clean and QEMU run targets
- modules/vga_buffer: VGA text buffer output, cursor sync, cursor blink, screen scrolling
- modules/keyboard: IRQ-driven keyboard input buffer with Shift handling
- modules/interrupts: IDT/PIC/PIT setup, exception handlers, IRQ handlers, syscall gate setup
- modules/interrupts/interrupts_entry.asm: stubs for exceptions, IRQ0/IRQ1 and syscall entry
- modules/memory: paging and heap allocator (kmalloc family)
- modules/syscalls: syscall numbers, dispatcher, wrappers, direct ksys helpers
- modules/shell: command parsing and command handlers
- modules/calc: shell calculator command
- modules/string: strcmp, find_space, strlen, ksys_print
- modules/auth: placeholder (empty)
- modules/fs: placeholder (empty)

## Boot/Init Flow

Kernel initialization order in kernel_main:

1. vga_init
2. paging_init
3. kmalloc_init
4. keyboard_init
5. interrupts_init
6. interrupts_enable

Then the shell prompt starts and input is processed in a loop.

## Implemented Features

- 32-bit multiboot-compatible entry and kernel startup
- VGA output:
  - putchar/print/clear
  - newline and backspace handling
  - hardware cursor sync and blink driven by timer ticks
  - scrolling on bottom line overflow
- Interrupts:
  - IDT with installed handlers for #DE, #GP, #PF
  - PIC remap for IRQ0 and IRQ1
  - PIT configured to 100 Hz
  - IRQ0 timer tick counter
  - IRQ1 keyboard scancode handling
  - syscall gate on int 0x90 with DPL=3
- Memory:
  - paging enabled via CR3/CR0
  - identity map for first 4 MB
  - map_page and unmap_page APIs
  - free-list heap allocator with split/coalesce
  - kmalloc, kfree, kcalloc, krealloc, ksize
  - heap stats: kmalloc_total, kmalloc_used, kmalloc_remaining
  - heap growth by mapping extra pages when needed
- Syscalls:
  - dispatcher: syscall_handler(eax, ebx, ecx, edx)
  - implemented numbers in dispatcher: GET_TICKS(0), WRITE(1), GETCHAR(2), PUTCHAR(3)
  - wrappers available: sys_get_ticks, sys_write, sys_getchar, sys_putchar, sys_clear_screen
- Shell with command parser and built-in commands
- Calculator module for basic arithmetic

## Shell Commands

- help
- clear
- about
- version (aliases: ver, -v)
- ticks
- systicks
- syswrite <text>
- mem
- panic
- echo <text>
- calc <num1> <op> <num2>

Notes:

- syswrite currently prints extra debug text before syscall result.
- calc supports operators: +, -, *, /.

## Known Gaps / TODO

- No user-mode task/process model yet (syscall gate exists, but no full ring3 runtime)
- No scheduler or multitasking
- No filesystem implementation yet (modules/fs is empty)
- No auth implementation yet (modules/auth is empty)
- SYS_CLEAR_SCREEN is declared in enum/wrapper, but not handled in syscall dispatcher