#include <anscheduler/functions.h>

static void * anscheduler_idxset_alloc();
static void anscheduler_idxset_free(void * ptr);

uint8_t anscheduler_idxset_init(anidxset_root_t * root) {
  return anidxset_initialize(root,
                             anscheduler_idxset_alloc,
                             anscheduler_idxset_free);
}

static void * anscheduler_idxset_alloc() {
  return anscheduler_alloc(0x1000);
}

static void anscheduler_idxset_free(void * ptr) {
  anscheduler_free(ptr);
}

