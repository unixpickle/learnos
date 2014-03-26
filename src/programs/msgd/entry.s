bits 64

extern main

section .entrypoint
start:
  call main
.hang:
  jmp .hang

