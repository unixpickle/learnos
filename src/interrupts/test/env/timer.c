#include "timer.h"
#include "threading.h"
#include <sys/time.h>

void anscheduler_timer_set(uint32_t ticks) {
  uint64_t next = anscheduler_get_time() + (uint64_t)ticks;
  antest_get_current_cpu_info()->nextInterrupt = next;
}

void anscheduler_timer_set_far() {
  anscheduler_timer_cancel();
}

void anscheduler_timer_cancel() {
  antest_get_current_cpu_info()->nextInterrupt = 0xffffffffffffffffL;
}

uint64_t anscheduler_get_time() {
  struct timeval timeval;
  gettimeofday(&timeval, NULL);
  return timeval.tv_usec + (timeval.tv_sec * 1000000);
}

uint64_t anscheduler_second_length() {
  return 1000000;
}
