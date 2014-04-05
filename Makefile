SOURCE_DIRS=src/startup src/startup/libkern32 src/libkern src/interrupts src/scheduler src/syscall src/programs src/memory src/acpi
BUILD_FILES=src/startup/build/*.o src/startup/libkern32/build/*.o src/libkern/build/*.o src/interrupts/build/*.o src/scheduler/build/*.o src/syscall/build/*.o src/programs/build/*.o src/memory/build/*.o src/acpi/build/*.o
LIB_BUILD=libs/anpages/build/*.o libs/anlock/build/*.o libs/anidxset/build/*.o libs/anscheduler/build/*.o libs/anmem/build/*.o libs/anmem/libs/analloc/build/*.o

learnos.bin: objects
	ld $(BUILD_FILES) $(LIB_BUILD) -T linker.ld -e multiboot_header --oformat binary -s -o learnos.bin

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

deps:
	cd libs && $(MAKE)
	cd src/programs/libs && $(MAKE)

deps-clean:
	cd libs && $(MAKE) clean
	cd src/programs/libs && $(MAKE) clean

