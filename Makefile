FILES = build/kernel.asm.o \
		build/kernel.o \
		build/idt/idt.asm.o \
		build/idt/idt.o \
		build/memory/memory.o \
		build/termio/termio.o \
		build/io/io.asm.o \
		build/memory/heap.o \
		build/memory/kheap.o \
		build/memory/paging.asm.o \
		build/memory/paging.o \
		build/disk/disk.o

SRC_C = $(shell find src -name "*.c")
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

.phony: all
all: folders bin/boot.bin bin/kernel.bin
	dd if=bin/boot.bin > bin/image.bin
	dd if=bin/kernel.bin >> bin/image.bin
	# padding and safe space all that
	dd if=/dev/zero bs=512 count=100 >> bin/image.bin

.phony: folders
folders:
	mkdir -p bin build

bin/boot.bin: src/boot/*
	@mkdir -p $(@D)
	# generate with debug symbols, then extract the binary
	nasm -f elf -g -F dwarf -i $(dir $<) src/boot/boot.asm -o build/boot.elf
	$(LD) -ggg -Ttext 0x7C00 build/boot.elf -o build/boot.o
	objcopy -O binary build/boot.o $@

bin/kernel.bin: $(OBJ_FILES) src/linker.ld
	@mkdir -p $(@D)
	$(LD) -ggg -relocatable $(OBJ_FILES) -o build/kernelfull.o
	$(CC) $(CFLAGS) -T src/linker.ld -ffreestanding -O0 -nostdlib build/kernelfull.o -o $@

build/%.o: src/%.c
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

build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

build/idt/%.asm.o: src/idt/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

build/io/%.asm.o: src/io/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@

build/idt/%.asm.o: src/idt/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) $< -o $@


build/%.o: %.c
	$(CC) $(CFLAGS)  -MD -c $< -o $@
##########################################
##########################################
.phony: run
run: all
	qemu-system-i386 -hda bin/image.bin

.phony: gdb
gdb: all
	gdb \
	-ex "set confirm off" \
	-ex "add-symbol-file build/boot.o 0x7c00 " \
	-ex "add-symbol-file build/kernelfull.o 0x0100000 " \
	-ex "target remote | qemu-system-i386 -S -gdb stdio -hda bin/image.bin" \
	-ex "break kernel_main"

.phony: clean
clean:
	rm -rf bin/*
	rm -rf build/*
