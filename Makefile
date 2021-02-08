C_FILES := $(shell find src -name "*.c")
OBJECT_C_FILES = $(C_FILES:.c=.o)
BUILD_C_FILES = $(OBJECT_C_FILES:src/%=build/%)

ASM_FILES := $(shell find src -path src/boot -prune -false -o -name "*.asm")
OBJECT_ASM_FILES = $(ASM_FILES:.asm=.asm.o)
BUILD_ASM_FILES = $(OBJECT_ASM_FILES:src/%=build/%)

BUILD_FILES = $(BUILD_ASM_FILES) $(BUILD_C_FILES)

BINARIES = bin/boot.bin bin/kernel.bin

INCLUDES = -Isrc/

CFLAGS = -ggdb3 \
	-ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops \
	-fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function \
	-fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
	-nostdlib -nostartfiles -nodefaultlibs \
	-Wall -Wextra -Werror -O0 -Iinc -m32 \
	-std=gnu11

all: bin/image.bin

bin/image.bin: $(BINARIES)
	dd if=bin/boot.bin > bin/image.bin
	dd if=bin/kernel.bin >> bin/image.bin
	# padding and safe space all that
	dd if=/dev/zero bs=512 count=100 >> bin/image.bin

bin/boot.bin: build/boot.elf
	mkdir -p $(@D)
	# extract binary from ELF
	i686-elf-ld -Ttext 0x7C00 build/boot.elf -o build/boot.o
	objcopy -O binary build/boot.o bin/boot.bin

build/boot.elf: src/boot/*
	mkdir -p $(@D)
	# generate elf so that we have debug symbols
	nasm -i src/boot/ -f elf -g -F dwarf src/boot/boot.asm -o build/boot.elf

bin/kernel.bin: $(BUILD_FILES)
	i686-elf-ld -ggg -relocatable $(BUILD_FILES) -o build/kernelfull.o
	i686-elf-gcc $(CFLAGS) -T src/linker.ld -o bin/kernel.bin build/kernelfull.o

$(BUILD_C_FILES): $@
	mkdir -p $(@D)
	i686-elf-gcc $(INCLUDES) $(CFLAGS) -c $(patsubst %.o,%.c,$(@:build/%=src/%)) -o $@

$(BUILD_ASM_FILES): $@
	mkdir -p $(@D)
	nasm -i $(@D:build/%=src/%) -f elf -g -F dwarf $(patsubst %.o,%,$(@:build/%=src/%)) -o $@

##########################################
##########################################

run: all
	qemu-system-i386 -hda bin/image.bin

gdb: all
	gdb \
	-ex "set confirm off" \
	-ex "add-symbol-file build/boot.o 0x7c00 " \
	-ex "add-symbol-file build/kernelfull.o 0x0100000 " \
	-ex "target remote | qemu-system-i386 -S -gdb stdio -hda bin/image.bin" \
	-ex "break kernel_main"

clean:
	rm -rf bin/*
	rm -rf build/*
