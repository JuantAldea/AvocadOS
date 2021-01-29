all: boot.bin

boot.bin: boot.asm message.txt
	nasm -f bin ./boot.asm -o boot.bin
	dd if=message.txt >> boot.bin
	# to complete the second 512 byte sector (+ some spare zeroes)
	dd if=/dev/zero bs=512 count=1 >> boot.bin
run: boot.bin
	qemu-system-x86_64 -hda boot.bin

clean:
	rm boot.bin