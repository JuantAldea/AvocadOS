FILES = build/kernel.asm.o build/kernel.o
INCLUDES = -Isrc

CFLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops \
	-fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function \
	-fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
	-nostdlib -nostartfiles -nodefaultlibs \
	-Wall -Wextra -Werror -O0 -Iinc

all: bin/boot.bin bin/kernel.bin
	dd if=bin/boot.bin > bin/os.bin
	dd if=bin/kernel.bin >> bin/os.bin
	# padding and safe space all that
	dd if=/dev/zero bs=512 count=100 >> bin/os.bin

bin/boot.bin: src/boot/boot.asm
	nasm -f bin src/boot/boot.asm -o bin/boot.bin

bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o build/kernelfull.o
	i686-elf-gcc $(CFLAGS) -T src/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib build/kernelfull.o

build/kernel.asm.o: src/kernel.asm
	nasm -f elf -g src/kernel.asm -o build/kernel.asm.o

build/kernel.o: src/kernel.c
	i686-elf-gcc $(INCLUDES) $(CFLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o 

run: all
	qemu-system-x86_64 -hda bin/os.bin

gdb:
	gdb \
	-ex "set confirm off" \
    -ex "add-symbol-file build/kernelfull.o 0x0100000 " \
    -ex "target remote |qemu-system-x86_64 -S -gdb stdio -hda bin/os.bin" \
    -ex "break kernel_main"

clean:
	rm -rf bin/*
	rm -rf build/*
