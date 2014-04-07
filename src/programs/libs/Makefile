LIBS=CKeyedBits

all:
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) CFLAGS=-Wall\ -ffreestanding\ -fno-builtin\ -fno-stack-protector\ -O2\ -fno-zero-initialized-in-bss\ -nostdinc\ -I../../libprog\ -I../../libprog/libc; \
		cd -; \
	done

v8_all: v8_deps v8_build

v8_deps:
	cd v8 && make dependencies

v8_build:
	# here, we'll set -DOS=learnos
	export GYP_DEFINES=-DOS=mac\ -Ddebuggersupport=off\ ; \
	export CXX=g++\ -nostdinc\ -ffreestanding\ -fno-builtin\ -fno-stack-protector\ -O2\ -fno-zero-initialized-in-bss\ -I`pwd`/../libprog -I`pwd`../libprog/libc; \
	cd v8 && make x64.release

clean:
	for dir in $(LIBS); do \
		cd $$dir && make clean && cd -; \
	done
	cd v8 && make clean

