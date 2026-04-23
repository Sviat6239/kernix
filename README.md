# kernix
Simple minimal microkernel in C++

## Build Requirements
- `i686-elf-gcc` and `i686-elf-g++` (cross-compiler)
- `i686-elf-ld` (cross-linker)
- `nasm` (assembler)

## Building
```bash
make
```

## Cleaning
```bash
make clean
```

## Running with QEMU
```bash
make run
# OR
qemu-system-i386 -kernel kernix.bin
```

## Architecture
- **boot.asm** - 16-bit BIOS bootloader (loads kernel from disk)
- **kernel_entry.asm** - 32-bit entry point (sets up stack, calls kernel_main)
- **kernel.cpp** - Main kernel code with VGA buffer output
- **linker.ld** - Linker script (kernel loaded at 0x100000)
- **Makefile** - Build configuration

## Kernel Features
- Multiboot compliant entry point
- Basic VGA text buffer output (80x25)
- Stack setup
- BSS section clearing
- Exception handling for pure virtual functions