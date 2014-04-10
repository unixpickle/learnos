#include <anmalloc/anmalloc.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <analloc.h>

typedef struct {
  uint64_t used;
  analloc_struct_t alloc;
} __attribute__((packed)) prefix_t;

static anmalloc_lock_t lock = ANMALLOC_LOCK_INITIALIZER;
static bool isInitialized = false;
static uint64_t allocatorCount = 0;
static void * baseBreak = NULL;

static void _ensure_initialized();
static uint64_t _allocator_offset(uint64_t alloc);
static void * _allocator_base(uint64_t alloc);
static uint64_t _allocator_lookup(void * buf);
#define _allocator_prefix(x) ((prefix_t *)_allocator_base(x))

static void * _raw_alloc(uint64_t size);
static bool _create_allocator();
static bool _release_allocator();

void anmalloc_free(void * buf) {
  anmalloc_lock(&lock);

  uint64_t allocator = _allocator_lookup(buf);
  assert(allocator < allocatorCount);

  prefix_t * prefix = _allocator_prefix(allocator);
  uint64_t size;
  void * ptr = analloc_mem_start(&prefix->alloc, buf, &size);
  assert(ptr != NULL);
  prefix->used -= size;

  analloc_free(&prefix->alloc, ptr, size);
  while (_release_allocator());

  anmalloc_unlock(&lock);
}

void * anmalloc_alloc(uint64_t size) {
  anmalloc_lock(&lock);
  _ensure_initialized();

  void * buf;
  while (!(buf = _raw_alloc(size))) {
    if (!_create_allocator()) {
      while (_release_allocator());
      anmalloc_unlock(&lock);
      return NULL;
    }
  }

  anmalloc_unlock(&lock);
  return buf;
}

void * anmalloc_aligned(uint64_t align, uint64_t size) {
  anmalloc_lock(&lock);
  _ensure_initialized();

  // do the alignment
  uint64_t nextPower;
  for (nextPower = 0; (1 << nextPower) < align; nextPower++);
  if (((uint64_t)baseBreak) & ((1 << nextPower) - 1)) {
    return NULL;
  }
  anmalloc_unlock(&lock);
  
  if ((1 << nextPower) == align) {
    // the normal malloc will be properly aligned
    return anmalloc_alloc(align > size ? align : size);
  }

  void * result = anmalloc_alloc((align > size ? align : size) + align);
  if (!result) return NULL;
  uint64_t remainder = ((uint64_t)result) % align;
  result += align - remainder;
  anmalloc_unlock(&lock);
  return result;
}

void * anmalloc_realloc(void * ptr, uint64_t size) {
  anmalloc_lock(&lock);
  _ensure_initialized();

  uint64_t allocator = _allocator_lookup(ptr);
  assert(allocator < allocatorCount);
  prefix_t * prefix = _allocator_prefix(allocator);

  uint64_t oldSize;
  void * mem = analloc_mem_start(&prefix->alloc, ptr, &oldSize);
  if (mem != ptr) {
    anmalloc_unlock(&lock);
    return NULL;
  }

  uint64_t newSize = size;
  void * newMem = analloc_realloc(&prefix->alloc, ptr, oldSize, &newSize, 0);
  anmalloc_unlock(&lock);
  if (newMem) return newMem;

  void * buffer = anmalloc_alloc(size);
  if (!buffer) return NULL;
  memcpy(buffer, ptr, oldSize > size ? size : oldSize);
  anmalloc_free(ptr);
  return buffer;
}

static void _ensure_initialized() {
  if (isInitialized) return;
  isInitialized = true;
  baseBreak = anmalloc_sbrk(0);
}

static uint64_t _allocator_offset(uint64_t alloc) {
  if (!alloc) return 0;
  return 0x100000 << (alloc - 1);
}

static void * _allocator_base(uint64_t alloc) {
  _ensure_initialized();
  return baseBreak + _allocator_offset(alloc);
}

static uint64_t _allocator_lookup(void * buf) {
  uint64_t offset = ((uint64_t)(buf - baseBreak));
  uint64_t allocator;
  for (allocator = 0; allocator < allocatorCount; allocator++) {
    if ((0x100000 << allocator) > offset) break;
  }
  return allocator;
}

static void * _raw_alloc(uint64_t size) {
  uint64_t i;
  for (i = 0; i < allocatorCount; i++) {
    prefix_t * pref = _allocator_prefix(i);
    uint64_t sizeOut = size;
    void * buff = analloc_alloc(&pref->alloc, &sizeOut, 0);
    if (buff) {
      pref->used += sizeOut;
      return buff;
    }
  }
  return NULL;
}

static bool _create_allocator() {
  void * startPtr = _allocator_base(allocatorCount);
  void * endPtr = _allocator_base(allocatorCount + 1);
  if (anmalloc_brk(endPtr)) return false;

  prefix_t * prefix = (prefix_t *)startPtr;
  prefix->used = 0;
  uint64_t total = (uint64_t)(endPtr - startPtr);
  uint64_t used = sizeof(prefix_t);
  uint8_t res = analloc_with_chunk(&prefix->alloc, startPtr, total, used, 0x20);
  if (!res) return false;
  allocatorCount++;

  return true;
}

static bool _release_allocator() {
  if (!allocatorCount) return false;
  prefix_t * prefix = _allocator_prefix(allocatorCount - 1);
  if (prefix->used) return false;
  if (anmalloc_brk((void *)prefix)) return false;
  allocatorCount--;
  return true;
}

