#include <anscheduler/types.h>

void * anscheduler_alloc(uint64_t size);
void anscheduler_free(void * buffer);
uint64_t antest_pages_alloced();
