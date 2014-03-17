bits 64

%include "../pushaq.s"
%include "../ctxswitch.s"

extern syscall_print_method, syscall_sleep_method, syscall_getint_method
extern syscall_thread_exit_method, syscall_pid_kill_method
extern task_switch_to_kernpage, task_save_state, cpu_dedicated_stack

section .text

global syscall_interrupt
syscall_interrupt:
  cmp rax, 7
  jae .syscall_not_found
  shl rax, 3
  mov r11, .syscall_table
  mov r10, [rax + r11]
  jmp r10
.syscall_table:
  dq syscall_print
  dq syscall_sleep
  dq syscall_getint
  dq syscall_pinin
  dq syscall_pinout
  dq syscall_thread_exit
  dq syscall_pid_kill
.syscall_not_found:
  mov rax, 0
  ret

syscall_print:
  beginframe
  mov rdi, [rsp + 0x50]
  call syscall_print_method
  endframe
  iretq

syscall_sleep:
  call task_save_state
  call cpu_dedicated_stack
  mov rsp, rax
  call syscall_sleep_method

syscall_getint:
  call task_save_state
  call cpu_dedicated_stack
  mov rsp, rax
  call syscall_getint_method ; never returns

syscall_pinin:
  xor rax, rax
  mov dx, di
  in al, dx
  iretq

syscall_pinout:
  mov ax, si
  mov dx, di
  out dx, al
  iretq

syscall_thread_exit:
  call syscall_thread_exit_method
  ; iretq will never be called anyway
  ; iretq

syscall_pid_kill:
  call syscall_pid_kill_method
  iretq

