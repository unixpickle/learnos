LIBS=CKeyedBits

all:
	for dir in $(LIBS); do \
		cd $$dir && $(MAKE) CFLAGS=-Wall\ -ffreestanding\ -fno-builtin\ -fno-stack-protector\ -O2\ -fno-zero-initialized-in-bss; \
		cd -; \
	done

clean:
	for dir in $(LIBS); do \
		cd $$dir && make clean && cd -; \
	done

