SOURCE_DIRS=src/startup src/startup/libkern32 src/libkern src/interrupts src/smp
BUILD_FILES=$(filter-out src/startup/build/entry.o, $(wildcard src/startup/build/*.o)) src/startup/libkern32/build/*.o src/libkern/build/*.o src/interrupts/build/*.o src/smp/build/*.o
LIBS=libs/anpages libs/anlock libs/anidxset
LIB_BUILD=libs/anpages/build/*.o libs/anlock/build/*.o libs/anidxset/build/*.o

learnos.bin: objects all_libs
	ld src/startup/build/entry.o $(BUILD_FILES) $(LIB_BUILD) -Ttext 0x100000 -e multiboot_header --oformat binary -s -o learnos.bin

objects:
	for dir in $(SOURCE_DIRS); do \
		cd $$dir && $(MAKE); \
		cd -; \
	done

all_libs:
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE); \
		cd -; \
	done

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

