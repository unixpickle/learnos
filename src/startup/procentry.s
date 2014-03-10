PML4_START equ 0x300000
GDT64_PTR equ 0x5ff6
GDT64_code equ 0x8
GDT64_data equ 0x10
START_STACK equ 0x4000

extern kernpage_alloc_virtual
extern kernpage_lock
extern kernpage_unlock
extern smp_entry
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

  ; set the LME bit of the EFER msr
  mov ecx, 0xC0000080
  rdmsr
  or eax, 0x100
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
  call kernpage_lock
  call kernpage_alloc_virtual
  push rax
  call kernpage_unlock
  pop rax

  ; use rax as our stack
  mov rbx, rax
  shl rbx, 12
  mov rbp, rbx
  mov rsp, rbp
  add rsp, 0x1000

  ; now that we have our new stack, call smp_entry(page)
  mov rdi, rax
  call smp_entry

global bootstrap_task
bootstrap_task:
.start:
  mov rcx, 300000000
.loop:
  loop .loop
  mov rdi, (.msg - bootstrap_task + 0x10500400000)
  ;int 0x21
  ; cause a GP fault
  hlt
  jmp .start
.msg:
  db 'Hey there, user space!', 0xa, 0

global bootstrap_task_end
bootstrap_task_end:

global load_new_gdt
load_new_gdt:
  mov rbx, GDT64_PTR
  lgdt [rbx]
  ret

