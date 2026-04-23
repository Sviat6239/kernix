.PHONY: all clean

TARGET = kernix.bin
KERNEL_ELF = kernel.elf

# Tools
CC = i686-elf-gcc
CXX = i686-elf-g++
LD = i686-elf-ld
NASM = nasm
OBJCOPY = i686-elf-objcopy

# Flags
CXXFLAGS = -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra -std=c++17 -m32
LDFLAGS = -m elf_i386 -T linker.ld

# Objects
OBJS = kernel_entry.o kernel.o

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

clean:
	rm -f $(OBJS) $(KERNEL_ELF) $(TARGET)
	@echo "Cleaned up build artifacts"

run: $(TARGET)
	@echo "To run with QEMU:"
	@echo "  qemu-system-i386 -kernel $(TARGET)"
