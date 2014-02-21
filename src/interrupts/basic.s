bits 64

extern int_keyboard
extern int_unknown_exception

%macro pushaq 0
  push rax
  push rcx
  push rdx
  push rbx
  push rsi
  push rdi
%endmacro

%macro popaq 0
  pop rdi
  pop rsi
  pop rbx
  pop rdx
  pop rcx
  pop rax
%endmacro

global handle_keyboard_exception
handle_keyboard_exception:
  pushaq
  call int_keyboard
  popaq
  iretq

global handle_unknown_exception
handle_unknown_exception:
  pushaq
  call int_unknown_exception
  popaq
  iretq

global load_idtr
load_idtr:
  lidt [rdi]
  ret
