#include "task.h"
#include <kernpage.h>

void task_add_thread(page_t task, void * rip) {
  // NYI
}

void task_start(page_t task) {
  // NYI
}

void task_term(page_t task) {
  // NYI
}

page_t task_find(uint64_t pid) {
  // NYI
  return 0;
}

void task_release(page_t task) {
}

void thread_release(page_t thread) {
}

page_t task_page_lookup(page_t taskPage, page_t page) {
  task_t * task = (task_t *)(taskPage << 12);
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

bool task_page_map(page_t taskPage, page_t virt, page_t phys, bool user) {
  task_t * task = (task_t *)(taskPage << 12);
  uint64_t pml4Page = kernpage_calculate_virtual(task->pml4);
  uint64_t * tablePtr = (uint64_t *)(pml4Page << 12);

  uint64_t indexInPT = virt % 0x200;
  uint64_t indexInPDT = (virt >> 9) % 0x200;
  uint64_t indexInPDPT = (virt >> 18) % 0x200;
  uint64_t indexInPML4 = (virt >> 27) % 0x200;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};

  int i;
  uint64_t flags = user ? 7 : 3;

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
      tablePtr[indices[i]] = (newPage << 12) | flags;
      tablePtr = newData;
    } else {
      if (user && !(value & 4)) { // make sure it's for users
        tablePtr[indices[i]] |= 4;
      }
      uint64_t physPage = value >> 12;
      uint64_t virPage = kernpage_calculate_virtual(physPage);
      if (!virPage) return false;
      tablePtr = (uint64_t *)(virPage << 12);
    }
  }

  tablePtr[indices[3]] = (phys << 12) | flags;
  return true;
}

void task_page_unmap(page_t taskPage, page_t virt) {
  task_t * task = (task_t *)(taskPage << 12);
  uint64_t pml4Page = kernpage_calculate_virtual(task->pml4);
  uint64_t * tablePtr = (uint64_t *)(pml4Page << 12);

  uint64_t indexInPT = virt % 0x200;
  uint64_t indexInPDT = (virt >> 9) % 0x200;
  uint64_t indexInPDPT = (virt >> 18) % 0x200;
  uint64_t indexInPML4 = (virt >> 27) % 0x200;
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

void task_pages_changed() {
  // NYI
}

