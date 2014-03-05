#include <stdint.h>

#define ANIDXSET_NODE_CAPACITY 0x1fe

/**
 * This structure must be a page, meaning 0x200 total uint64_t values.
 * The first and last uint64_t's are taken, leaving 0x1fe left for
 * indexes.
 */
typedef struct {
  uint64_t count;
  uint64_t indexes[ANIDXSET_NODE_CAPACITY];
  void * next;
} __attribute__((packed)) anidxset_node_t;

typedef void * (*anidxset_alloc_t)();
typedef void (*anidxset_free_t)(void * ptr);

typedef struct {
  uint64_t used;
  anidxset_node_t * first;
  anidxset_alloc_t alloc;
  anidxset_free_t free;
} __attribute__((packed)) anidxset_root_t;

uint8_t anidxset_initialize(anidxset_root_t * root,
                            anidxset_alloc_t alloc,
                            anidxset_free_t free);
uint64_t anidxset_get(anidxset_root_t * root);
uint8_t anidxset_put(anidxset_root_t * root, uint64_t value);

