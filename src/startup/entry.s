extern print32
extern hang32
extern basepage_initialize

extern print
extern hang
extern kernpage_initialize
extern apic_initialize
extern smp_initialize

bits 32

; these are multiboot flags
MBALIGN equ 1<<0
MEMINFO equ 1<<1
VIDEOINFO equ 1<<2
LINKINFO equ 1<<16
FLAGS equ LINKINFO | MEMINFO
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

LOADBASE equ 0x200000

%include "../shared/addresses.s"

section .multiboot

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

section .text

; this should be jumped to by GRUB2
start:
  ; I figure I might as well *know* the ESP will point to my data
  mov esp, _endstack
  mov ebp, esp

  ; set the Multiboot Information Structure pointer
  mov [MBOOT_INFO], ebx
  mov dword [MBOOT_INFO + 4], 0

  ; zero our flags
  mov esi, 0
  push esi
  popf

  ; reset the cursor position in memory
  mov dword [CURSOR_INFO], 0

  ; print our initialization message
  push startedMessage
  call print32
  add esp, 4

  call basepage_initialize
  push donePagesMessage
  call print32
  add esp, 4

  ; enable PAE-paging and PGE bit
  mov eax, cr4
  or eax, 0xa0
  mov cr4, eax

  ; set LM bit
  mov ecx, 0xC0000080 ; Extended Feature Enable Register
  rdmsr
  or eax, 0x101 ; long-mode bit & System Call Extensions bit
  wrmsr

  ; enable paging
  mov esi, PML4_START
  mov cr3, esi
  ; set the PG bit to enable paging
  mov eax, cr0
  or eax, 1 << 31
  mov cr0, eax

  ; say that we are now in compatibility mode
  push compatibilityMessage
  call print32
  add esp, 4

  ; load our GDT and jump!
  lgdt [GDT64_pointer]
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
  times 0xa000 db 0
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
    db 00100000b                 ; Granularity & 64-bit mode flag
    db 0                         ; Base (high).
    .data: equ $ - GDT64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access.
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .code_user: equ $ - GDT64    ; The user code descriptor
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 11111000b                 ; Access with DPL = 3
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .data_user: equ $ - GDT64
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 11110010b                 ; Access.
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
global GDT64_pointer
GDT64_pointer:
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

  ; if I don't set SS=0, iret causes a #GP
  xor ax, ax
  mov ss, ax

  mov rdi, inLongModeMessage
  call print
  call kernpage_initialize
  call apic_initialize
  call smp_initialize

inLongModeMessage:
  db 'Entered long mode', 10, 0

