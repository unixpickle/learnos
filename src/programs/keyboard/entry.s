bits 64

extern main

section .entrypoint
start:
  ;mov rdi, (.msg - start + 0x10500400000)
  ;int 0x21
  ; call main
.hang:
  jmp .hang
.msg:
  db 'hey there', 0xa, 0
