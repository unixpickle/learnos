bits 64

extern main, sys_print

section .entrypoint
start:
  mov r15, sys_print
  call main
.hang:
  jmp .hang

.msg:
  db 'hey there', 0xa, 0
