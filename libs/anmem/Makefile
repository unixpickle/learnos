CFILES=src/anmalloc.c
INCLUDES=-Iinclude -Ilibs/analloc/src

all: build
	for file in $(CFILES); do \
		gcc -c $(CFLAGS) $(INCLUDES) $$file -o build/`basename $$file .c`.o; \
	done

build:
	mkdir build

clean:
	rm -rf build/
