.phony: all clean


all: trap.asm
	mkdir -p build
	nasm -f elf ./trap.asm -o ./build/trap.asm.o
	i686-elf-gcc  -ffreestanding -O0 -nostdlib -fpic -g -T ./linker.ld  build/trap.asm.o -o ./build/trap.bin

clean:
	rm -rf ./build/trap.o
