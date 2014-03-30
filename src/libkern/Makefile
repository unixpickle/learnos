PROJECT_ROOT=../..
CSOURCES=stdio.c string.c ref.c debug.c

all: build/libkern_base.o csources

include ../directives.mk
include ../csources.mk

build/libkern_base.o: build
	nasm -f elf64 libkern_base.s -o build/libkern_base.o

include ../builddir.mk

