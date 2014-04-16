LIBS=CKeyedBits
CFLAGS=-Wall -ffreestanding -fno-builtin -fno-stack-protector -O2 -fno-zero-initialized-in-bss -nostdinc -I../../libprog -I../../libprog/libc -I../../libprog/base

all: anmalloc/build v8_all
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) "CFLAGS=$(CFLAGS)"; \
		cd -; \
	done

anmalloc/build:
	cd anmalloc && $(MAKE) "CFLAGS=$(CFLAGS) -I../../libprog/bindings/include"

v8_all: v8_deps v8_build

v8_deps:
	cd v8 && make dependencies

v8_build:
	# here, we'll set -DOS=learnos
	export GYP_DEFINES=-DOS=mac\ -Ddebuggersupport=off\ ; \
	export CXX=g++\ -nostdinc\ -ffreestanding\ -fno-builtin\ -fno-stack-protector\ -O2\ -fno-zero-initialized-in-bss\ -I`pwd`/../libprog\ -I`pwd`/../libprog/libc; \
	cd v8 && make x64.release

clean:
	for dir in $(LIBS); do \
		cd $$dir && make clean && cd -; \
	done
	cd v8 && make clean
	cd anmalloc && make clean

