all: lib

test: lib
	gcc build/*.o test/testall.c -Isrc -o build/testall

lib: build
	gcc -c src/anidxset.c -o build/anidxset.o

build:
	mkdir build

clean:
	rm -rf build/
