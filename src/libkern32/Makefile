include ../directives.mk

all: build/stdio.o

build/stdio.o: build/basic.o
	gcc -c stdio.c -I../ -m32 $(DISABLE_OPTIONS) -o build/stdio_32.o
	objcopy -O $(DESTARCH) build/stdio_32.o build/stdio.o
	rm build/stdio_32.o

build/basic.o: build
	nasm -f elf32 basic.s -o build/basic_32.o
	objcopy -O $(DESTARCH) build/basic_32.o build/basic.o
	rm build/basic_32.o

build:
	mkdir build

clean:
	rm -rf build/
