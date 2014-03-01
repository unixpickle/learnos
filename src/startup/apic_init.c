#include <stdio.h>
#include <libkern_base.h>
#include <interrupts/pic.h>
#include <interrupts/idt.h>
#include <interrupts/ioapic.h>
#include <interrupts/lapic.h>
#include <interrupts/acpi.h>
#include <interrupts/pit.h>
#include <interrupts/basic.h>

void apic_initialize() {
  if (!acpi_find_madt()) die("Failed to find MADT");

  // suck away all the dummy interrupts
  pic_disable();
  configure_dummy_idt();
  // do a few nops to make sure we catch all interrupts
  asm("sti\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\ncli");

  // initialize the good hardware
  lapic_initialize();
  ioapic_initialize();

  // setup real handlers and have fun!
  configure_global_idt();
  print64("enabling interrupts...\n");
  enable_interrupts();
  print64("now accepting interrupts.\n");

  pit_set_divisor(11932);
}

