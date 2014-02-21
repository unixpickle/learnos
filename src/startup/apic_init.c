#include <libkern64/stdio.h>
#include <interrupts/idt.h>
#include <interrupts/ioapic.h>

void apic_initialize() {
  print64("configuring APIC...\n");
  configure_global_idt();
  ioapic_initialize();
  enable_interrupts();
  print64("now accepting interrupts.\n");
}

