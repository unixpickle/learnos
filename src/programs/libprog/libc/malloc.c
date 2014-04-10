#include "malloc.h"
#include <anmalloc/anmalloc.h>

void free(void * buf) {
  anmalloc_free(buf);
}

void * malloc(size_t size) {
  return anmalloc_alloc((uint64_t)size);
}

int posix_memalign(void ** ptr, size_t align, size_t size) {
  void * buf = anmalloc_aligned((uint64_t)align, (uint64_t)size);
  if (!buf) return -1;
  *ptr = buf;
  return 0;
}

void * realloc(void * buf, size_t size) {
  return anmalloc_realloc(buf, (uint64_t)size);
}

