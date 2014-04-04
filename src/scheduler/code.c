#include "code.h"
#include <anscheduler/functions.h>
#include <anscheduler/task.h>

static void _free_code(code_t * code);
static page_t _alloc_code_page(code_t * code, page_t page);

static page_t _lookup_page(code_t * code, page_t codePage);
static bool _map_page(code_t * code, page_t codePage, page_t mapping);

code_t * code_allocate(void * base, uint64_t len) {
  code_t * code = anscheduler_alloc(0x1000);
  if (!code) return NULL;
  anscheduler_zero(code, 0x1000);
  code->retainCount = 1;
  code->kernpageBase = base;
  code->kernpageLen = len;
  return code;
}

code_t * code_retain(code_t * code) {
  anscheduler_inc(&code->retainCount);
  return code;
}

void code_release(code_t * code) {
  if (!__sync_sub_and_fetch(&code->retainCount, 1)) {
    _free_code(code);
  }
}

bool code_handle_page_fault(code_t * code,
                            thread_t * thread,
                            void * ptr,
                            uint64_t flags) {
  uint64_t offset = (uint64_t)ptr - (uint64_t)code->kernpageBase;
  if (offset >= code->kernpageLen) return false;

  page_t taskPage = ((page_t)ptr) >> 12;
  page_t codePage = taskPage - ANSCHEDULER_TASK_CODE_PAGE;

  page_t phyPage = 0;
  uint16_t phyFlags = 0;

  if (flags & 8) {
    // allocate the page for reading and writing
    page_t pageCopy = _alloc_code_page(code, codePage);
    if (!pageCopy) return false;
    phyPage = anscheduler_vm_physical(pageCopy);
    phyFlags = 7; // user, write, present
  } else {
    page_t vPage = _lookup_page(code, codePage);
    if (!vPage) {
      vPage = _alloc_code_page(code, codePage);
      if (!vPage) return false;
      if (!_map_page(code, codePage, vPage)) return false;
    }
    phyPage = anscheduler_vm_physical(vPage);
    phyFlags = 5; // user, read-only, present
  }

  // map the entry
  anscheduler_lock(&thread->task->vmLock);
  anscheduler_vm_map(thread->task->vm, taskPage, phyPage, phyFlags);
  anscheduler_unlock(&thread->task->vmLock);
  return true;
}

void code_task_cleanup(code_t * code, task_t * task) {
  // free write pages
  page_t codePageCount = code->kernpageLen >> 12;
  if (code->kernpageLen & 0xfff) codePageCount++;
  uint64_t i;
  for (i = 0; i < codePageCount; i++) {
    uint16_t flags;
    page_t entry = anscheduler_vm_lookup(task->vm,
                                         i + ANSCHEDULER_TASK_CODE_PAGE,
                                         &flags);
    if (!(flags & 2)) continue;
    page_t virt = anscheduler_vm_virtual(entry);
    anscheduler_free((void *)(virt << 12));
  }
  code_release(code);
}

static void _free_code(code_t * code) {
  uint64_t i, j;
  for (i = 0; i < 0xffd; i++) {
    if (!code->pageTables[i]) continue;
    for (j = 0; j < 0x1000; j++) {
      void * table = code->pageTables[i][j];
      if (!table) break;
      anscheduler_cpu_lock();
      anscheduler_free(table);
      anscheduler_cpu_unlock();
    }
  }
}

static page_t _alloc_code_page(code_t * code, page_t page) {
  uint8_t * buffer = anscheduler_alloc(0x1000);
  if (!buffer) return 0;

  uint64_t i, len = 0x1000;
  if (((page + 1) << 12) > code->kernpageLen) {
    len = code->kernpageLen - (page << 12);
  }
  for (i = 0; i < len; i++) {
    buffer[i] = *((uint8_t *)code->kernpageBase);
  }
  for (i = len; i < 0x1000; i++) {
    buffer[i] = 0;
  }
  return ((page_t)buffer) >> 12;
}

static page_t _lookup_page(code_t * code, page_t codePage) {
  uint64_t rootIndex = codePage >> 9;
  uint64_t subIndex = codePage & 0x1ff;
  if (rootIndex >= 0xffd) return 0;

  if (!code->pageTables[rootIndex]) return 0;
  return ((uint64_t)code->pageTables[rootIndex][subIndex]) >> 12;
}

static bool _map_page(code_t * code, page_t codePage, page_t mapping) {
  uint64_t rootIndex = codePage >> 9;
  uint64_t subIndex = codePage & 0x1ff;
  if (rootIndex >= 0xffd) return 0;

  if (!code->pageTables[rootIndex]) {
    void ** newTable = anscheduler_alloc(0x1000);
    if (!newTable) return false;
  }
  code->pageTables[rootIndex][subIndex] = (void *)(mapping << 12);
  return true;
}

