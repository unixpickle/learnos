bits 64

global hang64
hang64:
  hlt
  jmp hang64

global outb64
outb64:
  push rdx
  mov rdx, rdi ; first argument, port
  mov rax, rsi ; second argument, value
  out dx, al
  pop rdx
  ret

