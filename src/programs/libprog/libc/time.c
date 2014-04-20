#include "time.h"
#include <system.h>

clock_t clock() {
  return time(NULL);
}

double difftime(time_t end, time_t start) {
  return (double)(end - start);
}

time_t time(time_t * timer) {
  time_t res = sys_get_time();
  if (timer) *timer = res;
  return res;
}

