FILES = build/kernel/kernel.asm.o \
		build/kernel/kernel.o \
		build/idt/idt.asm.o \
		build/idt/idt.o \
		build/memory/memory.o \
		build/termio/termio.o \
		build/io/io.asm.o \
		build/memory/heap.o \
		build/memory/kheap.o \
		build/memory/paging.asm.o \
		build/memory/paging.o \
		build/disk/disk.o \

BOOT_FILES = $(shell find src/boot/)
TMP_C = $(SRC_C:.c=.o)
OBJ_C = $(TMP_C:src/%=build/%)

SRC_ASM = $(shell find src -path src/boot -prune -false -o -name "*.asm")
TMP_ASM = $(SRC_ASM:.asm=.asm.o)
OBJ_ASM = $(TMP_ASM:src/%=build/%)

OBJ_FILES = $(FILES) #$(OBJ_C) $(OBJ_ASM)

INCLUDES = -Isrc

CFLAGS = -ggdb3 -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops \
	-fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function \
	-fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
	-nostdlib -nostartfiles -nodefaultlibs \
	-Wall -Wextra -Werror -O0 -Iinc -m32 \
	-std=gnu11

CC = i686-elf-gcc
LD = i686-elf-ld
TARGET = bin/image.bin
QEMU_RUN_COMMAND = qemu-system-i386 -hda $(TARGET)

.phony: all folder run gdb
I_KNOW_HOW_TO_LINK = y

ifeq ($(I_KNOW_HOW_TO_LINK), y)

all: bin/image.bin
	
bin/image.bin: build/image.elf
	objcopy -O binary $< $@

build/image.elf:$(OBJ_FILES) src/linker.ld build/boot/boot.asm.o 
	$(LD) -ggg -relocatable $(OBJ_FILES) -o build/kernelfull.o
	$(CC) $(CFLAGS) -Tsrc/linker.ld $(OBJ_FILES) -o $@

build/boot/%.asm.o: $(BOOT_FILES)
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) src/boot/boot.asm -o $@

else
endif

build/kernel/%.o: src/kernel/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/idt/%.o: src/idt/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/termio/%.o: src/termio/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/memory/%.o: src/memory/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/disk/%.o: src/disk/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/fs/%.o: src/fs/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/string/%.o: src/string/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/memory/%.asm.o: src/memory/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

build/io/%.asm.o: src/io/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

build/idt/%.asm.o: src/idt/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

build/kernel/%.asm.o: src/kernel/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

##########################################
##########################################
folders:
	mkdir -p bin build

run: all
	$(QEMU_RUN_COMMAND)

gdb: all
	gdb \
	-ex "set confirm off" \
	-ex "add-symbol-file build/boot/boot.asm.o 0x7c00 " \
	-ex "add-symbol-file build/kernelfull.o 0x0100000 " \
	-ex "target remote | $(QEMU_RUN_COMMAND) -S -gdb stdio" \
	-ex "break kernel_main"

.phony: clean
clean:
	rm -rf bin/*
	rm -rf build/*
