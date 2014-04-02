/**
 * Intel's Advanced Configuration and Power Interface
 */

#include <shared/addresses.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint64_t signature;
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t ptrRSDT;
  uint32_t length;
  uint64_t ptrXSDT;
  uint8_t xChecksum;
  char reserved[3];
} __attribute__((packed)) acpi_rsdp;

typedef struct {
  uint32_t signature;
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  uint64_t oemTableId;
  uint32_t oemRev;
  uint32_t creatorId;
  uint32_t creatorRev;
} __attribute__((packed)) acpi_rsdt;

typedef struct {
  uint32_t signature;
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  uint64_t oemTableId;
  uint32_t oemRevision;
  uint32_t creatorId;
  uint32_t creatorRev;
} __attribute__((packed)) acpi_xsdt;

typedef struct {
  uint32_t signature;
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  uint64_t oemTableId;
  uint32_t oemRevision;
  uint32_t creatorId;
  uint32_t creatorRev;
  uint32_t lapicAddr;
  uint32_t flags;
} __attribute__((packed)) acpi_madt;

typedef struct {
  uint8_t type; // should be zero
  uint8_t length;
  uint8_t apicpuId;
  uint8_t apicId;
  uint32_t flags; // bit 0 set = usable
} __attribute__((packed)) acpi_entry_lapic;

typedef struct {
  uint8_t type;
  uint8_t length;
  uint8_t apicId;
  uint8_t reserved;
  uint32_t address;
  uint32_t gsib; // global system interrupt base
} __attribute__((packed)) acpi_entry_ioapic;

typedef struct {
  uint8_t type;
  uint8_t length;
  uint8_t bus;
  uint8_t source;
  uint32_t interrupt;
  uint16_t flags;
} __attribute__((packed)) acpi_entry_iso;

typedef struct {
  uint8_t type; // should be 9
  uint8_t length;
  uint16_t reserved;
  uint32_t x2apicId;
  uint32_t flags;
  uint32_t x2apicpuId;
} __attribute__((packed)) acpi_entry_x2apic;

typedef bool (* madt_iterator_t)(void * data, void * entry);

bool acpi_find_madt();
int acpi_count_lapics();
int acpi_count_ioapics();
bool acpi_has_pic();
void acpi_madt_iterate_type(uint8_t type, void * data, madt_iterator_t iter);
acpi_entry_iso * acpi_iso_lookup(uint8_t physicalIRQ);

