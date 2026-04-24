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
OBJS = kernel_entry.o kernel.o modules/vga_buffer.o modules/keyboard.o modules/string.o modules/interrupts.o modules/interrupts_entry.o

all: $(TARGET)

$(TARGET): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "Kernel binary created: $@"

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel ELF created: $@"

kernel_entry.o: kernel_entry.asm
	$(NASM) -f elf32 -o $@ $<

kernel.o: kernel.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/vga_buffer.o: modules/vga_buffer.cpp modules/vga_buffer.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/keyboard.o: modules/keyboard.cpp modules/keyboard.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/string.o: modules/string.cpp modules/string.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/interrupts.o: modules/interrupts.cpp modules/interrupts.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

modules/interrupts_entry.o: modules/interrupts_entry.asm
	$(NASM) -f elf32 -o $@ $<

clean:
	rm -f $(OBJS) $(KERNEL_ELF) $(TARGET)
	@echo "Cleaned up build artifacts"

run: $(KERNEL_ELF)
	qemu-system-i386 -kernel $(KERNEL_ELF)
