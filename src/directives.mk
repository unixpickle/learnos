DISABLE_OPTIONS=-Wall -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -fno-zero-initialized-in-bss
DESTARCH=elf64-x86-64
INCLUDES_64=-I$(PROJECT_ROOT)/src -I$(PROJECT_ROOT)/src/libkern -I$(PROJECT_ROOT)/libs/anpages/src -I$(PROJECT_ROOT)/libs/anlock/src -I$(PROJECT_ROOT)/libs/anidxset/src -I$(PROJECT_ROOT)/libs/anscheduler/include -I$(PROJECT_ROOT)/src/scheduler/include -I$(PROJECT_ROOT)/libs/anmem/include -I$(PROJECT_ROOT)/libs/anmem/libs/analloc/src -I$(PROJECT_ROOT)/src/memory/include
INCLUDES_32=-I$(PROJECT_ROOT)/src
