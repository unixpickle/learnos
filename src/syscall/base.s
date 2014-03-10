bits 64

%include "../pushaq.s"
%include "../ctxswitch.s"

extern print

extern syscall_print_method, task_switch_to_kernpage

global syscall_print
syscall_print:
  beginframe
  mov rdi, [rsp + 0x50]
  call syscall_print_method
  endframe
  iretq
.message: 
  db 'hey there bro', 0xa, 0
