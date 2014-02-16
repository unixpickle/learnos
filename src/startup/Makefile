all: build/entry.o build/apic.o

build/entry.o: build/basic_lib.o
	nasm -f elf64 entry.s -o build/entry.o

build/apic.o: build/basic_lib.o
	gcc -c apic.c -m64 -fno-stack-protector -o build/apic.o

build/basic_lib.o: build
	nasm -f elf64 basic_lib.s -o build/basic_lib.o

build:
	mkdir build

clean:
	rm -rf build/
