#include <stdint.h>
#include <shared/addresses.h>

typedef struct {
  uint16_t limit;
  uint64_t virtualAddress;
} __attribute__((packed)) idt_pointer;

void configure_dummy_idt();
void configure_global_idt();

void int_interrupt_exception(uint64_t vec);
void int_interrupt_exception_code(uint64_t vec, uint64_t code);
void int_interrupt_irq(uint64_t vec);
void int_interrupt_ipi(uint64_t vec);

