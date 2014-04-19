#ifndef __TIME_H__
#define __TIME_H__

#include <stddef.h>
#include <stdint.h>

#define CLOCK_PER_SEC 1000000

typedef uint64_t clock_t;
typedef uint64_t time_t;

struct tm {
  int32_t tm_sec;
  int32_t tm_min;
  int32_t tm_hour;
  int32_t tm_mday;
  int32_t tm_mon;
  int32_t tm_year;
  int32_t tm_wday;
  int32_t tm_yday;
  int32_t tm_isdst;
} __attribute__((packed));

clock_t clock();
double difftime(time_t end, time_t start);
time_t time(time_t * timer);

#endif

