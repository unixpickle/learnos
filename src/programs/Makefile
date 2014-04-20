PROGRAMS=keyboard timed msgd intd terminal pcid memd
CPPPROGRAMS=jsc
PROJECT_ROOT=../..
OBJECTS=libs/CKeyedBits/build/*.o $(PROJECT_ROOT)/libs/anlock/build/*.o $(PROJECT_ROOT)/libs/anmem/libs/analloc/build/*.o libprog/build/*.o libs/anmalloc/build/*.o
CPPOBJECTS=libcpp/build/*.o

all: programs cppprograms

programs: libprog/build build
	for dir in $(PROGRAMS); do \
		cd $$dir && $(MAKE); \
		cd ..; \
		ld $(OBJECTS) $$dir/build/*.o -T linker.ld --oformat binary -s -o $$dir/build/$${dir}.bin; \
		objcopy --input binary --binary-architecture i386 --output elf64-x86-64 $$dir/build/$${dir}.bin build/$${dir}.o; \
	done

cppprograms: libprog/build libcpp/build build
	for dir in $(CPPPROGRAMS); do \
		cd $$dir && $(MAKE); cd -; \
		ld $(OBJECTS) $(CPPOBJECTS) $$dir/build/*.o -T linker.ld --oformat binary -s -o $$dir/build/$${dir}.bin; \
		objcopy --input binary --binary-architecture i386 --output elf64-x86-64 $$dir/build/$${dir}.bin build/$${dir}.o; \
	done

libprog/build:
	cd libprog && $(MAKE)

libcpp/build:
	cd libcpp && $(MAKE)

build:
	mkdir build

clean:
	rm -rf build
	cd libprog && make clean
	for dir in $(PROGRAMS) $(CPPPROGRAMS); do \
		cd $$dir && make clean && cd ..; \
	done
	for dir in $(LIBS); do \
		cd $$dir && make clean && cd -; \
	done

deps:
	cd libs && make

deps-clean:
	cd libs && make clean

