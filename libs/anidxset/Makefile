all: lib

test: lib
	gcc $(CFLAGS) build/*.o test/testall.c -Isrc -o build/testall

lib: build
	gcc $(CFLAGS) -c src/anidxset.c -o build/anidxset.o

build:
	mkdir build

clean:
	rm -rf build/
