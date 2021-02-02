FILES = build/kernel.asm.o build/kernel.o build/idt/idt.asm.o build/idt/idt.o build/memory/memory.o build/termio/termio.o
INCLUDES = -Isrc

CFLAGS = -ggdb3 -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops \
	-fstrength-reduce -fno-omit-frame-pointer -finline-functions -Wno-unused-function \
	-fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
	-nostdlib -nostartfiles -nodefaultlibs \
	-Wall -Wextra -Werror -O0 -Iinc

all: folders bin/boot.bin bin/kernel.bin
	dd if=bin/boot.bin > bin/os.bin
	dd if=bin/kernel.bin >> bin/os.bin
	# padding and safe space all that
	dd if=/dev/zero bs=512 count=100 >> bin/os.bin

folders:
	mkdir -p bin build build/idt build/memory build/termio

bin/boot.bin: src/boot/*
	# generate with debug symbols, then extract the binary
	nasm -i src/boot/ -f elf -g -F dwarf src/boot/boot.asm -o build/boot.elf
	i686-elf-ld -Ttext 0x7C00 build/boot.elf -o build/boot.o
	objcopy -O binary build/boot.o bin/boot.bin

bin/kernel.bin: $(FILES)
	i686-elf-ld -ggg -relocatable $(FILES) -o build/kernelfull.o
	i686-elf-gcc $(CFLAGS) -T src/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib build/kernelfull.o

build/kernel.asm.o: src/kernel.asm
	nasm -f elf -g -F dwarf src/kernel.asm -o build/kernel.asm.o

build/kernel.o: src/kernel.c
	i686-elf-gcc $(INCLUDES) $(CFLAGS) -std=gnu99 -c src/kernel.c -o build/kernel.o

build/idt/idt.asm.o: src/idt/idt.asm
	nasm -f elf -g -F dwarf src/idt/idt.asm -o build/idt/idt.asm.o

build/idt/idt.o: src/idt/idt.c
	i686-elf-gcc $(INCLUDES) -Isrc/idt $(CFLAGS) -std=gnu99 -c src/idt/idt.c -o build/idt/idt.o

build/memory/memory.o: src/memory/memory.c
	i686-elf-gcc $(INCLUDES) -Isrc/memory $(CFLAGS) -std=gnu99 -c src/memory/memory.c -o build/memory/memory.o

build/termio/termio.o: src/termio/termio.c
	i686-elf-gcc $(INCLUDES) -Isrc/termio $(CFLAGS) -std=gnu99 -c src/termio/termio.c -o build/termio/termio.o


run: all
	qemu-system-x86_64 -hda bin/os.bin

gdb: all
	gdb \
	-ex "set confirm off" \
    -ex "add-symbol-file build/boot.o 0x7c00 " \
	-ex "add-symbol-file build/kernelfull.o 0x0100000 " \
    -ex "target remote |qemu-system-x86_64 -S -gdb stdio -hda bin/os.bin" \
    -ex "break kernel_main"

clean:
	rm -rf bin/*
	rm -rf build/*
