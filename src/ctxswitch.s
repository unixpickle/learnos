%macro beginframe 0
  pushaq
  mov rax, cr3
  push rax
  mov rax, rsp
  push rax
  call thread_switch_to_kernpage
%endmacro

%macro endframe 0
  pop rax ; rsp
  pop rbx ; cr3
  add rax, 8
  mov rsp, rax
  mov cr3, rbx
  popaq
%endmacro

