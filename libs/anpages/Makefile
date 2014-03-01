all: test

test: build/anpages.o
	cd test && $(MAKE)

build/anpages.o: build
	gcc -c src/anpages.c -o build/anpages.o

build:
	mkdir build

clean:
	rm -rf build/
