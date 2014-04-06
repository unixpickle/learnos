bits 64

%include "../shared/addresses.s"
extern syscall_entry, syscall_return

section .text

global syscall_configure_stack
syscall_configure_stack:
  ;we just ran:
  ; cli (done by syscall)
  ; xor rax, rax
  ; mov ss, ax
  ; mov r11, rsp
  ; mov rsp, NEW STACK

  mov r10, cr3
  mov rax, PML4_START
  mov cr3, rax
  sti

  push r10 ; cr3
  push r11 ; rsp
  push rcx ; rip
  mov rcx, r8 ; 4th argument
  call syscall_entry

  ; clear interrupts and return from the syscall
  cli
  push r15
  push r14
  push r13
  push r12
  push rbx
  push rbp
  push rax
  mov rdi, rsp
  call syscall_return

