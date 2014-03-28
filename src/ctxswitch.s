%macro beginframe 0
  pushaq
  mov rax, rsp
  push rax
  mov rax, cr3
  push rax
  call thread_switch_to_kernpage
%endmacro

%macro endframe 0
  pop rbx ; cr3
  pop rax ; rsp
  mov rsp, rax
  mov cr3, rbx
  popaq
%endmacro

