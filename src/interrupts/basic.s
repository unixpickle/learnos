bits 64

extern int_keyboard
extern int_div_zero
extern int_debugger
extern int_nmi
extern int_breakpoint
extern int_overflow
extern int_bounds
extern int_invalid_opcode
extern int_coprocessor_not_available
extern int_double_fault
extern int_coprocessor_segment_overrun
extern int_invalid_tss
extern int_segmentation_fault
extern int_stack_fault
extern int_general_protection_fault
extern int_page_fault
extern int_math_fault
extern int_alignment_check
extern int_machine_check
extern int_simd_exception
extern int_unknown_exception

%macro pushaq 0
  push rax
  push rcx
  push rdx
  push rbx
  push rsi
  push rdi
  push r12
  push r13
  push r14
  push r15
%endmacro

%macro popaq 0
  pop r15
  pop r14
  pop r13
  pop r12
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

global handle_div_zero
handle_div_zero:
  pushaq
  call int_div_zero
  popaq
  iretq

global handle_debugger
handle_debugger:
  pushaq
  call int_debugger
  popaq
  iretq

global handle_nmi
handle_nmi:
  pushaq
  call int_nmi
  popaq
  iretq

global handle_breakpoint
handle_breakpoint:
  pushaq
  call int_breakpoint
  popaq
  iretq

global handle_overflow
handle_overflow:
  pushaq
  call int_overflow
  popaq
  iretq

global handle_bounds
handle_bounds:
  pushaq
  call int_bounds
  popaq
  iretq

global handle_invalid_opcode
handle_invalid_opcode:
  push rsp
  mov rbp, rsp
  pushaq

  mov rdi, [rbp + 8]
  call int_invalid_opcode

  popaq
  leave
  iretq

global handle_coprocessor_not_available
handle_coprocessor_not_available:
  pushaq
  call int_coprocessor_not_available
  popaq
  iretq

global handle_double_fault
handle_double_fault:
  push rbp
  mov rbp, rsp
  pushaq
  mov rdi, [rbp + 18]
  call int_double_fault
  popaq
  leave
  iretq

global handle_coprocessor_segment_overrun
handle_coprocessor_segment_overrun:
  pushaq
  call int_coprocessor_segment_overrun
  popaq
  iretq

global handle_invalid_tss
handle_invalid_tss:
  pushaq
  call int_invalid_tss
  popaq
  iretq

global handle_segmentation_fault
handle_segmentation_fault:
  pushaq
  call int_segmentation_fault
  popaq
  iretq

global handle_stack_fault
handle_stack_fault:
  pushaq
  call int_stack_fault
  popaq
  iretq

global handle_general_protection_fault
handle_general_protection_fault:
  pushaq
  call int_general_protection_fault
  popaq
  iretq

global handle_page_fault
handle_page_fault:
  pushaq
  call int_page_fault
  popaq
  iretq

global handle_math_fault
handle_math_fault:
  pushaq
  call int_math_fault
  popaq
  iretq

global handle_alignment_check
handle_alignment_check:
  pushaq
  call int_alignment_check
  popaq
  iretq

global handle_machine_check
handle_machine_check:
  pushaq
  call int_machine_check
  popaq
  iretq

global handle_simd_exception
handle_simd_exception:
  pushaq
  call int_simd_exception
  popaq
  iretq

global handle_unknown_exception
handle_unknown_exception:
  push rsp
  mov rbp, rsp
  pushaq

  mov rdi, [rbp + 8]
  xor rsi, rsi
  mov si, [rbp + 16]
  mov rdx, [rbp + 18]
  call int_unknown_exception

  popaq
  leave
  iretq

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

