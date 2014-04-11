CFILES=basic_alloc.c realloc.c mem_start.c

all: ../build/test.o
	for file in $(CFILES); do \
		gcc ../build/*.o $$file -I../src -o ../build/`basename $$file .c`; \
	done

../build/test.o:
	gcc -c test.c -o ../build/test.o
