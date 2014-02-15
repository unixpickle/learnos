MBALIGN equ 1<<0
MEMINFO equ 1<<1
LINKINFO equ 1<<16
FLAGS equ MBALIGN | MEMINFO | LINKINFO
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)
LOADBASE equ 0x10000

org 0x0
section .text

regular_main:
  mov edi, start
  add edi, LOADBASE
  jmp edi

align 4
multiboot_header:
  dd MAGIC
  dd FLAGS
  dd CHECKSUM
  dd LOADBASE + multiboot_header
  dd LOADBASE
  dd 0
  dd 0
  dd LOADBASE + start

start:
  mov esp, _stack + 4096 + LOADBASE
  push 0
  popf
  push ebx
  push eax

  mov bl, 0x07
  
  ; output the letter h
  mov al, 'h'
  call .print
  mov al, 'e'
  call .print
  mov al, 'y'
  call .print
  jmp .hang

.print:
  mov [edi], al
  inc edi
  mov [edi], bl
  inc edi
  ret

.hang:
  hlt
  jmp .hang

align 4
_stack:
  times 4096 db 0

