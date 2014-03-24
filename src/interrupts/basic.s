bits 64

global handle_interrupt_exception
global handle_interrupt_exception_code
global handle_interrupt_irq
global handle_interrupt_ipi
global handle_unknown_int
extern int_interrupt_exception
extern int_interrupt_exception_code
extern int_interrupt_irq
extern int_interrupt_ipi
extern task_switch_to_kernpage, print

%include "../pushaq.s"
%include "../ctxswitch.s"

handle_interrupt_exception
  beginframe
  mov rdi, [rsp + 0x80] ; vector argument
  call int_interrupt_exception
  endframe
  add rsp, 8
  iretq

; we have a different handler for this because typically exceptions will try to
; access an error code and stuff
handle_interrupt_exception_code:
  beginframe
  mov rdi, [rsp + 0x80] ; vector argument
  mov rsi, [rsp + 0x88] ; error number argument (if one is pushed)
  call int_interrupt_exception_code
  endframe
  add rsp, 0x10
  iretq

handle_interrupt_irq:
  beginframe
  mov rdi, [rsp + 0x80] ; vector argument
  call int_interrupt_irq
  endframe
  add rsp, 8
  iretq

handle_interrupt_ipi:
  beginframe
  mov rdi, [rsp + 0x80] ; vector argument
  call int_interrupt_ipi
  endframe
  add rsp, 8
  iretq

handle_unknown_int:
  mov rdi, .msg
  call print
  cli
  hlt ; returns on NMI
  jmp $
.msg:
  db 'Got unknwon interrupt!', 0xa, 0

global load_idtr
load_idtr:
  lidt [rdi]
  ret

; these two functions are pretty straight forward
global handle_dummy_lower
global handle_dummy_upper
handle_dummy_upper:
  mov al, 0x20
  out 0xa0, al
handle_dummy_lower:
  mov al, 0x20
  out 0x20, al
  iretq

