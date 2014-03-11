bits 64

%include "../pushaq.s"
%include "../ctxswitch.s"

extern print

extern syscall_print_method, syscall_sleep_method, syscall_getint_method
extern task_switch_to_kernpage, task_save_state

global syscall_print
syscall_print:
  beginframe
  mov rdi, [rsp + 0x50]
  call syscall_print_method
  endframe
  iretq

global syscall_sleep
syscall_sleep:
  beginframe
  mov rdi, [rsp + 0x50]
  call syscall_sleep_method
  endframe
  iretq

global syscall_getint
syscall_getint:
  call task_save_state
  call syscall_getint_method ; never returns

