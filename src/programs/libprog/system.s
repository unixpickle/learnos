bits 64

global sys_print
sys_print:
  int 0x21
  ret

global sys_sleep
sys_sleep:
  int 0x22
  ret

global sys_getint
sys_getint:
  int 0x23
  ret

global sys_in
sys_in:
  mov rsi, rdi
  mov rdi, 1
  int 0x24
  ret

global sys_out
sys_out:
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 0
  int 0x24
  ret

global sys_get_time
sys_get_time:
  mov rdi, 0
  int 0x22
  ret

