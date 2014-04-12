lib: build/analloc.o

test: build/analloc.o
	cd test && $(MAKE)

build/analloc.o: build/anbtree.o
	gcc -c src/analloc.c -o build/analloc.o

build/anbtree.o: build
	gcc -c src/anbtree.c -o build/anbtree.o

build:
	mkdir build

clean:
	rm -rf build/
