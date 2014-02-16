MBALIGN equ 1<<0
MEMINFO equ 1<<1
VIDEOINFO equ 1<<2
LINKINFO equ 1<<16
FLAGS equ LINKINFO
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)
LOADBASE equ 0x100000

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
  mov esp, _endstack + LOADBASE
  mov ebp, esp
  push ebx
  push eax
  mov esi, 0
  push esi
  popf

  mov ah, 0xa0
  mov al, 'h'
  mov edi, 0xb8000
  mov word [edi], ax

.hang:
  hlt
  jmp .hang

align 4
_stack:
  times 4096 db 0
_endstack:
