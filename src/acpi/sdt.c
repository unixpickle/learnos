#include "sdt.h"

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

static acpi_rsdp * rsdp __attribute__((aligned(8))) = NULL;
static uint8_t _acpi_mem_checksum(uint8_t * ptr, int len);

bool acpi_sdt_find() {
  if (rsdp) return true;
  
  // find the RSDP in the BIOS areas.
  const char * signature = "RSD PTR ";
  uint64_t ptr;
  // the whole potential EBDA
  for (ptr = 0x80000; ptr < 0x9fc00; ptr++) {
    if (memcmp(signature, (void *)ptr, 8) == 0) {
      if (_acpi_mem_checksum((uint8_t *)ptr, 0x14) != 0) continue;
      rsdp = (void *)ptr;
      return true;
    }
  }
  for (ptr = 0xe0000; ptr < 0x100000; ptr++) {
    if (memcmp(signature, (void *)ptr, 8) == 0) {
      if (_acpi_mem_checksum((uint8_t *)ptr, 0x14) != 0) continue;
      rsdp = (void *)ptr;
      return true;
    }
  }
  
  return false;
}

uint64_t acpi_sdt_count_tables() {
  if (rsdp->revision == 0) {
    // use RSDT
    acpi_rsdt rsdt;
    uint64_t offset = (uint64_t)rsdp->ptrRSDT;
    kernpage_copy_physical(&rsdt, (void *)offset, sizeof(rsdt));
    return (rsdt.length - 0x24) >> 2;
  } else {
    // use XSDT
    acpi_xsdt xsdt;
    kernpage_copy_physical(&xsdt, (void *)rsdp->ptrXSDT, sizeof(xsdt));
    return (xsdt.length - 0x24) >> 3;
  }
}

void * acpi_sdt_get_table(uint64_t idx) {
  if (rsdp->revision == 0) {
    // use RSDT
    uint64_t off = 0x24 + (idx << 2);
    uint64_t res = 0; // must initialize to 0!
    uint64_t offset = (uint64_t)rsdp->ptrRSDT + off;
    kernpage_copy_physical(&res, (void *)offset, 4);
    return (void *)res;
  } else {
    // use XSDT
    uint64_t off = 0x24 + (idx << 3);
    uint64_t res;
    kernpage_copy_physical(&res, (void *)(rsdp->ptrXSDT + off), 8);
    return (void *)res;
  }
}

static uint8_t _acpi_mem_checksum(uint8_t * ptr, int len) {
  uint8_t sum = 0;
  while (len-- > 0) {
    sum += (*ptr);
    ptr++;
  }
  return sum;
}
