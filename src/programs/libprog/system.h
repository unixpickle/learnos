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
void sys_sleep(uint64_t ticks);

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


/**
 * Exits the current thread's point of execution.
 */
void sys_thread_exit();

/**
 * Initiates the kill process for a given PID.
 */
void sys_pid_kill(uint64_t pid);

#endif
