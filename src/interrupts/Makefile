PROJECT_ROOT=../..

CSOURCES=idt.c ioapic.c lapic.c acpi.c pic.c pit.c

all: csources

include ../directives.mk
include ../csources.mk

build/basic.o: build
	nasm -f elf64 basic.s -o build/basic.o

build:
	mkdir build

clean:
	rm -rf build/
