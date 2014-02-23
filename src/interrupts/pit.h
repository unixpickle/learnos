#include <shared/types.h>

/**
 * Driver for the global Programmable Interval Timer.
 */

/**
 * Sets the divisor of 1.193182MHz for the PIT.
 * Suggest value 11932 for ~100 Hz
 */
void pit_set_divisor(uint16_t divisor);

/**
 * Sleeps for a number of ticks.
 */
void pit_sleep(uint64_t ticks);

