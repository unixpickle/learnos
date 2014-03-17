#include "anpages.h"

typedef struct {
  uint64_t count; // the number of pages in this table
  uint64_t pages[0x1fe]; // page indexes
  uint64_t next; // address, not page; first bit = present
} __attribute__((packed)) anpagelist_t;

uint8_t anpages_initialize(anpages_t pages, uint64_t start, uint64_t total) {
  if (total == 0) return 0;
  
  pages->list = start;
  pages->start = start;
  pages->used = 1;
  pages->total = total;
  
  // create a new, empty list structure
  anpagelist_t * list = (anpagelist_t *)(pages->list << 0xc);
  list->count = 0;
  list->next = 0;
  
  return 1;
}

uint64_t anpages_alloc(anpages_t pages) {
  anpagelist_t * list = (anpagelist_t *)(pages->list << 0xc);
  if (list->count > 0) {
    // grab the last physical page from the list
    return list->pages[--(list->count)];
  } else if (list->next & 1L) {
    uint64_t page = pages->list;
    pages->list = list->next >> 0xc;
    return page;
  } else {
    if (pages->used < pages->total) {
      // add a certain number of unused pages to the queue
      uint64_t add = pages->total - pages->used;
      if (add > 0x1fe) add = 0x1fe;
      for (; add > 0; add--) {
        list->pages[list->count++] = pages->start + pages->used++;
      }
    } else return 0;
    return anpages_alloc(pages);
  }
}

void anpages_free(anpages_t pages, uint64_t page) {
  anpagelist_t * list = (anpagelist_t *)(pages->list << 0xc);
  if (list->count == 0x1fe) {
    // since the root list is full, we'll use our new free page to create
    // a new empty root list.
    list = (anpagelist_t *)(page << 0xc);
    list->count = 0;
    list->next = (pages->list << 0xc) | 1L;
    pages->list = page;
  } else {
    list->pages[list->count++] = page;
  }
}
