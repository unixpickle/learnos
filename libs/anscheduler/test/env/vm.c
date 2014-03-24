#include "vm.h"
#include "alloc.h"
#include "threading.h"
#include <string.h> // bzero

static void _table_free(uint64_t * table, int depth);
static void _table_free_async(uint64_t * table, int depth);

uint64_t anscheduler_vm_physical(uint64_t virt) {
  return virt;
}

uint64_t anscheduler_vm_virtual(uint64_t phys) {
  return phys;
}

void * anscheduler_vm_root_alloc() {
  void * buf = anscheduler_alloc(0x1000);
  bzero(buf, 0x1000);
  return buf;
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
      if (flags & 4) {
        // user flag
        table[idx] |= 4;
      }
      table = (uint64_t *)((((uint64_t)table[idx]) >> 12) << 12);
    } else {
      void * nextTable = anscheduler_alloc(0x1000);
      bzero(nextTable, 0x1000);
      table[idx] = 3 | ((uint64_t)nextTable);
      if (flags & 4) table[idx] |= 4;
      table = (uint64_t *)nextTable;
    }
  }
  
  table[ptIndex] = (dpage << 12) | flags;
  return true;
}

void anscheduler_vm_unmap(void * root, uint64_t vpage) {
  // this is a giant pain to do correctly, so we'll just do it the lousy way
  anscheduler_vm_map(root, vpage, 0, 0);
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
  // recursive table free
  _table_free((uint64_t *)root, 0);
}

void anscheduler_vm_root_free_async(void * root) {
  // recursive table free
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
    anscheduler_cpu_lock();
    anscheduler_free(table);
    return anscheduler_cpu_unlock();
  }

  int i;
  for (i = 0; i < 0x200; i++) {
    if (table[i] & 1) {
      uint64_t * nTable = (uint64_t *)((table[i] >> 12) << 12);
      _table_free_async(nTable, depth + 1);
    }
  }
  anscheduler_cpu_lock();
  anscheduler_free(table);
  anscheduler_cpu_unlock();
}

