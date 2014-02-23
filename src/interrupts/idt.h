#include <shared/types.h>
#include <shared/addresses.h>
#include <libkern64/stdio.h>

typedef struct {
  uint16_t limit;
  uint64_t virtualAddress;
} __attribute__((packed)) idt_pointer;

typedef struct {
  uint16_t low_offset;
  uint16_t code_segment;
  uint8_t reserved1;
  uint8_t flags;
  uint16_t mid_offset;
  uint32_t high_offset;
  uint32_t reserved2;
} __attribute__((packed)) idt_entry;

void configure_dummy_idt();
void configure_global_idt();

void int_keyboard();
void int_div_zero();
void int_debugger();
void int_nmi();
void int_breakpoint();
void int_overflow();
void int_bounds();
void int_invalid_opcode(uint64_t ptr);
void int_coprocessor_not_available();
void int_double_fault(uint64_t error);
void int_coprocessor_segment_overrun();
void int_invalid_tss();
void int_segmentation_fault();
void int_stack_fault();
void int_general_protection_fault();
void int_page_fault();
void int_math_fault();
void int_alignment_check();
void int_machine_check();
void int_simd_exception();
void int_unknown_exception(uint64_t retAddr, uint64_t codeSeg, uint64_t flags);

