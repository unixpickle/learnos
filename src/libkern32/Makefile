DISABLE_OPTIONS=-ffreestanding --no-builtin -fno-stack-protector

all: build/stdio.o

build/stdio.o: build/basic.o
	gcc -c stdio.c -I../ -m32 $(DISABLE_OPTIONS) -o build/stdio_32.o
	objcopy -O elf64-x86-64 build/stdio_32.o build/stdio.o
	rm build/stdio_32.o

build/basic.o: build
	nasm -f elf32 basic.s -o build/basic_32.o
	objcopy -O elf64-x86-64 build/basic_32.o build/basic.o
	rm build/basic_32.o

build:
	mkdir build

clean:
	rm -rf build/
