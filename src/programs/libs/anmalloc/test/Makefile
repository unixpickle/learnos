CFILES=anmalloc_bindings.c
PROGRAMS=test_basic.c
OBJS=../build/*.o ./build/*.o ../libs/analloc/build/*.o
INCLUDES=-Iinclude -I../include -I../libs/analloc/src

all: ../build build ../libs/analloc/build
	for file in $(CFILES); do \
		gcc -c $(INCLUDES) $$file -o build/`basename $$file .c`.o; \
	done
	for file in $(PROGRAMS); do \
		gcc $(INCLUDES) $(OBJS) $$file -o build/`basename $$file .c`; \
	done

../build:
	cd .. && $(MAKE) CFLAGS=-Itest/include

build:
	mkdir build

../libs/analloc/build:
	cd ../libs/analloc && $(MAKE)

clean:
	rm -rf build/
	cd ../ && $(MAKE) clean
	cd ../libs/analloc && $(MAKE) clean
