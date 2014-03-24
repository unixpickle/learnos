#include <anscheduler/types.h>

void anscheduler_lock(uint64_t * ptr);
void anscheduler_unlock(uint64_t * ptr);
void anscheduler_abort(const char * error);
void anscheduler_zero(void * buf, int len);
void anscheduler_inc(uint64_t * ptr);
void anscheduler_or_32(uint32_t * ptr, uint32_t flag);