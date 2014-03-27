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

global sys_exit
sys_exit:
  mov rdi, 3
  syscall

global sys_thread_exit
sys_thread_exit:
  mov rdi, 4
  syscall

global sys_wants_interrupts
sys_wants_interrupts:
  mov rdi, 5
  syscall
  ret

global sys_get_interrupts
sys_get_interrupts:
  mov rdi, 6
  syscall
  ret

global sys_open
sys_open:
  mov rdi, 7
  syscall
  ret

global sys_connect
sys_connect:
  mov rdx, rsi ; PID
  mov rsi, rdi ; fd
  mov rdi, 8
  syscall
  ret

global sys_close
sys_close:
  mov rsi, rdi
  mov rdi, 9
  syscall
  ret

global sys_write
sys_write:
  mov r8, rdx ; 3rd argument since rcx is reserved for syscall
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 10
  syscall
  ret

global sys_read
sys_read:
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 11
  syscall
  ret

global sys_poll
sys_poll:
  mov rdi, 12
  syscall
  ret

global sys_remote_pid
sys_remote_pid:
  mov rsi, rdi
  mov rdi, 13
  syscall
  ret

global sys_remote_uid
sys_remote_uid:
  mov rsi, rdi
  mov rdi, 14
  syscall
  ret

global sys_in
sys_in:
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 15
  syscall
  ret

global sys_out
sys_out:
  mov r8, rdx
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 0x10
  syscall
  ret

