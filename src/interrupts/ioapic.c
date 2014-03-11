#include "ioapic.h"
#include "acpi.h"
#include <kernpage.h>
#include <stdio.h>
#include <libkern_base.h>
#include <shared/addresses.h>

#define IOAPIC_BASE 0xfec00000L

static void _ioapic_configure_irqs();
static void _ioapic_configure_pci();
static void _ioapic_disable_others();
static void _ioapic_start_receiving();

void ioapic_initialize() {
  if (!acpi_count_ioapics()) {
    die("No I/O APICs exist");
  }

  print("mapping I/O APIC page... ");
  uint64_t page = (uint64_t)(IOAPIC_BASE >> 12);
  uint64_t virtualPage = kernpage_last_virtual() + 1;
  if (!kernpage_map(virtualPage, page)) die("failed to map");
  IOAPIC_PTR = (void *)(virtualPage << 12);
  print("mapped to 0x");
  printHex((uint64_t)IOAPIC_PTR);
  print(" [OK]\n");

  _ioapic_configure_irqs();
  _ioapic_configure_pci();
  _ioapic_disable_others();
  _ioapic_start_receiving();

  print("I/O APIC Version ");
  printHex(ioapic_get_version());
  print(", pins available ");
  printHex(ioapic_get_pin_count());
  print("\n");
}

void ioapic_write_reg(uint8_t reg, uint32_t val) {
  volatile uint32_t * regs = (volatile uint32_t *)IOAPIC_PTR;
  regs[0] = (uint32_t)reg;
  regs[4] = val;
}

uint32_t ioapic_read_reg(uint8_t reg) {
  volatile uint32_t * regs = (volatile uint32_t *)IOAPIC_PTR;
  regs[0] = (uint32_t)reg;
  return regs[4];
}

uint32_t ioapic_get_version() {
  return ioapic_read_reg(IOAPIC_REG_VER) & 0xff;
}

uint32_t ioapic_get_pin_count() {
  return ((ioapic_read_reg(IOAPIC_REG_VER) >> 16) & 0xff) + 1;
}

void ioapic_set_red_table(uint8_t index, ioapic_redirection entry) {
  const uint32_t * valPtr = (const uint32_t *)(&entry);
  ioapic_write_reg(0x10 + (index * 2), 0x10000); // disable the entry
  ioapic_write_reg(0x11 + (index * 2), valPtr[1]);
  ioapic_write_reg(0x10 + (index * 2), valPtr[0]);
}

static void _ioapic_configure_irqs() {
  // this idea was from http://www.osdever.net/tutorials/pdf/apic.pdf
  uint8_t vectors[] = IOAPIC_IRQ_VECTORS;

  ioapic_redirection entry;
  entry.delmode = 0; // fixed
  entry.destmode = 0; // single CPU
  entry.intpol = 0; // high active
  entry.triggermode = 0; // edge trigger
  entry.imask = 0;
  entry.reserved = 0;
  entry.destfield = 0; // TODO: figure out how to address more proc's
  uint8_t i;

  for (i = 0; i < 0x10; i++) {
    if (!vectors[i]) continue;
    acpi_entry_iso * iso = acpi_iso_lookup(i);
    entry.vector = vectors[i];
    if (iso) {
      // Interrupt Source Override allows us to get more info about this IRQ
      if ((iso->flags & 0x3) == 0x3) entry.intpol = 1;
      if (((iso->flags >> 2) & 0x3) == 3) entry.triggermode = 1;
      ioapic_set_red_table(iso->interrupt, entry);
      entry.intpol = 0;
      entry.triggermode = 0;
    } else {
      ioapic_set_red_table(i, entry);
    }
  }
}

static void _ioapic_configure_pci() {
  // configure the 4 PCI interrupts according to http://www.osdever.net/tutorials/pdf/apic.pdf
  ioapic_redirection entry;
  entry.delmode = 0; // fixed
  entry.destmode = 0; // single APIC
  entry.intpol = 1; // low active
  entry.triggermode = 1; // level trigger
  entry.imask = 1; // PCI won't be supported for now
  entry.reserved = 0;
  entry.destfield = 0; // TODO: figure out how to address more proc's
  uint8_t i;

  // TODO: properly address these vectors so that the priorities are good
  for (i = 0x10; i < 0x14; i++) {
    entry.vector = i + 0x40;
    ioapic_set_red_table(i, entry);
  }
}

static void _ioapic_disable_others() {
  ioapic_redirection entry;
  entry.delmode = 0;
  entry.destmode = 0;
  entry.intpol = 0;
  entry.triggermode = 0;
  entry.imask = 1;
  entry.reserved = 0;
  entry.destfield = 0;
  entry.vector = 0x20; // making it something valid is probably important
  uint8_t i;
  for (i = 0x14; i < 0x18; i++) {
    ioapic_set_red_table(i, entry);
  }
}

static void _ioapic_start_receiving() {
  // apparently this is a motherboard thing
  outb(0x22, 0x70);
  outb(0x23, 0x01);
}

