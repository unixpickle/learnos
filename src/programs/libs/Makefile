LIBS=CKeyedBits
CFLAGS=-Wall -ffreestanding -fno-builtin -fno-stack-protector -O2 -fno-zero-initialized-in-bss -nostdinc -I../../libprog -I../../libprog/libc -I../../libprog/base

all: anmalloc/build
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) "CFLAGS=$(CFLAGS)"; \
		cd -; \
	done

anmalloc/build:
	cd anmalloc && $(MAKE) "CFLAGS=$(CFLAGS) -I../../libprog/bindings/include"

clean:
	for dir in $(LIBS); do \
		cd $$dir && make clean && cd -; \
	done
	cd anmalloc && make clean && cd -
	# cd v8 && make clean && cd -

