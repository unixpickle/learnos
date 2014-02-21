#include "idt.h"
#include "basic.h"

static void _initialize_idt(idt_entry * ptr);

void configure_global_idt() {
  volatile idt_pointer * idtr = (volatile idt_pointer *)IDTR_PTR;
  idtr->limit = 0x100 * 16;
  idtr->virtualAddress = IDT_PTR;
  _initialize_idt((idt_entry *)IDT_PTR);
  // TODO: add handlers
  load_idtr((void *)idtr);
}

void int_keyboard() {
  print64("Got keyboard interrupt.\n");
}

void int_unknown_exception() {
  print64("Got unknown interrupt.\n");
}

static void _initialize_idt(idt_entry * ptr) {
  idt_entry def;

  uint64_t excAddress = (uint64_t)handle_unknown_exception;
  def.low_offset = excAddress & 0xffff;
  def.mid_offset = (excAddress >> 16) & 0xffff;
  def.high_offset = excAddress >> 32;

  def.code_segment = 0x8;
  def.reserved1 = 0;
  def.reserved2 = 0;
  def.flags = 0xf | (1 << 7); // make this a trap gate because i'm trapped in this silly OS

  int i;
  for (i = 0; i < 0x100; i++) {
    ptr[i] = def;
  }
}

