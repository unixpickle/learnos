PROJECT_ROOT=../..

CSOURCES=$(wildcard *.c)

all: csources build/context.o

build/context_s.o: build
	nasm -f elf64 context.s -o build/context_s.o

include ../directives.mk
include ../csources.mk
include ../builddir.mk

