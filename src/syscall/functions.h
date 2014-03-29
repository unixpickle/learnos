#ifndef __INTERRUPT_FUNCTIONS_H__
#define __INTERRUPT_FUNCTIONS_H__

#include <stdint.h>

#define FD_INVAL 0xffffffffffffffffL
#define ANSCHEDULER_TASK_KILL_REASON_ACCESS 3

typedef struct {
  uint64_t rax;
  uint64_t rbp;
  uint64_t rbx;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rip;
  uint64_t rsp;
  uint64_t cr3;
} __attribute__((packed)) restore_regs;

uint64_t syscall_entry(uint64_t arg1,
                       uint64_t arg2,
                       uint64_t arg3,
                       uint64_t arg4);
void syscall_return(restore_regs * regs);

/**
 * Print out a string.
 */
void syscall_print(void * ptr);

/**
 * Return the current system time in microseconds.
 */
uint64_t syscall_get_time();

/**
 * Sleep for a specified number of microseconds.
 */
void syscall_sleep(uint64_t until);

/**
 * Terminate the current task.
 */
void syscall_exit();

/**
 * Terminate the current thread of this task. Note that the task will not die
 * even if this is the last thread.
 */
void syscall_thread_exit();

/**
 * Tells the system that the current thread would like to be the designated
 * interrupt server.
 * @administrator
 */
void syscall_wants_interrupts();

/**
 * Returns the thread's interrupt mask. If the thread is not the interrupt
 * server, 0 will always be returned.
 */
uint64_t syscall_get_interrupts();

/**
 * Sets the current terminal color.
 */
void syscall_set_color(uint8_t color);

/**
 * Returns the number of pages allocated by the page allocator.
 */
uint64_t syscall_mem_usage();

#endif
