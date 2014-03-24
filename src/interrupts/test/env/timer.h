#include <anscheduler/types.h>

void anscheduler_timer_set(uint32_t ticks);
void anscheduler_timer_set_far();
void anscheduler_timer_cancel();
uint64_t anscheduler_get_time();
uint64_t anscheduler_second_length();
