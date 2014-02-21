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

void configure_global_idt();
void int_keyboard();
void int_unknown_exception();

