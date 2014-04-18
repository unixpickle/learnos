#include <anmalloc_bindings.h>
#include <stdint.h>

// anmalloc_bindings.h must define the following functions:
// - anmalloc_sbrk
// - anmalloc_brk
// - anmalloc_lock
// - anmalloc_unlock
// and these other defines
// - ANMALLOC_LOCK_INIT
// - anmalloc_lock_t

void anmalloc_free(void * buf);
void * anmalloc_alloc(uint64_t size);
void * anmalloc_aligned(uint64_t alignment, uint64_t size);
void * anmalloc_realloc(void * ptr, uint64_t size);
uint64_t anmalloc_used();
