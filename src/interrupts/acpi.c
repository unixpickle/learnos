#include "acpi.h"
#include <string.h>
#include <kernpage.h>
#include <stdio.h>

static void * _acpi_find_rsdp();
static uint8_t _acpi_mem_checksum(uint8_t * ptr, int len);
static uint64_t _acpi_get_madt(acpi_rsdp * rsdp);
static uint64_t _acpi_get_madt_v0(acpi_rsdp * rsdp);
static uint64_t _acpi_get_madt_v1(acpi_rsdp * rsdp);
static bool _increment_value(void * ptr1, void * ptr2);
static bool _find_iso(void * ptr, void * entry);

bool acpi_find_madt() {
  // scan for magic identifier
  acpi_rsdp * rsdp = _acpi_find_rsdp();
  if (!rsdp) return false;

  uint64_t madtAddr = _acpi_get_madt(rsdp);
  uint32_t len;
  kernpage_copy_physical(&len, (void *)madtAddr + 4, 4);
  if (len > 0x1000) {
    die("Many bytes, such MADT, wow");
  }
  kernpage_copy_physical((void *)ACPI_MADT_PTR, (void *)madtAddr, len);
  return true;
}

int acpi_count_lapics() {
  uint64_t count;
  acpi_madt_iterate_type(0, &count, _increment_value);
  return (int)count;
}

int acpi_count_ioapics() {
  uint64_t count;
  acpi_madt_iterate_type(1, &count, _increment_value);
  return (int)count;
}

bool acpi_has_pic() {
  acpi_madt * madt = (acpi_madt *)ACPI_MADT_PTR;
  return madt->flags & 1;
}

void acpi_madt_iterate_type(uint8_t type, void * data, madt_iterator_t iter) {
  uint64_t fieldPtr = ACPI_MADT_PTR + 0x2c;
  uint64_t fieldMax = ACPI_MADT_PTR + ((const uint32_t *)ACPI_MADT_PTR)[1];
  while (fieldPtr < fieldMax) {
    uint8_t aType = *((uint8_t *)(fieldPtr));
    uint8_t len = *((uint8_t *)(fieldPtr + 1));
    if (type == aType) {
      if (!iter(data, (void *)fieldPtr)) return;
    }
    fieldPtr += len;
  }
}

acpi_entry_iso * acpi_iso_lookup(uint8_t physicalIRQ) {
  struct {
    acpi_entry_iso * iso;
    uint8_t phys;
  } __attribute__((packed)) isoInfo;
  isoInfo.phys = physicalIRQ;
  isoInfo.iso = NULL;
  acpi_madt_iterate_type(2, &isoInfo, _find_iso);
  return isoInfo.iso;
}

static void * _acpi_find_rsdp() {
  // scan the address space

  const char * signature = "RSD PTR ";
  uint64_t ptr;
  // the whole potential EBDA
  for (ptr = 0x80000; ptr < 0x9fc00; ptr++) {
    if (memequal(signature, (void *)ptr, 8)) {
      if (_acpi_mem_checksum((uint8_t *)ptr, 0x14) != 0) continue;
      return (void *)ptr;
    }
  }
  for (ptr = 0xe0000; ptr < 0x100000; ptr++) {
    if (memequal(signature, (void *)ptr, 8)) {
      if (_acpi_mem_checksum((uint8_t *)ptr, 0x14) != 0) continue;
      return (void *)ptr;
    }
  }
  return NULL;
}

static uint8_t _acpi_mem_checksum(uint8_t * ptr, int len) {
  uint8_t sum = 0;
  while (len-- > 0) {
    sum += (*ptr);
    ptr++;
  }
  return sum;
}

static uint64_t _acpi_get_madt(acpi_rsdp * rsdp) {
  if (rsdp->revision == 0) {
    return _acpi_get_madt_v0(rsdp);
  } else if (rsdp->revision >= 1) {
    return _acpi_get_madt_v1(rsdp);
  } else return 0;
}

static uint64_t _acpi_get_madt_v0(acpi_rsdp * rsdp) {
  acpi_rsdt rsdt;
  kernpage_copy_physical(&rsdt, (const void *)((uint64_t)rsdp->ptrRSDT), sizeof(rsdt));
  int i, ptrCount = (rsdt.length - 0x24) / 4;
  for (i = 0; i < ptrCount; i++) {
    uint64_t pointer = 0;
    uint64_t offset = rsdp->ptrRSDT + (i * 4) + 0x24;
    kernpage_copy_physical(&pointer, (void *)offset, 4);

    // `pointer` points to more physical memory that we have to compare
    char bytes[5] = {0, 0, 0, 0, 0};
    kernpage_copy_physical(bytes, (void *)pointer, 4);
    if (memequal(bytes, "APIC", 4)) return pointer;
  }
  return 0;
}

static uint64_t _acpi_get_madt_v1(acpi_rsdp * rsdp) {
  acpi_xsdt xsdt;
  kernpage_copy_physical(&xsdt, (const void *)rsdp->ptrXSDT, sizeof(xsdt));
  int i, ptrCount = (xsdt.length - 0x24) / 8;
  for (i = 0; i < ptrCount; i++) {
    uint64_t pointer = 0;
    uint64_t offset = rsdp->ptrXSDT + (i * 8) + 0x24;
    kernpage_copy_physical(&pointer, (void *)offset, 8);

    // `pointer` points to more physical memory that we have to compare
    char bytes[5] = {0, 0, 0, 0, 0};
    kernpage_copy_physical(bytes, (void *)pointer, 4);
    if (memequal(bytes, "APIC", 4)) return pointer;
  }
  return 0;
}

static bool _increment_value(void * ptr1, void * ptr2) {
  (*((uint64_t *)ptr1))++;
  return 1;
}

static bool _find_iso(void * ptr, void * entry) {
  acpi_entry_iso ** isoOut = (acpi_entry_iso **)ptr;
  acpi_entry_iso * iso = (acpi_entry_iso *)entry;
  uint8_t * phys = (uint8_t *)ptr + sizeof(acpi_entry_iso *);
  if (iso->source == (*phys) && iso->bus == 0) {
    (*isoOut) = iso;
    return 0;
  }
  return 1;
}

