extern print32
extern hang32
extern configurePages

extern print64
extern hang64
extern configureAPIC

bits 32

; these are multiboot flags
MBALIGN equ 1<<0
MEMINFO equ 1<<1
VIDEOINFO equ 1<<2
LINKINFO equ 1<<16
FLAGS equ LINKINFO
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

LOADBASE equ 0x100000
CUROFFSET equ 0x200000
END_PAGE_INFO equ 0x200004
PML4_START equ 0x300000

section .text

align 4
global multiboot_header
multiboot_header:
  dd MAGIC
  dd FLAGS
  dd CHECKSUM
  dd multiboot_header ; header pointer
  dd LOADBASE         ; base pointer
  dd 0                ; loadEndAddr - can be 0 to default
  dd 0                ; bssEndAddr, 0 = no BSS segment
  dd start            ; entry point

; this should be jumped to by GRUB2
start:
  ; I figure I might as well *know* the ESP will point to my data
  mov esp, _endstack
  mov ebp, esp
  ; ebx is a pointer to useful information, eax is multiboot magic
  push ebx
  push eax
  ; zero our flags
  mov esi, 0
  push esi
  popf

  mov dword [CUROFFSET], 0

  ; print our initialization message
  push startedMessage
  call print32
  add esp, 4

  ; make sure memory information flag is set
  mov esi, [ebp - 4]
  mov eax, [esi]
  or eax, 1
  jnz .setupPages
  push noMemoryFlagMessage
  call print32
  call hang32

.setupPages:
  ; pushes the amount of 1024 byte blocks (after 1MB) to the stack
  mov eax, [esi + 8]
  push eax
  call configurePages
  mov [END_PAGE_INFO], eax
  pop eax

  push donePagesMessage
  call print32
  add esp, 4

  ; enable PAE-paging
  mov eax, cr4
  or eax, 1 << 5
  mov cr4, eax
  ; set LM bit
  mov ecx, 0xC0000080 ; EFER MSR
  rdmsr
  or eax, 1 << 8 ; long-mode bit
  wrmsr
  ; enable paging
  mov esi, PML4_START
  mov cr3, esi
  mov eax, cr0
  or eax, 1 << 31
  mov cr0, eax

  push compatibilityMessage
  call print32
  add esp, 4

  ; load our GDT and jump!
  lgdt [GDT64.pointer]
  jmp GDT64.code:_entry64

startedMessage:
  db 'Creating page tables', 10, 0

noMemoryFlagMessage:
  db 'GRUB did not set the memory flag', 10, 0

donePagesMessage:
  db 'Created page tables in memory', 10, 0

compatibilityMessage:
  db 'Entered compatibility mode', 10, 0

loadingPageMessage:
  db 'Loading another page', 10, 0

; yeah i know, this is somewhat pointless but whatevs man
align 4
_stack:
  times 0x2000 db 0
_endstack:

align 8
; from http://wiki.osdev.org/User:Stephanvanschaik/Setting_Up_Long_Mode
GDT64:                           ; Global Descriptor Table (64-bit).
    .null: equ $ - GDT64         ; The null descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0                         ; Access.
    db 0                         ; Granularity.
    db 0                         ; Base (high).
    .code: equ $ - GDT64         ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011000b                 ; Access.
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .data: equ $ - GDT64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010000b                 ; Access.
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.

bits 64

_entry64:
  cli
  mov ax, GDT64.data
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov rdi, inLongModeMessage
  call print64
  call configureAPIC
  call hang64

inLongModeMessage:
  db 'Entered long mode', 10, 0

