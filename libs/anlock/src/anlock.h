#include <stdint.h>

typedef uint64_t * anlock_t;

void anlock_initialize(anlock_t lock);
void anlock_lock(anlock_t lock);
void anlock_lock_waiting(anlock_t lock, void * data, void (*fn)(void * d));
void anlock_unlock(anlock_t lock);
