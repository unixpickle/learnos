#include "malloc.h"
#include <analloc.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <anlock.h>
#include <assert.h>
#include <base/alloc.h>

static uint64_t allocatorCount = 0;
static uint64_t allocLock __attribute__((aligned(8))) = 0;

#define ALLOCATOR_OFF(x) (x == 0 ? 0 : (0x100000 << (x - 1)))
#define ALLOCATOR_BASE(x) (ALLOC_DATA_BASE + ALLOCATOR_OFF(x))
#define ALLOCATOR_PREFIX(x) ((alloc_prefix_t *)ALLOCATOR_BASE(x))

typedef struct {
  uint64_t bytesUsed;
  analloc_struct_t alloc;
} __attribute__((packed)) alloc_prefix_t;

static uint64_t _allocator_for_buffer(void * buf);
static void * _raw_alloc(size_t size);
static bool _create_allocator();
static bool _release_allocator();

void free(void * buf) {
  anlock_lock(&allocLock);
  uint64_t allocator = _allocator_for_buffer(buf);

  // this should NEVER be the case
  assert(allocator < allocatorCount);

  alloc_prefix_t * prefix = ALLOCATOR_PREFIX(allocator);
  uint64_t size;
  void * ptr = analloc_mem_start(&prefix->alloc, buf, &size);
  assert(ptr != NULL);
  prefix->bytesUsed -= size;

  analloc_free(&prefix->alloc, ptr, size);
  while (_release_allocator());

  anlock_unlock(&allocLock);
}

void * malloc(size_t size) {
  void * buf;
  anlock_lock(&allocLock);
  while (!(buf = _raw_alloc(size))) {
    // brk here and generate the next allocator
    if (!_create_allocator()) {
      anlock_unlock(&allocLock);
      return NULL;
    }
  }
  anlock_unlock(&allocLock);
  return buf;
}

int posix_memalign(void ** ptr, size_t align, size_t size) {
  // the data section is only aligned on a 4GB boundary, so we cannot align
  // things bigger than that without wasting a PHENOMENAL amount of memory.
  if (size > 0xffffffff) return -1;

  uint64_t nextPower = 0;
  for (nextPower = 0; (1 << nextPower) < align; nextPower++);

  // if the alignment is a power of 2, this allocation is easy!
  uint64_t grabSize = size < align ? align : size;
  if ((1 << nextPower) == align) {
    (*ptr) = malloc(grabSize);
    return 0;
  }

  void * buff = malloc(grabSize + align);
  uint64_t remainder = ((uint64_t)buff) % align;
  (*ptr) = buff + (align - remainder);
  return 0;
}

void * realloc(void * buf, size_t size) {
  anlock_lock(&allocLock);
  uint64_t allocator = _allocator_for_buffer(buf);
  assert(allocator < allocatorCount);

  alloc_prefix_t * prefix = ALLOCATOR_PREFIX(allocator);

  // you may not realloc() something from posix_memalign()
  size_t oldSize = 0;
  void * realMem = analloc_mem_start(&prefix->alloc, buf, &oldSize);
  if (realMem != buf) {
    anlock_unlock(&allocLock);
    return NULL;
  }

  // if we can use realloc(), we will do it
  size_t newSize = size;
  void * newMem = analloc_realloc(&prefix->alloc, buf, oldSize, &newSize, 0);
  anlock_unlock(&allocLock);
  if (newMem) return newMem;

  // allocate a whole new buffer and copy in *our* buffer
  void * buffer = malloc(size);
  if (!buffer) return NULL;

  memcpy(buffer, buf, oldSize > size ? size : oldSize);
  free(buf);
  return buffer;
}

static uint64_t _allocator_for_buffer(void * buf) {
  uint64_t byteOffset = ((uint64_t)(buf - ALLOC_DATA_BASE));
  uint64_t allocator;
  for (allocator = 0; allocator < allocatorCount; allocator++) {
    if ((0x100000 << allocator) > byteOffset) {
      break;
    }
  }
  return allocator;
}

static void * _raw_alloc(size_t size) {
  uint64_t i;
  for (i = 0; i < allocatorCount; i++) {
    alloc_prefix_t * prefix = ALLOCATOR_PREFIX(i);
    size_t sizeOut = size;
    void * buff = analloc_alloc(&prefix->alloc, &sizeOut, 0);
    if (buff) {
      prefix->bytesUsed += sizeOut;
      return buff;
    }
  }
  return NULL;
}

static bool _create_allocator() {
  void * startPtr = ALLOCATOR_BASE(allocatorCount);
  void * endPtr = ALLOCATOR_BASE(allocatorCount + 1);
  if (brk(endPtr)) return false;

  alloc_prefix_t * prefix = (alloc_prefix_t *)startPtr;
  prefix->bytesUsed = 0;
  uint64_t total = (uint64_t)(endPtr - ((uint64_t)startPtr));
  uint64_t used = sizeof(alloc_prefix_t);
  uint8_t res = analloc_with_chunk(&prefix->alloc, startPtr, total, used, 0x20);
  if (!res) return false;
  allocatorCount++;

  return true;
}

static bool _release_allocator() {
  if (!allocatorCount) return false;
  alloc_prefix_t * prefix = ALLOCATOR_PREFIX(allocatorCount - 1);
  if (prefix->bytesUsed) return false;
  if (brk((void *)prefix)) return false;
  allocatorCount--;
  return true;
}

