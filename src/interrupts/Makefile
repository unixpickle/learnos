PROJECT_ROOT=../..

CSOURCES=idt.c ioapic.c lapic.c pic.c pit.c

all: csources build/basic.o

include ../directives.mk
include ../csources.mk

build/basic.o: build
	nasm -f elf64 basic.s -o build/basic.o

include ../builddir.mk

