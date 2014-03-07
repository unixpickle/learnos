bits 64

global task_run_with_stack
task_run_with_stack:
  mov rsp, rsi
  mov rbp, rsi
  mov rsi, rdi
  jmp rdx

; leapfrog off a thread's stack and setup the context for an iretq
global task_switch
task_switch:
  lea rsi, [rdi + 0x20] ; offset of the `state` field
  mov rsp, [rsi] ; enter the thread's stack
  mov rbp, [rsi + 8]

  ; push state for iretq
  ; push FLAGS
  mov rax, [rsi + 0x20]
  push rax
  ; push CS with correct priviledge level
  mov ax, 0x8
  mov rax, [rsi + 0x10]
  cmp rax, 0x300000
  je .pushCS
  or ax, 0x3 ; PL = 3
.pushCS
  push ax ; check if it needs kernel priviledge level
  ; push rip
  mov rax, [rsi + 0x18]
  push rax

  ; load the appropriate registers
  mov rax, [rsi + 0x28]
  mov rbx, [rsi + 0x30]
  mov rcx, [rsi + 0x38]
  mov rdx, [rsi + 0x40]
  mov rdi, [rsi + 0x50]

  ; finally, get the rsi value
  push rbx
  mov rbx, [rsi + 0x48];
  mov rsi, rbx
  pop rbx
  iretq

