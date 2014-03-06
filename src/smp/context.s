global task_run_with_stack
task_run_with_stack:
  mov rsp, rsi
  mov rbp, rsi
  mov rsi, rdi
  jmp rdx

