bits 64

global hang64
hang64:
  hlt
  jmp hang64

global outb64
outb64:
  push rdx
  mov rdx, rdi ; first argument, port
  mov rax, rsi ; second argument, value
  out dx, al
  pop rdx
  ret

global cpuid
cpuid:
  push rbx
  push rbp
  mov rbp, rsp

  ; push arguments to stack
  push rcx
  push rdx
  push rsi

  xor rax, rax
  mov eax, edi ; argument 1, selector
  cpuid

  ; output registers to destinations
  mov rdi, [rsp] ; argument 2, ebx out
  mov [rdi], ebx
  mov rdi, [rsp+8] ; argument 3, ecx out
  mov [rdi], ecx
  mov rdi, [rsp+16] ; argument 4, edx out
  mov [rdi], edx

  leave
  pop rbx ; this register has to be preserved, apparently
  ret

global msr_read
msr_read:
  mov ecx, edi
  xor rax, rax
  rdmsr ; input ecx, output edx:eax
  shl rdx, 32
  or rax, rdx
  ret

global msr_write
msr_write:
  mov rdx, rsi ; upper half of value
  shr rdx, 32
  mov eax, esi ; lower half of value
  mov ecx, edi ; selector
  wrmsr
  ret

global invalidate_page
invalidate_page:
  shl rdi, 12
  invlpg [rdi]
  ret

global enable_interrupts
enable_interrupts:
  sti
  ret

global disable_interrupts
disable_interrupts:
  cli
  ret

