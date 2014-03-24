bits 64

global _antest_thread_run
_antest_thread_run:
  jmp antest_thread_run

global _antest_save_return_state
_antest_save_return_state:
  jmp antest_save_return_state;

global antest_thread_run
antest_thread_run:
  add rsi, 0x40 ; point directly to the state

  ; push SS
  xor rax, rax
  mov ax, ss
  push rax
  
  ; push RSP
  mov rax, [rsi + 0x38]
  push rax
  
  ; push RFLAGS
  mov rax, [rsi + 0x88]
  push rax

  ; push CS
  xor rax, rax
  mov ax, cs
  push rax
  
  ; push RIP
  mov rax, [rsi + 0x80]
  push rax

  ; mov all the registers to the stack
  sub rsp, 0x78
  
  ; copy up to rbp
  mov rcx, 7
  mov rdi, rsp
  rep movsq
  
  ; copy up to r15
  add rsi, 0x8 ; skip the stack pointer
  mov rcx, 8
  rep movsq
  
  pop rax
  pop rbx
  pop rcx
  pop rdx
  pop rdi
  pop rsi
  pop rbp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
  iretq

global antest_save_return_state
antest_save_return_state:
  ; rsi is an argument, and rdx is a function to call
  ; rdi is our thread pointer...move it to point to the state
  add rdi, 0x40
  
  ; rbx
  mov [rdi + 8], rbx
  
  ; rbp
  mov [rdi + 0x30], rbp
  
  ; rsp
  lea rax, [rsp + 8]
  mov [rdi + 0x38], rax
  
  ; r12 - r15
  mov [rdi + 0x60], r12
  mov [rdi + 0x68], r13
  mov [rdi + 0x70], r14
  mov [rdi + 0x78], r15
  
  ; rip
  mov rax, [rsp]
  mov [rdi + 0x80], rax
  
  ; flags
  pushfq
  pop rax
  mov [rdi + 0x88], rax
  
  ; we need to align the stack on OS X, and it's nice to have frames
  push rbp
  mov rbp, rsp
  ; call the continuation which should never return
  mov rdi, rsi
  call rdx
  
  ; SHOULD NEVER BE REACHED
  leave
  ret