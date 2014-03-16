bits 64

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
extern int_irq0
extern int_irq1
extern int_irq2
extern int_irq3
extern int_irq4
extern int_irq5
extern int_irq6
extern int_irq7
extern int_irq8
extern int_irq9
extern int_irq10
extern int_irq11
extern int_irq12
extern int_irq13
extern int_irq14
extern int_irq15
extern task_switch_to_kernpage

%include "../pushaq.s"
%include "../ctxswitch.s"

global handle_div_zero
handle_div_zero:
  beginframe
  call int_div_zero
  endframe
  iretq

global handle_debugger
handle_debugger:
  beginframe
  call int_debugger
  endframe
  iretq

global handle_nmi
handle_nmi:
  beginframe
  call int_nmi
  endframe
  iretq

global handle_breakpoint
handle_breakpoint:
  beginframe
  call int_breakpoint
  endframe
  iretq

global handle_overflow
handle_overflow:
  beginframe
  call int_overflow
  endframe
  iretq

global handle_bounds
handle_bounds:
  beginframe
  call int_bounds
  endframe
  iretq

global handle_invalid_opcode
handle_invalid_opcode:
  push rbp
  mov rbp, rsp
  beginframe

  mov rdi, [rbp + 8]
  call int_invalid_opcode

  endframe
  leave
  iretq

global handle_coprocessor_not_available
handle_coprocessor_not_available:
  beginframe
  call int_coprocessor_not_available
  endframe
  iretq

global handle_double_fault
handle_double_fault:
  push rbp
  mov rbp, rsp
  beginframe
  mov rdi, [rbp + 18]
  call int_double_fault
  endframe
  leave
  iretq

global handle_coprocessor_segment_overrun
handle_coprocessor_segment_overrun:
  beginframe
  call int_coprocessor_segment_overrun
  endframe
  iretq

global handle_invalid_tss
handle_invalid_tss:
  beginframe
  call int_invalid_tss
  endframe
  iretq

global handle_segmentation_fault
handle_segmentation_fault:
  beginframe
  call int_segmentation_fault
  endframe
  iretq

global handle_stack_fault
handle_stack_fault:
  beginframe
  call int_stack_fault
  endframe
  iretq

global handle_general_protection_fault
handle_general_protection_fault:
  beginframe
  mov rdi, [rsp + 0x88]
  mov rsi, [rsp + 0x80]
  call int_general_protection_fault
  endframe
  add rsp, 8
  iretq

global handle_page_fault
handle_page_fault:
  beginframe
  mov rdi, [rsp + 0x78]
  mov rsi, [rsp + 0x70]
  call int_page_fault
  endframe
  add rsp, 8
  iretq

global handle_math_fault
handle_math_fault:
  beginframe
  call int_math_fault
  endframe
  iretq

global handle_alignment_check
handle_alignment_check:
  beginframe
  call int_alignment_check
  endframe
  iretq

global handle_machine_check
handle_machine_check:
  beginframe
  call int_machine_check
  endframe
  iretq

global handle_simd_exception
handle_simd_exception:
  beginframe
  call int_simd_exception
  endframe
  iretq

global handle_spurious
handle_spurious:
  iretq

global handle_unknown_exception
handle_unknown_exception:
  push rbp
  mov rbp, rsp
  beginframe

  mov rdi, [rbp + 8]
  xor rsi, rsi
  mov si, [rbp + 16]
  mov rdx, [rbp + 18]
  call int_unknown_exception

  endframe
  leave
  iretq

global handle_irq0
handle_irq0:
  beginframe
  call int_irq0
  endframe
  iretq

global handle_irq1
handle_irq1:
  beginframe
  call int_irq1
  endframe
  iretq

global handle_irq2
handle_irq2:
  beginframe
  call int_irq2
  endframe
  iretq

global handle_irq3
handle_irq3:
  beginframe
  call int_irq3
  endframe
  iretq

global handle_irq4
handle_irq4:
  beginframe
  call int_irq4
  endframe
  iretq

global handle_irq5
handle_irq5:
  beginframe
  call int_irq5
  endframe
  iretq

global handle_irq6
handle_irq6:
  beginframe
  call int_irq6
  endframe
  iretq

global handle_irq7
handle_irq7:
  beginframe
  call int_irq7
  endframe
  iretq

global handle_irq8
handle_irq8:
  beginframe
  call int_irq8
  endframe
  iretq

global handle_irq9
handle_irq9:
  beginframe
  call int_irq9
  endframe
  iretq

global handle_irq10
handle_irq10:
  beginframe
  call int_irq10
  endframe
  iretq

global handle_irq11
handle_irq11:
  beginframe
  call int_irq11
  endframe
  iretq

global handle_irq12
handle_irq12:
  beginframe
  call int_irq12
  endframe
  iretq

global handle_irq13
handle_irq13:
  beginframe
  call int_irq13
  endframe
  iretq

global handle_irq14
handle_irq14:
  beginframe
  call int_irq14
  endframe
  iretq

global handle_irq15
handle_irq15:
  beginframe
  call int_irq15
  endframe
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

