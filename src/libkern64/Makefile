DISABLE_OPTIONS=-ffreestanding --no-builtin -fno-stack-protector

all: build/stdio.o

build/stdio.o: build/basic.o
	gcc -c stdio.c -I../ -m64 $(DISABLE_OPTIONS) -o build/stdio.o

build/basic.o: build
	nasm -f elf64 basic.s -o build/basic.o

build:
	mkdir build

clean:
	rm -rf build/
