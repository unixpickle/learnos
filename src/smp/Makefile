PROJECT_ROOT=../..

CSOURCES=cpu_config.c cpu_list.c task.c vm.c thread.c scheduler.c gdt.c

all: csources build/context.o

build/context.o: build
	nasm -f elf64 context.s -o build/context.o

include ../directives.mk
include ../csources.mk
include ../builddir.mk

