#include <stdio.h>
#include <libkern_base.h>
#include <interrupts/pic.h>
#include <interrupts/idt.h>
#include <interrupts/ioapic.h>
#include <interrupts/lapic.h>
#include <interrupts/pit.h>
#include <interrupts/basic.h>
#include <acpi/madt.h>

void apic_initialize() {
  if (!acpi_madt_find()) die("failed to find MADT");

  // suck away all the dummy interrupts
  pic_disable();
  configure_dummy_idt();
  // do a few nops to make sure we catch all interrupts
  __asm__ __volatile__("sti\nnop\nnop\nnop\nnop\nnop\ncli");

  // initialize the good hardware
  lapic_initialize();
  ioapic_initialize();

  // setup real handlers and have fun!
  configure_global_idt();
  print("enabling interrupts...\n");
  enable_interrupts();
  print("now accepting interrupts.\n");

  pit_set_divisor(11932);
  lapic_calculate_bus_speed();
  print("calculated bus speed at 0x");
  printHex(lapic_get_bus_speed());
  print("0 Hz\n");
}

