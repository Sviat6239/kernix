.PHONY: all clean run

TARGET = kernix.bin
KERNEL_ELF = kernel.elf

# Tools
ifeq ($(shell command -v i686-elf-g++ >/dev/null 2>&1; echo $$?),0)
TOOLCHAIN_PREFIX = i686-elf-
else
TOOLCHAIN_PREFIX =
endif

CC = $(TOOLCHAIN_PREFIX)gcc
CXX = $(TOOLCHAIN_PREFIX)g++
LD = $(TOOLCHAIN_PREFIX)ld
NASM = nasm
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy

# Flags
CXXFLAGS = -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra -std=c++17 -m32
LDFLAGS = -m elf_i386 -T linker.ld

# Objects
OBJS = kernel_entry.o kernel.o modules/vga_buffer/vga_buffer.o modules/keyboard/keyboard.o modules/string/string.o modules/interrupts/interrupts.o modules/interrupts/interrupts_entry.o modules/memory/memory.o modules/auth/auth.o modules/shell/shell.o modules/fs/fs.o modules/syscalls/syscalls.o

all: $(TARGET)

$(TARGET): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "Kernel binary created: $@"

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel ELF created: $@"

kernel_entry.o: kernel_entry.asm
	$(NASM) -f elf32 -o $@ $<

kernel.o: kernel.cpp modules/vga_buffer/vga_buffer.hpp modules/keyboard/keyboard.hpp modules/string/string.hpp modules/interrupts/interrupts.hpp modules/memory/memory.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/vga_buffer/vga_buffer.o: modules/vga_buffer/vga_buffer.cpp modules/vga_buffer/vga_buffer.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/keyboard/keyboard.o: modules/keyboard/keyboard.cpp modules/keyboard/keyboard.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/string/string.o: modules/string/string.cpp modules/string/string.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/interrupts/interrupts.o: modules/interrupts/interrupts.cpp modules/interrupts/interrupts.hpp modules/keyboard/keyboard.hpp modules/vga_buffer/vga_buffer.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/interrupts/interrupts_entry.o: modules/interrupts/interrupts_entry.asm
	$(NASM) -f elf32 -o $@ $<

modules/memory/memory.o: modules/memory/memory.cpp modules/memory/memory.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/auth/auth.o: modules/auth/auth.cpp modules/auth/auth.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/shell/shell.o: modules/shell/shell.cpp modules/shell/shell.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/fs/fs.o: modules/fs/fs.cpp modules/fs/fs.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/syscalls/syscalls.o: modules/syscalls/syscalls.cpp modules/syscalls/syscalls.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(KERNEL_ELF) $(TARGET)
	@echo "Cleaned up build artifacts"

run: $(KERNEL_ELF)
	qemu-system-i386 -kernel $(KERNEL_ELF)
