#include <libkern64/stdio.h>
#include <interrupts/idt.h>
#include <interrupts/ioapic.h>
#include <interrupts/acpi.h>

void apic_initialize() {
  if (!acpi_find_madt()) die("Failed to find MADT");

  // suck away all the dummy interrupts
  pic_disable();
  configure_dummy_idt();
  asm("sti\nnop\ncli");

  // initialize the good hardware
  lapic_initialize();
  ioapic_initialize();

  // setup real handlers and have fun!
  configure_global_idt();
  print64("enabling interrupts...\n");
  enable_interrupts();
  print64("now accepting interrupts.\n");
}

