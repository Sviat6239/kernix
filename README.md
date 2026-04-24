# kernix

A minimal 32-bit C++ kernel with a simple VGA text interface.

Current kernel banner/version in runtime: `Kernix v0.4`.

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
- `kernel.cpp` - kernel initialization and simple command interface
- `modules/vga_buffer.*` - VGA text output (80x25)
- `modules/keyboard.*` - keyboard scancode decode, character ring buffer, Shift support
- `modules/interrupts.*` - IDT setup, PIC remap, IRQ handlers (`IRQ0` timer, `IRQ1` keyboard)
- `modules/interrupts_entry.asm` - IRQ assembly stubs (`irq0_stub`, `irq1_stub`)
- `modules/memory.*` - paging setup (4MB identity map) and simple `kmalloc` bump allocator
- `modules/string.*` - `strcmp` and space search in a string
- `linker.ld` - links kernel at base address `0x100000`
- `Makefile` - build and run rules

## What Is Already Implemented
- Boot via multiboot header into 32-bit mode (`_start` entry)
- Stack initialization and `.bss` clearing
- Basic VGA output: character/string printing, newline, screen clear, backspace
- Interrupt subsystem basics:
	- IDT with 256 entries
	- `lidt` load of IDT descriptor
	- PIC remap (`IRQ0 -> 32`, `IRQ1 -> 33`)
	- `sti` to enable hardware interrupts
- IRQ handlers:
	- `IRQ0` timer increments `ticks`
	- `IRQ1` keyboard reads scancode from port `0x60`
- PIT initialization for timer base:
	- Channel 0 configured in mode `0x36`
	- Frequency set to `100 Hz`
- Memory subsystem basics:
	- Paging enabled via CR3/CR0
	- Page directory + first page table
	- Identity mapping for first `4 MB`
	- `kmalloc_init()` and `kmalloc(size)` (bump allocator, no free)
- Keyboard input via interrupt-driven buffering (no polling path), with Shift support
- Keyboard input queue API (FIFO ring buffer):
	- `bool keyboard_available()`
	- `char keyboard_getchar()`
- Simple command console:
	- `help`
	- `clear`
	- `about`
	- `version`
	- `echo <text>`

## What Is Missing
- No separate 16-bit bootloader file (`boot.asm`) and no disk loading path
- No CPU exception handlers (e.g. divide-by-zero/page fault diagnostics)
- No wall-clock/time API yet (only raw `ticks` counter is available)
- No scheduler or multitasking
- No free/allocator metadata in heap manager (only bump allocation)
- No dynamic mapping beyond the initial 4MB identity map
- No user mode or system calls
- No filesystem and no device drivers beyond basic keyboard input
- No VGA scrolling (when reaching the bottom, cursor stays on the last line)