# kernix

A minimal 32-bit C++ kernel with a simple VGA text interface.

## Build Requirements
- `nasm`
- Cross-toolchain `i686-elf-*` is recommended (`gcc`, `g++`, `ld`, `objcopy`)
- If `i686-elf-g++` is not found, Makefile tries to use system `gcc/g++/ld/objcopy`

## Build
```bash
make
```

## Clean
```bash
make clean
```

## Run in QEMU
```bash
make run
```

## Structure
- `kernel_entry.asm` - `_start` entry point, multiboot header, stack setup, `.bss` clear, call to `kernel_main`
- `kernel.cpp` - main kernel loop and simple command interface
- `modules/vga_buffer.*` - VGA text output (80x25)
- `modules/keyboard.*` - keyboard polling via ports `0x60/0x64`, character buffer, Shift support
- `modules/string.*` - `strcmp` and space search in a string
- `linker.ld` - links kernel at base address `0x100000`
- `Makefile` - build and run rules

## What Is Already Implemented
- Boot via multiboot header into 32-bit mode (`_start` entry)
- Stack initialization and `.bss` clearing
- Basic VGA output: character/string printing, newline, screen clear, backspace
- Keyboard input (polling without interrupts), input buffering, Shift support
- Simple command console:
	- `help`
	- `clear`
	- `about`
	- `version`
	- `echo <text>`

## What Is Missing
- No separate 16-bit bootloader file (`boot.asm`) and no disk loading path
- No IDT/IRQ and interrupt handling (keyboard works via polling)
- No timer (PIT), scheduler, or multitasking
- No memory manager (paging/heap/allocator)
- No user mode or system calls
- No filesystem and no device drivers beyond basic keyboard input
- No VGA scrolling (when reaching the bottom, cursor stays on the last line)