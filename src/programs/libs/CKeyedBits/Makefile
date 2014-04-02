CFILES=src/buff_decoder.c src/buff_encoder.c src/validation.c
override INCLUDES += -I./include

lib: build
	for file in $(CFILES); do \
		gcc $(CFLAGS) $(INCLUDES) -c $$file -o ./build/`basename $$file .c`.o; \
	done

tests:
	cd test && $(MAKE)

build:
	mkdir build

clean:
	rm -rf build/
	cd test && make clean
