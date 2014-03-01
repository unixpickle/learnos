#include <stdint.h>

typedef struct {
  uint64_t start;
  uint64_t used;
  uint64_t list;
  uint64_t total;
} __attribute__((packed)) * anpages_t;

uint8_t anpages_initialize(anpages_t pages, uint64_t start, uint64_t total);
uint64_t anpages_alloc(anpages_t pages);
void anpages_free(anpages_t pages, uint64_t page);
