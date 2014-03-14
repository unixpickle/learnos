extern printf_impl

; this is a hack if i've seen one, but it's kinda nice!
global printf
printf:
  pop rax ; return address
  push r9
  push r8
  push rcx
  push rdx
  push rsi
  push rdi
  push rax
  lea rdi, [rsp + 8]
  call printf_impl
  pop rax
  add rsp, 0x30
  jmp rax

