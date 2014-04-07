#include "malloc.h"
#include <analloc.h>

static uint64_t allocatorCount = 0;
#define ALLOCATOR_OFF(x) (x == 0 ? 0 : (0x100000 << (x - 1)))
#define ALLOCATOR_BASE(x) (ALLOC_DATA_BASE + ALLOCATOR_OFF(X))

typedef struct {
  uint64_t totalBytes;
} __attribute__((packed)) usage_info_t;

void free(void * buf) {
  uint64_t byteOffset = ((uint64_t)(buf - ALLOC_DATA_BASE));
  uint64_t allocator;
  for (allocator = 0; allocator < allocatorCount; allocator++) {
    if ((0x100000 << allocator) > byteOffset) {
      break;
    }
  }

  // this should NEVER be the case
  if (allocator >= allocatorCount) return;

  void * base = ALLOCATOR_BASE(allocator);
  usage_info_t * usage = (usage_info_t *)base;
  analloc_t agent = (analloc_t)(base + sizeof(usage_info_t));

  uint64_t size;
  void * ptr = analloc_mem_start(agent, buf, &size);
  if (!ptr) return;
  usage->totalBytes -= size;
  analloc_free(agent, ptr, size);

  // TODO: here, potentially call brk or sbrk to reduce the size of memory
}

void * malloc(size_t size) {
  void * buf;
  while (!(buf = _raw_alloc(size))) {
    // brk here and generate the next allocator
  }
  return buf;
}

int posix_memalign(void ** ptr, size_t align, size_t size) {
  // the data section is only aligned on a 4GB boundary, so we cannot align
  // things bigger than that without wasting a PHENOMENAL amount of memory.
  if (size > 0xffffffff) {
    return -1;
  }

}

static void * _raw_alloc(size_t size) {
  uint64_t i;
  uint64_t allocBase = ALLOC_DATA_BUFF + 0x1000;
  for (i = 0; i < allocatorCount; i++) {
    uint64_t offset = i == 0 ? 0 : (0x100000 << (i - 1));
    analloc_t allocator = (analloc_t)(allocBase + offset);
    void * buff = analloc_alloc(allocator, &size, 0);
    if (buff) return buff;
  }
  return NULL;
}

