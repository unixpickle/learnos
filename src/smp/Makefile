PROJECT_ROOT=../..

CSOURCES=cpu.c gdt.c tasks.c vm.c util.c creation.c destruction.c lifecycle.c scheduler.c

all: csources build/context.o

build/context.o: build
	nasm -f elf64 context.s -o build/context.o

include ../directives.mk
include ../csources.mk
include ../builddir.mk

