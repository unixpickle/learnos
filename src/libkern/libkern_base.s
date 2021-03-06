bits 64

extern print, printHex
%include "../pushaq.s"

global hang
hang:
  hlt
  jmp hang

global halt
halt:
  hlt
  ret

global outb
outb:
  push rdx
  mov rdx, rdi ; first argument, port
  mov rax, rsi ; second argument, value
  out dx, al
  pop rdx
  ret

global inb
inb:
  xor rax, rax
  mov rdx, rdi
  in al, dx
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

global stack_log
stack_log:
  pushaq

  mov rdi, .startmessage
  call print

  mov rcx, [rsp + 0x40] ; orginal rdi
  mov rdx, 0x88
.loopy:
  push rcx
  push rdx
  xor rax, rax
  mov al, [rsp + rdx]
  mov rdi, rax
  call printHex
  mov rdi, .space
  call print
  pop rdx
  pop rcx
  add rdx, 1
  loop .loopy

  mov rdi, .endmessage
  call print

  popaq
  ret
.startmessage:
  db 'debug call: ', 0
.space:
  db ' ', 0
.endmessage:
  db 0xa, 0

global zero_page
zero_page:
  mov rax, 0
  shl rdi, 12
  mov rcx, 0x200
  rep stosq
  ret

