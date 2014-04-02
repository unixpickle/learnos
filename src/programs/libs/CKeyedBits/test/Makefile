CFILES=test_buff_decoder.c test_buff_encoder.c test_validation.c

lib: build ../build
	for file in $(CFILES); do \
		gcc -I../include $$file ../build/*.o -o build/`basename $$file .c`; \
	done

build:
	mkdir build

../build:
	cd .. && $(MAKE)

clean:
	rm -rf build
