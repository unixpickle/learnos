#include <anmem/alloc.h>
#include <anlock.h>
#include <analloc.h>
#include <anpages.h>

void * anmem_alloc_aligned(anmem_t * mem, uint64_t len) {
  if (len == 1) return anmem_alloc_page(mem);
  
  uint64_t k, i;
  uint64_t size = len << 12;
  for (k = 0; k < mem->count; k++) {
    i = mem->count - k - 1;
    if (mem->allocators[i].type != 1) continue;
    
    anlock_lock(&mem->allocators[i].lock);
    analloc_t alloc = &mem->allocators[i].anallocRoot;
    void * buff = analloc_alloc(alloc, &size, 0);
    anlock_unlock(&mem->allocators[i].lock);
    if (buff) return buff;
  }
  return NULL;
}

void anmem_free_aligned(anmem_t * mem, void * buffer, uint64_t len) {
  if (len == 1) return anmem_free_page(mem, buffer);
  
  uint64_t page = ((uint64_t)buffer) >> 12;
  // figure out which allocator it was from
  uint64_t k, i;
  uint64_t size = len << 12;
  for (k = 0; k < mem->count; k++) {
    i = mem->count - k - 1;
    if (mem->allocators[i].type != 1) continue;
    if (mem->allocators[i].start > page) continue;
    if (mem->allocators[i].start + mem->allocators[i].len <= page) {
      continue;
    }
    
    anlock_lock(&mem->allocators[i].lock);
    analloc_t alloc = &mem->allocators[i].anallocRoot;
    analloc_free(alloc, buffer, len);
    anlock_unlock(&mem->allocators[i].lock);
    return;
  }
}

void * anmem_alloc_page(anmem_t * mem) {
  uint64_t k, i;
  for (k = 0; k < mem->count; k++) {
    i = mem->count - k - 1;
    if (mem->allocators[i].type == 0) {
      anlock_lock(&mem->allocators[i].lock);
      uint64_t page = anpages_alloc(&mem->allocators[i].anpagesRoot);
      anlock_unlock(&mem->allocators[i].lock);
      if (page) return (void *)(page << 12);
    } else {
      anlock_lock(&mem->allocators[i].lock);
      analloc_t alloc = &mem->allocators[i].anallocRoot;
      uint64_t size = 0x1000;
      void * buff = analloc_alloc(alloc, &size, 0);
      anlock_unlock(&mem->allocators[i].lock);
      if (buff) return buff;
    }
  }
  return NULL;
}

void anmem_free_page(anmem_t * mem, void * buffer) {
  uint64_t page = ((uint64_t)buffer) >> 12;
  // figure out which allocator it was from
  uint64_t k, i;
  for (k = 0; k < mem->count; k++) {
    i = mem->count - k - 1;
    if (mem->allocators[i].start > page) continue;
    if (mem->allocators[i].start + mem->allocators[i].len <= page) {
      continue;
    }
    
    if (mem->allocators[i].type == 0) {
      anlock_lock(&mem->allocators[i].lock);
      anpages_free(&mem->allocators[i].anpagesRoot, page);
      anlock_unlock(&mem->allocators[i].lock);
      return;
    } else {
      anlock_lock(&mem->allocators[i].lock);
      analloc_t alloc = &mem->allocators[i].anallocRoot;
      analloc_free(alloc, buffer, 1);
      anlock_unlock(&mem->allocators[i].lock);
      return;
    }
  }
}
