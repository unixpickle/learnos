SOURCE_DIRS=src/startup src/startup/libkern32 src/libkern src/interrupts src/scheduler src/syscall src/programs
BUILD_FILES=src/startup/build/*.o src/startup/libkern32/build/*.o src/libkern/build/*.o src/interrupts/build/*.o src/scheduler/build/*.o src/syscall/build/*.o src/programs/build/*.o
LIBS=libs/anpages libs/anlock libs/anidxset
LIB_BUILD=libs/anpages/build/*.o libs/anlock/build/*.o libs/anidxset/build/*.o libs/anscheduler/build/*.o

learnos.bin: objects all_libs anscheduler
	ld $(BUILD_FILES) $(LIB_BUILD) -T linker.ld -e multiboot_header --oformat binary -s -o learnos.bin

objects:
	for dir in $(SOURCE_DIRS); do \
		cd $$dir && $(MAKE); \
		cd -; \
	done

all_libs:
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) CFLAGS=-fno-stack-protector\ -mno-red-zone\ -ffreestanding INCLUDES=$(LIB_INCLUDE); \
		cd -; \
	done

anscheduler:
	cd libs/anscheduler && $(MAKE) CFLAGS=-fno-stack-protector\ -mno-red-zone\ -ffreestanding INCLUDES=-I../../src/scheduler/include\ -I../../src/\ -I../../src/libkern

image: learnos.bin
	cp learnos.bin isodir/boot/learnos.bin
	grub-mkrescue -o learnos.iso isodir/

clean:
	rm -rf learnos.bin
	rm -rf isodir/boot/learnos.bin
	rm -rf learnos.iso
	for dir in $(SOURCE_DIRS); do \
		cd $$dir && $(MAKE) clean; \
		cd -; \
	done
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) clean; \
		cd -; \
	done
	cd libs/anscheduler && $(MAKE) clean

