bits 64

extern syscall_entry

section .text

global syscall_configure_stack
syscall_configure_stack:
  ;we just ran:
  ; cli (done by syscall)
  ; xor rax, rax
  ; mov ss, ax
  ; push rbp
  ; mov rbp, rsp
  ; mov rsp, NEW STACK

  mov r11, cr3
  mov r10, 0x300000
  mov cr3, r10
  sti

  push rcx
  push r11
  mov rax, syscall_entry
  call rax

  ; clear interrupts while our page table and stack are on the fritz
  cli
  pop r11
  pop rcx
  mov cr3, r11
  leave
  sti

  sysret

