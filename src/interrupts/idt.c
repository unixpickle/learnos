#include "idt.h"
#include "lapic.h"
#include "basic.h"
#include "inttable.h"
#include <shared/addresses.h> // for PIT
#include <stdio.h>

static void _initialize_idt(idt_entry_t * ptr);

void configure_dummy_idt() {
  idt_pointer * idtr = (idt_pointer *)IDTR_PTR;
  idtr->limit = 0x1000 - 1;
  idtr->virtualAddress = IDT_PTR;

  // create entries for low IRQs
  idt_entry_t * table = (idt_entry_t *)IDT_PTR;
  _initialize_idt((idt_entry_t *)table);

  idt_entry_t lowerEntry = IDT_ENTRY_INIT(((uint64_t)handle_dummy_lower), 0x8e);
  idt_entry_t upperEntry = IDT_ENTRY_INIT(((uint64_t)handle_dummy_upper), 0x8e);

  int i;
  for (i = 0x8; i <= 0xf; i++) {
    table[i] = lowerEntry;
  }
  for (i = 0x70; i <= 0x78; i++) {
    table[i] = upperEntry;
  }

  load_idtr((void *)idtr);
}

void configure_global_idt() {
  volatile idt_pointer * idtr = (volatile idt_pointer *)IDTR_PTR;
  idtr->limit = 0x1000 - 1;
  idtr->virtualAddress = IDT_PTR;

  idt_entry_t * table = (idt_entry_t *)IDT_PTR;
  _initialize_idt(table);

  // setup the interrupt handlers
  uint64_t callRoutines[4] = {
    (uint64_t)&handle_interrupt_exception,
    (uint64_t)&handle_interrupt_exception_code,
    (uint64_t)&handle_interrupt_irq,
    (uint64_t)&handle_interrupt_ipi,
  };

  int i;
  for (i = 0; i < sizeof(gIDTHandlers) / sizeof(int_handler_t); i++) {
    uint64_t handler = (uint64_t)&gIDTHandlers[i];
    uint64_t flags = 0x8e;

    // set the correct method
    gIDTHandlers[i].function = callRoutines[gIDTHandlers[i].function];
    
    idt_entry_t entry = IDT_ENTRY_INIT(handler, flags);
    table[gIDTHandlers[i].argument] = entry;
  }

  load_idtr((void *)idtr);
}

void int_interrupt_exception(uint64_t vec) {
  print("Got exception vector ");
  printHex(vec);
  print("\n");
  __asm__("cli\nhlt");
  // TODO: here, save task state and terminate it
}

void int_interrupt_exception_code(uint64_t vec, uint64_t code) {
  print("Got exception vector ");
  printHex(vec);
  print(" with code ");
  printHex(code);
  print("\n");
  __asm__("cli\nhlt");
  // TODO: here, save task state and terminate it, or do a page fault
}

void int_interrupt_irq(uint64_t vec) {
  if (vec == 0x20 || vec == 0x22) {
    PIT_TICK_COUNT++;
  }
  if (lapic_is_in_service(vec)) {
    lapic_send_eoi();
  }
}

void int_interrupt_ipi(uint64_t vec) {
  if (vec == 0x31) {
    print("GOT PAGE IPI\n");
  }
  if (lapic_is_in_service(vec)) {
    lapic_send_eoi();
  }
}

static void _initialize_idt(idt_entry_t * ptr) {
  idt_entry_t entry = IDT_ENTRY_INIT((uint64_t)handle_unknown_int, 0x8e);
  int i;
  for (i = 0; i < 0x100; i++) {
    ptr[i] = entry;
  }
}

