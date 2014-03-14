#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdtype.h>

/**
 * User call.
 * Prints the NULL-terminated string `buffer`.
 */
void sys_print(const char * buffer);

/**
 * User call.
 * Sleeps the current thread for `ticks` units.
 */
void sys_sleep(uint64_t ticks);

/**
 * Administrator call.
 * Returns a mask with all IRQs set that have been received since the last call
 * to sys_getint() and since the thread started.
 */
uint64_t sys_getint();

/**
 * Administrator call.
 * Returns a byte, read from the pin `pin`.
 */
uint8_t sys_in(uint16_t pin);

/**
 * Administrator call.
 * Writes a byte `out` to the pin `pin`.
 */
void sys_out(uint16_t pin, uint8_t out);

#endif
