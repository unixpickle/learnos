#include "vm.h"
#include <kernpage.h>
#include <anscheduler/functions.h>

static void _table_free(uint64_t * table, int depth);
static void _table_free_async(uint64_t * table, int depth);
static void _backclear(uint64_t * indices, uint64_t ** tables);

uint64_t anscheduler_vm_physical(uint64_t virt) {
  return kernpage_calculate_physical(virt);
}

uint64_t anscheduler_vm_virtual(uint64_t phys) {
  return kernpage_calculate_virtual(phys);
}

void * anscheduler_vm_root_alloc() {
  return anscheduler_alloc(0x1000);
}

bool anscheduler_vm_map(void * root,
                        uint64_t vpage,
                        uint64_t dpage,
                        uint16_t flags) {
  uint64_t ptIndex = (vpage & 0x1ff);
  uint64_t pdtIndex = (vpage >> 9) & 0x1ff;
  uint64_t pdptIndex = (vpage >> 18) & 0x1ff;
  uint64_t pml4Index = (vpage >> 27) & 0x1ff;
  uint64_t indices[3] = {pml4Index, pdptIndex, pdtIndex};
  
  int i;
  uint64_t * table = (uint64_t *)root;
  for (i = 0; i < 3; i++) {
    uint64_t idx = indices[i];
    if (table[idx] & 1) {
      // make sure all needed flags are set
      if (flags & 4) table[idx] |= 4;
      if (flags & 2) table[idx] |= 2;
      table = (uint64_t *)((((uint64_t)table[idx]) >> 12) << 12);
    } else {
      void * nextTable = anscheduler_alloc(0x1000);
      anscheduler_zero(nextTable, 0x1000);
      table[idx] = 1 | ((uint64_t)nextTable);
      if (flags & 4) table[idx] |= 4;
      if (flags & 2) table[idx] |= 2;
      table = (uint64_t *)nextTable;
    }
  }
  
  table[ptIndex] = (dpage << 12) | flags;
  return true;
}

void anscheduler_vm_unmap(void * root, uint64_t vpage) {
  uint64_t * tablePtr = (uint64_t *)root;

  uint64_t indexInPT = vpage & 0x1ff;
  uint64_t indexInPDT = (vpage >> 9) & 0x1ff;
  uint64_t indexInPDPT = (vpage >> 18) % 0x1ff;
  uint64_t indexInPML4 = (vpage >> 27) % 0x1ff;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};
  uint64_t * tablePtrs[4] = {tablePtr, NULL, NULL, NULL};
  int i;

  for (i = 0; i < 3; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 1)) return;
    uint64_t physPage = value >> 12;
    uint64_t virPage = kernpage_calculate_virtual(physPage);
    if (!virPage) return;
    tablePtr = (uint64_t *)(virPage << 12);
    tablePtrs[i + 1] = tablePtr;
  }

  tablePtr[indices[3]] = 0;

  _backclear(indices, tablePtrs);
}

uint64_t anscheduler_vm_lookup(void * root,
                               uint64_t vpage,
                               uint16_t * flags) {
  uint64_t ptIndex = (vpage & 0x1ff);
  uint64_t pdtIndex = (vpage >> 9) & 0x1ff;
  uint64_t pdptIndex = (vpage >> 18) & 0x1ff;
  uint64_t pml4Index = (vpage >> 27) & 0x1ff;
  uint64_t indices[4] = {pml4Index, pdptIndex, pdtIndex, ptIndex};
  uint64_t * table = (uint64_t *)root;
  int i;
  for (i = 0; i < 3; i++) {
    uint64_t idx = indices[i];
    if (table[idx] & 1) {
      table = (uint64_t *)((table[idx] >> 12) << 12);
    } else {
      (*flags) = 0;
      return 0;
    }
  }
  (*flags) = (uint16_t)(table[indices[3]] & 0xfff);
  return table[indices[3]] >> 12;
}

void anscheduler_vm_root_free(void * root) {
  _table_free((uint64_t *)root, 0);
}

void anscheduler_vm_root_free_async(void * root) {
  _table_free_async((uint64_t *)root, 0);
}

static void _table_free(uint64_t * table, int depth) {
  if (depth == 3) {
    return anscheduler_free(table);
  }
  int i;
  for (i = 0; i < 0x200; i++) {
    if (table[i] & 1) {
      uint64_t * nTable = (uint64_t *)((table[i] >> 12) << 12);
      _table_free(nTable, depth + 1);
    }
  }
  anscheduler_free(table);
}

static void _table_free_async(uint64_t * table, int depth) {
  if (depth == 3) {
    return anscheduler_free(table);
  }
  int i;
  for (i = 0; i < 0x200; i++) {
    if (table[i] & 1) {
      uint64_t * nTable = (uint64_t *)((table[i] >> 12) << 12);
      _table_free_async(nTable, depth + 1);
    }
  }
  anscheduler_free(table);
}

static void _backclear(uint64_t * indices, uint64_t ** tables) {
  uint64_t * table = tables[3];
  int i, j;
  for (i = 3; i > 0; i--) {
    uint64_t collectiveEntries = 0;
    for (j = 0; j < 0x200; j++) {
      collectiveEntries |= table[j];
    }

    table = tables[i - 1];
    if (!collectiveEntries) {
      // free this entire table
      anscheduler_free(tables[i]);
      table[indices[i - 1]] = 0;
      continue;
    }

    // unset flags if they are no longer needed
    if (!(collectiveEntries & 4)) {
      if (table[indices[i - 1]] & 4) {
        table[indices[i - 1]] ^= 4;
      }
    }
    if (!(collectiveEntries & 2)) {
      if (table[indices[i - 1]] & 2) {
        table[indices[i - 1]] ^= 2;
      }
    }
  }
}

