bits 64

extern main

section .entrypoint
start:
  call main
.hang:
  jmp .hang

.msg:
  db 'hey there', 0xa, 0
