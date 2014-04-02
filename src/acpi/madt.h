/**
 * APIC Descriptor Table interface. Useful for detecting CPUs and interrupt
 * redirects which apply to the I/O APIC.
 */

#include "sdt.h"

typedef struct {
  uint8_t type;
  uint8_t length;
  uint8_t bus;
  uint8_t source;
  uint32_t interrupt;
  uint16_t flags;
} __attribute__((packed)) madt_iso_t;

typedef void (* madt_iterator_t)(void * ui, void * entry,
                                 uint8_t type, uint8_t len);
typedef void (* madt_lapic_iterator_t)(void * ui, uint64_t apicId);

/**
 * Must be called before calling any other methods. This will automatically
 * call acpi_sdt_find() for you, so you don't have to do anything but call
 * this.
 */
bool acpi_madt_find();

/**
 * Iterate over entries in the MADT. Each entry will call fn, and `entry` will
 * point to the entry in the MADT (in virtual memory, don't worry).
 */
void acpi_madt_iterate(void * ui, madt_iterator_t iterator);

/**
 * Returns `true` if this system has an old PIC you should disable.
 */
bool acpi_madt_has_8259_pic();

/**
 * Returns the number of I/O APICs on this system. Anything >0 is good, 1 is
 * ideal.
 */
uint64_t acpi_madt_count_ioapics();

/**
 * Returns the number of LAPICs available on this system. Anything >0 is
 * perfect!
 */
uint64_t acpi_madt_count_lapics();

/**
 * Iterates over the local APICs.
 */
void acpi_madt_get_lapics(void * ui, madt_lapic_iterator_t iter);

/**
 * Looks up an interrupt redirection in the table.
 * @return true if one is found
 */
bool acpi_madt_iso_lookup(uint8_t physicalIRQ, madt_iso_t * iso);
