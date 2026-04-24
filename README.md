# kernix

A minimal 32-bit C++ kernel with a simple VGA text interface.

Current kernel banner/version in runtime: `Kernix v0.0.6`.

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
- `modules/vga_buffer/vga_buffer.*` - VGA text output (80x25)
- `modules/keyboard/keyboard.*` - keyboard scancode decode, character ring buffer, Shift support
- `modules/interrupts/interrupts.*` - IDT setup, PIC remap, CPU exception handling, IRQ handlers (`IRQ0` timer, `IRQ1` keyboard)
- `modules/interrupts/interrupts_entry.asm` - ISR/IRQ assembly stubs (`isr0_stub`, `isr13_stub`, `isr14_stub`, `irq0_stub`, `irq1_stub`, `syscall_stub`)
- `modules/memory/memory.*` - paging setup, page mapping API (`map_page/unmap_page`), and kernel heap allocator (`kmalloc/kfree/kcalloc/krealloc`)
- `modules/string/string.*` - `strcmp` and space search in a string
- `modules/auth/auth.*` - auth module placeholder
- `modules/shell/shell.*` - shell module placeholder
- `modules/syscalls/syscalls.*` - syscall subsystem:
	- IDT vector `0x90` with ring3 privilege (DPL=3)
	- syscall dispatcher: `syscall_handler(eax, ebx, ecx, edx)` dispatches by syscall number in `eax`
	- kernel-side implementations: `ksys_write`, `ksys_getchar` (direct calls, not syscalls)
	- user-space wrappers: `sys_get_ticks` (0), `sys_write` (1), `sys_getchar` (2)
	- syscall entry stub: saves/restores DS/ES/FS/GS segments, passes registers as parameters to C++ handler
- `linker.ld` - links kernel at base address `0x100000`
- `Makefile` - build and run rules

## What Is Already Implemented
- Boot via multiboot header into 32-bit mode (`_start` entry)
- Stack initialization and `.bss` clearing
- VGA text output subsystem:
	- character/string printing, newline, screen clear, backspace
	- automatic scrolling when reaching bottom line (buffer shifts up, last row is cleared)
	- hardware cursor synchronization with text position and periodic blink
- Interrupt subsystem basics:
	- IDT with 256 entries
	- `lidt` load of IDT descriptor
	- PIC remap (`IRQ0 -> 32`, `IRQ1 -> 33`)
	- `sti` to enable hardware interrupts
- CPU exception handlers (minimum set):
	- Divide by zero (`#DE`, vector 0)
	- General protection fault (`#GP`, vector 13)
	- Page fault (`#PF`, vector 14)
	- For `#PF`, fault address is read from `CR2` and printed with error code
	- Exception path prints diagnostics and halts CPU (`cli` + `hlt` loop)
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
	- Runtime page mapping API: `map_page(virt, phys, flags)` and `unmap_page(virt)`
	- Page-table allocation pool for new virtual mappings
	- free-list heap allocator with 8-byte alignment and block split/coalesce
	- heap auto-growth: when no suitable free block exists, allocator maps additional pages and appends them to free-list tail
	- allocation API: `kmalloc(size)`, `kfree(ptr)`, `kcalloc(count, size)`, `krealloc(ptr, new_size)`, `ksize(ptr)`
	- allocator diagnostics: `kmalloc_total()`, `kmalloc_used()`, `kmalloc_remaining()`
- Keyboard input via interrupt-driven buffering (no polling path), with Shift support
- Keyboard input queue API (FIFO ring buffer):
	- `bool keyboard_available()`
	- `char keyboard_getchar()`
- Syscall subsystem (int 0x90):
	- IDT gate configured for ring3 (`DPL=3`, type attr `0xEE`)
	- ASM syscall entry (`syscall_stub`) saves/restores segment registers, pushes `eax/ebx/ecx/edx` as parameters
	- C++ dispatcher `syscall_handler(eax, ebx, ecx, edx)` uses switch on `eax` (syscall number)
	- Syscall numbers: `SYS_GET_TICKS (0)`, `SYS_WRITE (1)`, `SYS_GETCHAR (2)`
	- User-space wrappers invoke `int 0x90` with proper register setup via inline assembly
	- Kernel-side `ksys_*` functions provide actual implementation (write via vga_buffer, getchar via keyboard)
	- `help` - show commands
	- `clear` - clear screen
	- `about` - about project
	- `version` - kernel/shell versions
	- `ticks` - read timer via kernel function call
	- `systicks` - read timer via syscall `int 0x90` (SYS_GET_TICKS)
	- `syswrite <text>` - write text via syscall `int 0x90` (SYS_WRITE)
	- `mem` - heap statistics (total/used/free)
	- `panic` - trigger divide-by-zero exception test
	- `echo <text>` - echo text to screen

## What Is Missing
- No separate 16-bit bootloader file (`boot.asm`) and no disk loading path
- No wall-clock/time API yet (only raw `ticks` counter is available)
- No scheduler or multitasking
- No privilege level switching (ring0 ↔ ring3) yet; syscalls available but no user-mode isolation
- Limited syscall coverage (3 syscalls implemented; easy to add more via enum/dispatcher)
- No filesystem and no device drivers beyond basic keyboard input