BOOT_FILES = $(shell find src/boot/)
SRC_C = $(shell find src -name "*.c")
TMP_C = $(SRC_C:.c=.o)
OBJ_C = $(TMP_C:src/%=build/%)

SRC_ASM = $(shell find src -path src/boot -prune -false -o -name "*.asm")
TMP_ASM = $(SRC_ASM:.asm=.asm.o)
OBJ_ASM = $(TMP_ASM:src/%=build/%)

OBJ_FILES = $(OBJ_C) $(OBJ_ASM)
LINKER_FILES = $(shell find src/ -name "*.ld")
BOOT_FILES = src/boot/boot.asm src/boot/gdt.inc src/boot/ata_lba_read.inc
INCLUDES = -Isrc

CFLAGS = -ggdb3 -ffreestanding -nostdlib \
	-falign-jumps -falign-functions -falign-labels -falign-loops \
	-fstrength-reduce -fomit-frame-pointer -finline-functions \
	-std=gnu11 -O0 -Iinc \
	-Wall -Wextra -Werror \
	-Wno-unused-function -Wno-unused-label -Wno-cpp -Wno-unused-parameter

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

define BUILD_RULE
$(1): $(2) $(3)
	@mkdir -p $(dir $(1))

	$(eval ASM = $(findstring .asm, $(1)))

	$(if $(ASM),
		nasm -f elf -g -F dwarf -i $(dir $(2)) $(2) -o $(1), \
		$(CC) $(INCLUDES) -I$(dir $2) $(CFLAGS) -c $(2) -o $(1) \
	)
endef

$(foreach file, $(OBJ_ASM), \
	$(eval SRC_FILE = $(file:build/%=src/%)) \
	$(eval $(call BUILD_RULE, $(file), $(SRC_FILE:%.asm.o=%.asm))) \
)

$(foreach file, $(OBJ_C), \
	$(eval SRC_FILE = $(file:build/%=src/%)) \
	$(eval $(call BUILD_RULE, $(file), $(SRC_FILE:%.o=%.c), $(SRC_FILE:%.o=%.h))) \
)

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

build/boot/boot.asm.o: $(BOOT_FILES)
	@mkdir -p $(@D)
	nasm -f elf -g -F dwarf -i $(dir $<) src/boot/boot.asm -o $@

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
