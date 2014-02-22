#include "acpi.h"
#include <libkern64/string.h>
#include <libkern64/kernpage.h>

static void * _acpi_find_rsdp();
static uint8_t _acpi_mem_checksum(uint8_t * ptr, int len);
static uint64_t _acpi_get_madt(acpi_rsdp * rsdp);
static uint64_t _acpi_get_madt_v0(acpi_rsdp * rsdp);
static uint64_t _acpi_get_madt_v1(acpi_rsdp * rsdp);
static int _acpi_count_type(uint8_t type);

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
  return _acpi_count_type(0);
}

int acpi_count_ioapics() {
  return _acpi_count_type(1);
}

void acpi_get_lapics(acpi_entry_lapic * output) {
  // TODO: nyi
}

static void * _acpi_find_rsdp() {
  // scan the address space

  const char * signature = "RSD PTR ";
  uint64_t ptr, i;
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
  uint8_t sum;
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

static int _acpi_count_type(uint8_t type) {
  uint64_t fieldPtr = ACPI_MADT_PTR + 0x2c;
  uint64_t fieldMax = ACPI_MADT_PTR + ((const uint32_t *)ACPI_MADT_PTR)[1];
  int count = 0;
  while (fieldPtr < fieldMax) {
    uint8_t aType = *((uint8_t *)(fieldPtr));
    uint8_t len = *((uint8_t *)(fieldPtr + 1));
    if (type == aType) count++;
    fieldPtr += len;
  }
  return count;
}

