C32FILES=page_init.c
C64FILES=apic.c
DISABLE_OPTIONS=-ffreestanding --no-builtin -fno-stack-protector

all: build/entry.o

build/entry.o: c32sources c64sources
	nasm -f elf64 entry.s -o build/entry.o

c32sources: build
	for file in $(C32FILES); do\
		gcc -c $$file -I./../ -m32 $(DISABLE_OPTIONS) -o build/`basename $$file .c`_32.o;\
		objcopy -O elf64-x86-64 build/`basename $$file .c`_32.o build/`basename $$file .c`.o;\
		rm build/`basename $$file .c`_32.o;\
	done

c64sources: build
	for file in $(C64FILES); do\
		gcc -c $$file -I./../ -m64 $(DISABLE_OPTIONS) -o build/`basename $$file .c`.o;\
	done

build:
	mkdir build

clean:
	rm -rf build/
