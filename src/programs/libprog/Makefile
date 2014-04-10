PROJECT_ROOT=../../..
CSOURCES=$(wildcard *.c) $(wildcard libc/*.c) $(wildcard base/*.c) $(wildcard bindings/*.c)
ASMSOURCES=$(wildcard *.s) $(wildcard libc/*.s) $(wildcard base/*.s) $(wildcard bindings/*.s)

all: asmsources csources

include ../directives.mk
include ../csources.mk
include ../asmsources.mk
include ../builddir.mk

