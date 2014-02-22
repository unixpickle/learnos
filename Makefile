SOURCE_DIRS=src/startup src/libkern32 src/libkern64 src/interrupts
BUILD_FILES=$(filter-out src/startup/build/entry.o, $(wildcard src/startup/build/*.o)) src/libkern32/build/*.o src/libkern64/build/*.o src/interrupts/build/*.o

learnos.bin: objects
	ld src/startup/build/entry.o $(BUILD_FILES) -Ttext 0x100000 -e multiboot_header --oformat binary -s -o learnos.bin

objects:
	for dir in $(SOURCE_DIRS); do \
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

