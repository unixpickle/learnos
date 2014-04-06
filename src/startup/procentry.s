GDT64_PTR equ 0x5ff6
GDT64_code equ 0x8
GDT64_data equ 0x10
START_STACK equ 0x4000

%include "../shared/addresses.s"

extern kernpage_alloc_virtual
extern proc_initialize
extern print

bits 16
section .text

global proc_entry
proc_entry:
  mov sp, START_STACK
  mov bp, sp
  cli

  mov eax, 10100000b ; set PAE and PGE bit
  mov cr4, eax

  mov edx, PML4_START
  mov cr3, edx

  ; set the LME and SCE bits of the EFER msr
  mov ecx, 0xC0000080
  rdmsr
  or eax, 0x101
  wrmsr

  ; enable paging and protection simultaneously
  mov ebx, cr0
  or ebx, 0x80000001
  mov cr0, ebx

  mov ebx, GDT64_PTR
  lgdt [ebx]
  jmp GDT64_code:((proc_entry_64 - proc_entry) + 0x6000)

bits 64

proc_entry_64:
  ; this will no longer be running in real mode, and it will be loaded
  ; past the 1MB mark
  mov ax, GDT64_data
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; if I don't set SS=0, iret causes a #GP
  xor ax, ax
  mov ss, ax

  mov rbp, START_STACK
  mov rsp, rbp

  mov rbx, initiate_routine
  call rbx

global proc_entry_end
proc_entry_end:

initiate_routine:
  call kernpage_alloc_virtual

  ; use rax as our stack
  mov rbx, rax
  shl rbx, 12
  mov rbp, rbx
  mov rsp, rbp
  add rsp, 0x1000

  ; now that we have our new stack, call smp_entry(page)
  mov rdi, rax
  call proc_initialize

  ; should never happen
  mov rdi, .donemessage
  call print
  cli
  hlt
.donemessage:
  db 'somehow, proc_initialize returned', 0x0a, 0

global load_new_gdt
load_new_gdt:
  mov rax, GDT64_PTR
  lgdt [rax]
  ret

