PROJECT_ROOT=../..

CSOURCES=functions.c

all: csources build/base.o

build/base.o: build
	nasm -f elf64 base.s -o build/base.o

include ../directives.mk
include ../csources.mk
include ../builddir.mk

