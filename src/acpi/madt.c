#include "madt.h"

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
  uint8_t type; // should be 9
  uint8_t length;
  uint16_t reserved;
  uint32_t x2apicId;
  uint32_t flags; // bit 0 set = usable
  uint32_t x2apicpuId;
} __attribute__((packed)) acpi_entry_x2apic;

typedef struct {
  void * ui;
  madt_lapic_iterator_t iter;
} lapic_iterator_ui_t;

typedef struct {
  madt_iso_t * iso;
  uint64_t physIRQ;
  bool found;
} iso_iterator_ui_t;

static void * madt __attribute__((aligned(8))) = NULL;
static acpi_madt madtHeader;

static void _ioapic_iterator(uint64_t * count,
                             void * entry,
                             uint8_t type,
                             uint8_t len);
static void _lapic_count_iterator(uint64_t * count,
                                  void * entry,
                                  uint8_t type,
                                  uint8_t len);
static void _lapic_iterator(lapic_iterator_ui_t * iter,
                            void * entry,
                            uint8_t type,
                            uint8_t len);
static void _iso_iterator(iso_iterator_ui_t * iter,
                          void * entry,
                          uint8_t type,
                          uint8_t len);

bool acpi_madt_find() {
  if (madt) return true;
  if (!acpi_sdt_find()) return false;
  
  uint64_t i, count = acpi_sdt_count_tables();
  for (i = 0; i < count; i++) {
    void * phys = acpi_sdt_get_table(i);
    kernpage_copy_physical(&madtHeader, phys, 4);
    if (!memcmp(&madtHeader.signature, "APIC", 4)) {
      madt = phys;
      kernpage_copy_physical(&madtHeader, phys, sizeof(madtHeader));
      return true;
    }
  }
  
  return false;
}

void acpi_madt_iterate(void * ui, madt_iterator_t iter) {
  uint64_t i = (uint64_t)madt + sizeof(madtHeader);
  while (i + 2 < (uint64_t)madt + madtHeader.length) {
    uint16_t typeAndLen;
    kernpage_copy_physical(&typeAndLen, (void *)i, 2);
    iter(ui, (void *)i, typeAndLen & 0xff, typeAndLen >> 8);
    i += (uint64_t)(typeAndLen >> 8);
  }
}

bool acpi_madt_has_8259_pic() {
  return (madtHeader.flags & 1);
}

uint64_t acpi_madt_count_ioapics() {
  uint64_t count = 0;
  acpi_madt_iterate(&count, (madt_iterator_t)_ioapic_iterator);
  return count;
}

uint64_t acpi_madt_count_lapics() {
  uint64_t count = 0;
  acpi_madt_iterate(&count, (madt_iterator_t)_lapic_count_iterator);
  return count;
}

void acpi_madt_get_lapics(void * ui, madt_lapic_iterator_t fn) {
  lapic_iterator_ui_t iter;
  iter.iter = fn;
  iter.ui = ui;
  acpi_madt_iterate(&iter, (madt_iterator_t)_lapic_iterator);
}

bool acpi_madt_iso_lookup(uint8_t physicalIRQ, madt_iso_t * iso) {
  iso_iterator_ui_t ui;
  ui.physIRQ = physicalIRQ;
  ui.iso = iso;
  ui.found = false;
  acpi_madt_iterate(&ui, (madt_iterator_t)_iso_iterator);
  return ui.found;
}

static void _ioapic_iterator(uint64_t * count,
                             void * entry,
                             uint8_t type,
                             uint8_t len) {
  if (type != 1) return;
  (*count)++;
}

static void _lapic_count_iterator(uint64_t * count,
                                  void * entry,
                                  uint8_t type,
                                  uint8_t len) {
  if (type != 0 && type != 9) return;
  (*count)++;
}

static void _lapic_iterator(lapic_iterator_ui_t * iter,
                            void * entry,
                            uint8_t type,
                            uint8_t len) {
  if (type == 0 && len >= sizeof(acpi_entry_lapic)) {
    // xAPIC
    acpi_entry_lapic lapic;
    kernpage_copy_physical(&lapic, entry, sizeof(lapic));
    if (lapic.flags & 1) {
      iter->iter(iter->ui, lapic.apicId);
    }
  } else if (type == 9 && len >= sizeof(acpi_entry_x2apic)) {
    // x2APIC
    acpi_entry_x2apic lapic;
    kernpage_copy_physical(&lapic, entry, sizeof(lapic));
    if (lapic.flags & 1) {
      iter->iter(iter->ui, lapic.x2apicId);
    }
  }
}

static void _iso_iterator(iso_iterator_ui_t * iter,
                          void * entry,
                          uint8_t type,
                          uint8_t len) {
  if (type != 2 || len < sizeof(madt_iso_t)) return;
  if (iter->found) return;
  
  kernpage_copy_physical(iter->iso, entry, sizeof(madt_iso_t));
  if (iter->iso->source == iter->physIRQ && iter->iso->bus == 0) {
    iter->found = true;
    return;
  }
}
