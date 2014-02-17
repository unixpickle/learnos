bits 32

global hang32
hang32:
  hlt
  jmp hang32

global outb32
outb32:
  push dx
  push ax
  mov dx, [esp + 8] ; first argument, port
  mov al, [esp + 12] ; second argument, value
  out dx, al
  pop ax
  pop dx
  ret

