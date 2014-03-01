lib: build/anlock.o

test: lib
	cd test && $(MAKE)

build/anlock.o: build
	gcc -c src/anlock.c -o build/anlock.o

build:
	mkdir build

clean:
	rm -rf build/
