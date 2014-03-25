bits 64

section .text

global sys_print
sys_print:
  mov rsi, rdi
  xor rdi, rdi
  syscall
  ret

global sys_get_time
sys_get_time:
  mov rdi, 1
  syscall
  ret

global sys_sleep
sys_sleep:
  mov rsi, rdi
  mov rdi, 2
  syscall
  ret

