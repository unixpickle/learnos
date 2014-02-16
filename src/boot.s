; these are multiboot flags
MBALIGN equ 1<<0
MEMINFO equ 1<<1
VIDEOINFO equ 1<<2
LINKINFO equ 1<<16
FLAGS equ LINKINFO
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)
; I saw http://forum.osdev.org/viewtopic.php?f=1&t=21260 and he used this addr
LOADBASE equ 0x100000

org 0x0
section .text

; incase for *some reason* something jumps to the *start* of this file
regular_main:
  mov edi, start
  add edi, LOADBASE
  jmp edi

align 4
multiboot_header:
  dd MAGIC
  dd FLAGS
  dd CHECKSUM
  dd LOADBASE + multiboot_header ; header pointer
  dd LOADBASE                    ; base pointer
  dd 0                           ; loadEndAddr - can be 0 to default
  dd 0                           ; bssEndAddr, 0 = no BSS segment
  dd LOADBASE + start            ; entry point

; this should be jumped to by GRUB2
start:
  ; I figure I might as well *know* the ESP will point to my data
  mov esp, _endstack + LOADBASE
  mov ebp, esp
  ; these are supposedly inputs from GRUB, but I don't use them yet
  push ebx
  push eax

  ; zero our flags
  mov esi, 0
  push esi
  popf

  ; this *should* print the letter `h` to the damn screen
  mov ah, 0xa0
  mov al, 'h'
  mov edi, 0xb8000
  mov word [edi], ax

; this is how I know VirtualBox is running *something*. When I remove the
; hlt instruction, the thread uses 100% CPU, otherwise it uses none at all.
.hang:
  hlt
  jmp .hang

; yeah i know, this is somewhat pointless but whatevs man
align 4
_stack:
  times 4096 db 0
_endstack:
