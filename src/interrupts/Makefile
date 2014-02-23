include ../directives.mk

CSOURCES=idt.c ioapic.c lapic.c acpi.c pic.c

all: csources

csources: build/basic.o
	for file in $(CSOURCES); do\
		gcc -c $$file -I../ -m64 $(DISABLE_OPTIONS) -o build/`basename $$file .c`.o;\
	done

build/basic.o: build
	nasm -f elf64 basic.s -o build/basic.o

build:
	mkdir build

clean:
	rm -rf build/
