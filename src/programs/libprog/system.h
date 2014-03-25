#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdtype.h>

/**
 * User call.
 * Prints the NULL-terminated string `buffer`.
 */
void sys_print(const char * buffer);

/**
 * Returns the system time in microseconds
 */
uint64_t sys_get_time();

/**
 * User call.
 * Sleeps the current thread for `ticks` microseconds
 */

#endif
