CSOURCES32=page_init.c
CSOURCES=apic_init.c smp_init.c
PROJECT_ROOT=../..

include ../directives.mk

all: build/entry.o build/procentry.o

build/entry.o: csources32 csources
	nasm -f elf64 entry.s -o build/entry.o

build/procentry.o:
	nasm -f elf64 procentry.s -o build/procentry.o

csources32: build
	for file in $(CSOURCES32); do\
		gcc -c $$file -I./../ -m32 $(DISABLE_OPTIONS) -o build/`basename $$file .c`_32.o;\
		objcopy -O $(DESTARCH) build/`basename $$file .c`_32.o build/`basename $$file .c`.o;\
		rm build/`basename $$file .c`_32.o;\
	done

include ../csources.mk

build:
	mkdir build

clean:
	rm -rf build/
