#include <libkern64/stdio.h>
#include <interrupts/idt.h>
#include <interrupts/ioapic.h>
#include <interrupts/acpi.h>

void apic_initialize() {
  print64("configuring APIC...\n");
  if (!acpi_find_madt()) die("Failed to find MADT");
  lapic_initialize();
  ioapic_initialize();
  configure_global_idt();
  enable_interrupts();
  print64("now accepting interrupts.\n");
  //asm("int $0x40");
}

