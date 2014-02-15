MBALIGN equ 1<<0
MEMINFO equ 1<<1
FLAGS equ MBALIGN | MEMINFO
MAGIC equ 0x1BADB002
CHECKSUM equ ~(MAGIC + FLAGS)

section .multiboot
align 4
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

section .bootstrap_stack
align 4
stack_bottom:
times 16384 db 0
stack_top:

section .text
global _start
_start:
  mov esp, stack_top
  mov al, 'h'
  mov bl, 0x07
  
  ; output the letter h
  mov edi, 0xb800
  mov [edi], al
  inc edi
  mov [edi], bl
  
.hang:
  hlt
  jmp hang
  
