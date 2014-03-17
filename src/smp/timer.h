#include <stdint.h>

void timer_send_eoi();
void timer_set_timeout(uint32_t ticks);
void timer_set_far_timeout();
void timer_cancel_timeout();

uint64_t timer_get_time();
uint64_t timer_second_length();

