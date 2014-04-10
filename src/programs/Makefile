PROGRAMS=keyboard ticktock msgd intd terminal pcid memd
PROJECT_ROOT=../..
OBJECTS=libs/CKeyedBits/build/*.o $(PROJECT_ROOT)/libs/anlock/build/*.o $(PROJECT_ROOT)/libs/anmem/libs/analloc/build/*.o libprog/build/*.o libs/anmalloc/build/*.o

programs: libprog/build build
	for dir in $(PROGRAMS); do \
		cd $$dir && $(MAKE); \
		cd ..; \
		ld $(OBJECTS) $$dir/build/*.o -T linker.ld --oformat binary -s -o $$dir/build/$${dir}.bin; \
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
	for dir in $(LIBS); do \
		cd $$dir && make clean && cd -; \
	done

deps:
	cd libs && make

deps-clean:
	cd libs && make clean

