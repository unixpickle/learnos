bits 32

; these are multiboot flags
MBALIGN equ 1<<0
MEMINFO equ 1<<1
VIDEOINFO equ 1<<2
LINKINFO equ 1<<16
FLAGS equ LINKINFO
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

; these are offsets in physical memory that learnos
; uses for various information
CUROFFSET equ 0x100000
PML4_START equ 0x110000
PDPT_START equ 0x111000
PDT_START equ 0x112000
PT_START equ 0x113000
LOADBASE equ 0x320000

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

  mov edi, 0xb8000
  mov [CUROFFSET], edi
  mov dword [CUROFFSET + 4], 0

  ; print our initialization message
  push startedMessage
  call print
  add esp, 4

configurePages:
  ; make sure memory information flag is set
  mov esi, [ebp - 4]
  mov eax, [esi]
  or eax, 1
  jnz .getAvailable
  push noMemoryFlagMessage
  call print
  jmp hang

.getAvailable:
  ; pushes the amount of 1024 byte blocks (after 1MB) to the stack
  mov eax, [esi + 8]
  push eax

  ; zero the PML4, PDPT, PDT, PT
  mov edi, PML4_START
  xor eax, eax
  mov ecx, 0x3000 + 0x200000
  ; TODO: figure out how to use rep here
.zeroPages:
  mov [edi], al
  inc edi
  loop .zeroPages

  ; create page tables
  mov eax, [esp] ; eax will be our pages remaining in physical memory counter
  shr eax, 2 ; 4k blocks instead of 1k blocks
  add eax, 1 << 8 ; there are 2^8 pages in the first 1MB
  mov edi, PT_START ; pointer to our place in page table
  mov esi, 3 ; pointer to physical memory + flags
  mov ebx, PDT_START ; pointer to PDT offset
.loopPDT:
  ; create a PDT entry
  mov ecx, 0x200 ; create 512 entries, 8 bytes each
  mov [ebx], edi
  or byte [ebx], 3
  add ebx, 8

  pushad
  push loadingPageMessage
  call print
  add esp, 4
  popad

.loopPT:
  ; create page table entries until we run out of memory
  mov [edi], esi
  add edi, 8
  add esi, 0x1000
  sub eax, 1
  jz doneCreatingPages
  loop .loopPT
  jmp .loopPDT

doneCreatingPages:
  push donePagesMessage
  call print
  add esp, 4

  ; put in the appropriate addresses
  mov dword [PML4_START], PDPT_START + 3
  mov dword [PDPT_START], PDT_START + 3
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
  call print
  add esp, 4

  ; load our GDT and jump!
  lgdt [GDT64.pointer]
  jmp GDT64.code:_entry64

hang:
  hlt
  jmp hang

print:
  push ebp
  mov ebp, esp
  mov esi, [ebp + 8]
  mov edi, [CUROFFSET]
  mov ah, 0x0a
  mov ecx, 160
_printLoop:
  mov al, [esi]
  cmp al, 0
  je _printEnd
  mov word [edi], ax
  add edi, 2
  inc esi
  sub ecx, 2
  jmp _printLoop
_printEnd:
  add edi, ecx
  mov [CUROFFSET], edi
  xor eax, eax
  leave
  ret

startedMessage:
  db 'Creating page tables', 0

noMemoryFlagMessage:
  db 'GRUB did not set the memory flag', 0

donePagesMessage:
  db 'Created page tables in memory', 0

compatibilityMessage:
  db 'Entered compatibility mode', 0

loadingPageMessage:
  db 'Loading another page', 0

; yeah i know, this is somewhat pointless but whatevs man
align 4
_stack:
  times 4096 db 0
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

extern hang64
extern print64
extern configureAPIC

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
  db 'Entered long mode', 0

