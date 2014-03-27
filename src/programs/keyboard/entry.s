bits 64

extern main, sys_exit

section .entrypoint
start:
  call main
  call sys_exit
.hang:
  jmp .hang

