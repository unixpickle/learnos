LIBS=anpages anlock anidxset

all: general anscheduler/build anmem/build

clean:
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) clean; \
		cd -; \
	done
	cd anscheduler && make clean && cd -
	cd anmem && make clean && cd -
	cd anmem/libs/analloc && make clean && cd -

general:
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) CFLAGS=-fno-zero-initialized-in-bss\ -fno-stack-protector\ -mno-red-zone\ -ffreestanding INCLUDES=$(LIB_INCLUDE); \
		cd -; \
	done

anscheduler/build:
	cd anscheduler && $(MAKE) CFLAGS=-fno-zero-initialized-in-bss\ -fno-stack-protector\ -mno-red-zone\ -ffreestanding INCLUDES=-I../../src/scheduler/include\ -I../../src/\ -I../../src/libkern

anmem/build:
	cd anmem && $(MAKE) CFLAGS=-fno-zero-initialized-in-bss\ -fno-stack-protector\ -mno-red-zone\ -ffreestanding INCLUDES=-I../../src/memory/include\ -I../../src/\ -I../../src/libkern && cd -
	cd anmem/libs/analloc && make

