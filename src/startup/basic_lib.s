bits 64

CUROFFSET equ 0x100000

section .text

global hang64
hang64:
  hlt
  jmp hang64

; input argument is rdi
global print64
print64:
  pushaq
  push rbp
  mov rbp, rsp
  mov rsi, rdi ; input argument
  mov rdi, [CUROFFSET]
  mov ah, 0x0a
  mov rcx, 160
.printLoop:
  mov al, [rsi]
  cmp al, 0
  je .printEnd
  mov word [rdi], ax
  add rdi, 2
  inc rsi
  sub rcx, 2
  jmp .printLoop
.printEnd:
  add rdi, rcx
  mov [CUROFFSET], rdi
  popaq
  leave
  ret
