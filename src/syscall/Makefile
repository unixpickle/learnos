PROJECT_ROOT=../..

CSOURCES=$(wildcard *.c)

all: csources build/entry.o

build/entry.o:
	nasm -f elf64 entry.s -o build/entry.o

include ../directives.mk
include ../csources.mk
include ../builddir.mk

