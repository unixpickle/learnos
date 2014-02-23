#include "idt.h"
#include "basic.h"

static void _initialize_idt(idt_entry * ptr);
static void _make_entry(idt_entry * out, void (* ptr)());

void configure_dummy_idt() {
  volatile idt_pointer * idtr = (idt_pointer *)IDTR_PTR;
  idtr->limit = 0x1000 - 1;
  idtr->virtualAddress = IDT_PTR;

  // create entries for low IRQs
  volatile idt_entry * table = (idt_entry *)IDT_PTR;
  idt_entry entry;
  int i;
  _initialize_idt((idt_entry *)table);
  _make_entry(&entry, handle_dummy_lower);
  for (i = 0x8; i <= 0xf; i++) {
    table[i] = entry;
  }
  _make_entry(&entry, handle_dummy_upper);
  for (i = 0x70; i <= 0x78; i++) {
    table[i] = entry;
  }

  load_idtr((void *)idtr);
}

void configure_global_idt() {
  volatile idt_pointer * idtr = (volatile idt_pointer *)IDTR_PTR;
  idtr->limit = 0x1000 - 1;
  idtr->virtualAddress = IDT_PTR;
  _initialize_idt((idt_entry *)IDT_PTR);
  // TODO: add handlers
  load_idtr((void *)idtr);
}

void int_keyboard() {
  print64("Got keyboard interrupt.\n");
}

void int_div_zero() {
  print64("got div zero\n");
}

void int_debugger() {
  print64("got debugger\n");
}

void int_nmi() {
  print64("got nmi\n");
}

void int_breakpoint() {
  print64("got breakpoint\n");
}

void int_overflow() {
  print64("got overflow\n");
}

void int_bounds() {
  print64("got bounds\n");
}

void int_invalid_opcode(uint64_t ptr) {
  print64("got invalid opcode: ");
  printHex64(ptr);
  print64("\n");
  hang64();
}

void int_coprocessor_not_available() {
  print64("got coprocessor not available\n");
}

void int_double_fault(uint64_t error) {
  print64("got double fault: ");
  printHex64(error);
  print64("\n");
}

void int_coprocessor_segment_overrun() {
  print64("got coprocessor segment overrun\n");
}

void int_invalid_tss() {
  print64("got invalid tss\n");
}

void int_segmentation_fault() {
  print64("got segmentation fault\n");
}

void int_stack_fault() {
  print64("got stack fault\n");
}

void int_general_protection_fault() {
  print64("got general protection fault\n");
  hang64();
}

void int_page_fault() {
  print64("got page fault\n");
}

void int_math_fault() {
  print64("got math fault\n");
}

void int_alignment_check() {
  print64("got alignment check\n");
}

void int_machine_check() {
  print64("got machine check\n");
}

void int_simd_exception() {
  print64("got simd exception\n");
}

void int_unknown_exception(uint64_t retAddr, uint64_t codeSeg, uint64_t flags) {
  print64("INT, retAddr=");
  printHex64(retAddr);
  print64(", codeSeg=");
  printHex64(codeSeg);
  print64(", flags=");
  printHex64(flags);
  print64("\n");
}

static void _initialize_idt(idt_entry * ptr) {
  void (* functions[])() = {handle_div_zero, handle_debugger, handle_nmi, handle_breakpoint, handle_overflow, handle_bounds, handle_invalid_opcode, handle_coprocessor_not_available, handle_double_fault, handle_coprocessor_segment_overrun, handle_invalid_tss, handle_segmentation_fault, handle_stack_fault, handle_general_protection_fault, handle_page_fault, handle_unknown_exception, handle_math_fault, handle_alignment_check, handle_machine_check, handle_simd_exception};
  idt_entry def;

  int i;
  for (i = 0; i < 0x14; i++) {
    _make_entry(&def, functions[i]);
    ptr[i] = def;
  }
  _make_entry(&def, handle_unknown_exception);
  for (i = 0x14; i < 0x100; i++) {
    ptr[i] = def;
  }
}

static void _make_entry(idt_entry * out, void (* ptr)()) {
  uint64_t excAddress = (uint64_t)ptr;
  out->low_offset = excAddress & 0xffff;
  out->mid_offset = (excAddress >> 16) & 0xffff;
  out->high_offset = excAddress >> 32;

  out->code_segment = 0x8;
  out->reserved1 = 0;
  out->reserved2 = 0;
  out->flags = 0x8f; // make this a trap gate because i'm trapped in this silly OS
}

