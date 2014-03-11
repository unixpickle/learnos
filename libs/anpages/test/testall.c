#include <anpages.h>
#include <stdio.h>
#include <stdlib.h>

void die(void * buffer, const char * msg) {
  free(buffer); // grace, bro, it's important
  fprintf(stderr, "[DIE]: %s\n", msg);
  exit(1);
}

int main(int argc, const char * argv[]) {
  // do 0x3FD pages (0x3FE to ensure alignment) to get two entries in the
  // linked list.
  uint8_t * buffer = (uint8_t *)malloc(0x3FE000);
  
  // we need to align the buffer up to the next page
  uint64_t masked = ((uint64_t)buffer & 0xfffffffffffff000);
  if (masked != (uint64_t)buffer) masked += 0x1000;
  uint64_t start = masked >> 0xc;
  
  uint64_t fields[4];
  anpages_t pages = (anpages_t)fields;
  
  anpages_initialize(pages, start, 0x3fd);
  
  // first of all, we *know* where the first page is going to be
  uint64_t * table = (uint64_t *)masked;
  if (table[0x1ff] != 0) die(buffer, "invalid initial next value");
  if (table[0] != 0) die(buffer, "invalid initial count value");
  
  // allocate 0x1fe pages and make sure they're consecutive
  uint64_t i;
  for (i = 0; i < 0x1fe; i++) {
    uint64_t aPage = anpages_alloc(pages);
    if (aPage != start + 0x1fe - i) {
      die(buffer, "invalid first-link page index");
    }
  }
  
  // allocate 0x1fe pages more and make sure they're consecutive
  for (i = 0; i < 0x1fe; i++) {
    uint64_t aPage = anpages_alloc(pages);
    if (aPage != start + 0x3fc - i) {
      die(buffer, "invalid first-link second-round page index");
    }
  }
  
  // free the first 0x1fe pages and make sure it's reflected properly
  for (i = start + 1; i < start + 0x1ff; i++) {
    anpages_free(pages, i);
    if (table[0] != i - start) die(buffer, "invalid first-free round");
  }
  
  // free the next page and see what happens
  anpages_free(pages, start + 0x1ff);
  if (table[0] != 0x1fe) die(buffer, "invalid table after 1ff'th free.");
  if (pages->list != start + 0x1ff) die(buffer, "invalid new pages list");
  
  if (anpages_alloc(pages) != start + 0x1ff) { 
    die(buffer, "failed to free table list");
  }
  if (pages->list != start) die(buffer, "freeing table list error");
  
  printf("all tests passed.\n");
  free(buffer);
  return 0;
}


