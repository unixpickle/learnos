#include "vm.h"
#include <anlock.h>
#include <kernpage.h>

page_t task_vm_lookup(task_t * task, page_t page) {
  uint64_t pml4Page = kernpage_calculate_virtual(task->pml4);
  uint64_t * tablePtr = (uint64_t *)(pml4Page << 12);

  // check if it's set in the page table
  uint64_t indexInPT = page & 0x1ff;
  uint64_t indexInPDT = (page >> 9) & 0x1ff;
  uint64_t indexInPDPT = (page >> 18) & 0x1ff;
  uint64_t indexInPML4 = (page >> 27) & 0x1ff;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};

  int i;
  for (i = 0; i < 4; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 1)) {
      return 0;
    } else if (i == 3) {
      return tablePtr[indices[i]] >> 12;
    }

    uint64_t physPage = value >> 12;
    uint64_t virPage = kernpage_calculate_virtual(physPage);
    tablePtr = (uint64_t *)(virPage << 12);
  }

  // will never be reached
  return 0;
}

bool task_vm_set(task_t * task, page_t virt, uint64_t value) {
  uint64_t pml4Page = kernpage_calculate_virtual(task->pml4);
  uint64_t * tablePtr = (uint64_t *)(pml4Page << 12);

  uint64_t indexInPT = virt & 0x1ff;
  uint64_t indexInPDT = (virt >> 9) & 0x1ff;
  uint64_t indexInPDPT = (virt >> 18) & 0x1ff;
  uint64_t indexInPML4 = (virt >> 27) & 0x1ff;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};

  int i;

  for (i = 0; i < 3; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 1)) {
      // create a subtable
      kernpage_lock();
      uint64_t newVirPage = kernpage_alloc_virtual();
      kernpage_unlock();

      if (!newVirPage) return false;
      uint64_t newPage = kernpage_calculate_physical(newVirPage);
      if (!newPage) return false;

      uint64_t * newData = (uint64_t *)(newVirPage << 12);
      int j;
      for (j = 0; j < 0x200; j++) newData[j] = 0;
      tablePtr[indices[i]] = (newPage << 12) | 3;
      tablePtr = newData;
    } else {
      uint64_t physPage = value >> 12;
      uint64_t virPage = kernpage_calculate_virtual(physPage);
      if (!virPage) return false;
      tablePtr = (uint64_t *)(virPage << 12);
    }
  }

  tablePtr[indices[3]] = value;
  return true;
}

void task_vm_make_user(task_t * task, page_t virt) {
  uint64_t pml4Page = kernpage_calculate_virtual(task->pml4);
  uint64_t * tablePtr = (uint64_t *)(pml4Page << 12);

  uint64_t indexInPT = virt & 0x1ff;
  uint64_t indexInPDT = (virt >> 9) & 0x1ff;
  uint64_t indexInPDPT = (virt >> 18) & 0x1ff;
  uint64_t indexInPML4 = (virt >> 27) & 0x1ff;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};

  int i;
  for (i = 0; i < 3; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 1)) return;
    if (!(value & 4)) tablePtr[indices[i]] |= 4;
    uint64_t physPage = value >> 12;
    uint64_t virPage = kernpage_calculate_virtual(physPage);
    if (!virPage) return;
    tablePtr = (uint64_t *)(virPage << 12);
  }

  tablePtr[indices[3]] |= 4;
}

void task_vm_unmap(task_t * task, page_t virt) {
  uint64_t pml4Page = kernpage_calculate_virtual(task->pml4);
  uint64_t * tablePtr = (uint64_t *)(pml4Page << 12);

  uint64_t indexInPT = virt & 0x1ff;
  uint64_t indexInPDT = (virt >> 9) & 0x1ff;
  uint64_t indexInPDPT = (virt >> 18) % 0x1ff;
  uint64_t indexInPML4 = (virt >> 27) % 0x1ff;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};
  uint64_t * tablePtrs[3] = {tablePtr, NULL, NULL};

  int i;

  for (i = 0; i < 3; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 1)) return;
    uint64_t physPage = value >> 12;
    uint64_t virPage = kernpage_calculate_virtual(physPage);
    if (!virPage) return;
    tablePtr = (uint64_t *)(virPage << 12);
    if (i != 2) tablePtrs[i + 1] = tablePtr;
  }

  tablePtr[indices[3]] = 0;

  // free up all previous tables if they can be cleaned up
  for (i = 3; i > 0; i--) {
    bool isEmpty = true;
    int j;
    for (j = 0; j < 0x200; j++) {
      if (tablePtr[j]) {
        isEmpty = false;
        break;
      }
    }
    if (!isEmpty) break;
    tablePtrs[i - 1][indices[i - 1]] = 0;
    kernpage_lock();
    kernpage_free_virtual(((uint64_t)tablePtr) >> 12);
    kernpage_unlock();
    tablePtr = tablePtrs[i - 1];
  }
}

void * task_vm_get_from_kernpage(task_t * task, void * ptr) {
  page_t pval = ((uint64_t)ptr) >> 12;
  page_t phys = kernpage_calculate_physical(pval);
  anlock_lock(&task->pml4Lock);
  page_t taskPage = task_vm_lookup(task, phys);
  anlock_unlock(&task->pml4Lock);
  return (void *)((taskPage << 12) + (((uint64_t)ptr) & 0xfff));
}

