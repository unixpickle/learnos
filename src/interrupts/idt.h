#include <stdint.h>
#include <shared/addresses.h>

#define IDT_VECTOR_TIMER 0x20
#define IDT_VECTOR_PRINT 0x21

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
void int_general_protection_fault(void * rip, uint64_t error);
void int_page_fault(void * rip, uint64_t error);
void int_math_fault();
void int_alignment_check();
void int_machine_check();
void int_simd_exception();
void int_unknown_exception(uint64_t retAddr, uint64_t codeSeg, uint64_t flags);

void int_irq0();
void int_irq1();
void int_irq2();
void int_irq3();
void int_irq4();
void int_irq5();
void int_irq6();
void int_irq7();
void int_irq8();
void int_irq9();
void int_irq10();
void int_irq11();
void int_irq12();
void int_irq13();
void int_irq14();
void int_irq15();

