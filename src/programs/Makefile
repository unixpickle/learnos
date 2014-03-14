build/keyboard.o: build
	nasm -f elf64 keyboard.s -o build/keyboard.o

libprog/build:
	cd libprog && $(MAKE)

build:
	mkdir build

clean:
	rm -rf build
	cd libprog && make clean
