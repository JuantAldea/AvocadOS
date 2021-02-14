
FILES = build/kernel/kernel.o \
		build/idt/idt.asm.o \
		build/idt/idt.o \
		build/termio/termio.o \
		build/io/io.asm.o \
		build/memory/heap.o \
		build/memory/kheap.o \
		build/memory/paging.asm.o \
		build/memory/paging.o \
		build/disk/disk.o \
		build/disk/disk_stream.o \
		build/string/string.o \
		build/fs/path_parser.o \
		build/fs/vfs.o \
		build/fs/fat16.o \
		#build/kernel/kernel.asm.o \

BOOT_FILES = $(shell find src/boot/)
TMP_C = $(SRC_C:.c=.o)
OBJ_C = $(TMP_C:src/%=build/%)

SRC_ASM = $(shell find src -path src/boot -prune -false -o -name "*.asm")
TMP_ASM = $(SRC_ASM:.asm=.asm.o)
OBJ_ASM = $(TMP_ASM:src/%=build/%)

OBJ_FILES = $(FILES) #$(OBJ_C) $(OBJ_ASM)
LINKER_FILES = $(shell find src/ -name "*.ld")
BOOT_FILES = src/boot/boot.asm src/boot/gdt.inc src/boot/ata_lba_read.inc
INCLUDES = -Isrc

CFLAGS = -ggdb3 -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops \
	-fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function \
	-fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
	-nostdlib -nostartfiles -nodefaultlibs \
	-Wall -Wextra -Werror -O0 -Iinc \
	-std=gnu11

CC = i686-elf-gcc
LD = i686-elf-ld
TARGET = bin/image.bin
QEMU_RUN_COMMAND = qemu-system-i386 -hda $(TARGET)

.phony: all folder run gdb clean

all: $(TARGET)

$(TARGET): bin/boot.bin bin/kernel.bin
	dd if=bin/boot.bin > $@
	dd if=bin/kernel.bin >> $@
	# padding and safe space all that
	dd if=/dev/zero bs=1048576 count=16 >> $@
	mkdir -p mnt
	fusefat -o rw+ $@ mnt/
	echo "Would you fancy some avocados?" > mnt/dummy.txt
	fusermount -u -q -z mnt/ || /bin/true

build/kernel/kernel.elf: $(OBJ_FILES) $(LINKER_FILES) build/kernel/kernel.asm.o
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -T src/kernel/linker.ld $(OBJ_FILES) -lgcc -o $@

build/boot/boot.elf: build/boot/boot.asm.o $(LINKER_FILES)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -T src/boot/linker.ld -lgcc $< -o $@

bin/kernel.bin: build/kernel/kernel.elf
	@mkdir -p $(@D)
	objcopy -O binary $< $@

bin/boot.bin: build/boot/boot.elf
	@mkdir -p $(@D)
	objcopy -O binary $< $@

build/kernel/%.o: src/kernel/%.c src/kernel/%.h
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/idt/%.o: src/idt/%.c src/idt/%.h
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/termio/%.o: src/termio/%.c src/termio/%.h
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/memory/%.o: src/memory/%.c src/memory/%.h
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/disk/%.o: src/disk/%.c src/disk/%.h
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/fs/%.o: src/fs/%.c src/fs/%.h
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/string/%.o: src/string/%.c src/string/%.h
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) -I$(dir $<) $(CFLAGS) -c $< -o $@

build/boot/boot.asm.o: $(BOOT_FILES)
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) src/boot/boot.asm -o $@

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

run: all
	$(QEMU_RUN_COMMAND)

gdb: all
	gdb \
	-ex "set confirm off" \
	-ex "add-symbol-file build/boot/boot.elf 0x7c00 " \
	-ex "add-symbol-file build/kernel/kernel.elf 0x0100000 " \
	-ex "target remote | $(QEMU_RUN_COMMAND) -S -gdb stdio" \
	-ex "break kernel_main"

clean:
	rm -rf bin/*
	rm -rf build/*
