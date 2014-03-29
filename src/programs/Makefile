PROGRAMS=keyboard ticktock msgd intd terminal pcid

programs: libprog/build build
	for dir in $(PROGRAMS); do \
		cd $$dir && $(MAKE); \
		cd ..; \
		ld $$dir/build/*.o libprog/build/*.o -T linker.ld --oformat binary -s -o $$dir/build/$${dir}.bin; \
		objcopy --input binary --binary-architecture i386 --output elf64-x86-64 $$dir/build/$${dir}.bin build/$${dir}.o; \
	done

libprog/build:
	cd libprog && $(MAKE)

build:
	mkdir build

clean:
	rm -rf build
	cd libprog && make clean
	for dir in $(PROGRAMS); do \
		cd $$dir && make clean && cd ..; \
	done

