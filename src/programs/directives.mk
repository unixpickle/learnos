CFLAGS += -Wall -ffreestanding -fno-builtin -fno-stack-protector -masm=intel -O2 -fno-zero-initialized-in-bss -nostdinc
INCLUDES=-I$(PROJECT_ROOT)/src/programs/libprog -I$(PROJECT_ROOT)/src/programs/libs/CKeyedBits/include -I$(PROJECT_ROOT)/libs/anlock/src -I$(PROJECT_ROOT)/src/programs/libprog/libc
