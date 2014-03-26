#include <stdint.h>
#include "functions.h"

/**
 * Create a new socket. This returns a new FD, or FD_INVAL on error.
 */
uint64_t syscall_open_socket();

/**
 * Connects a file descriptor to a task with the given process ID. Returns 0 on
 * failure, 1 on success.
 */
uint64_t syscall_connect(uint64_t desc, uint64_t pid);

/**
 * Close a socket given a file descriptor. After you call this, you may never
 * rely on this file descriptor again (it may become a different socket).
 */
void syscall_close_socket(uint64_t desc);

/**
 * Send a data message to a remote on a socket. Returns 1 if the message was
 * sent, or 0 if the buffer was full or the remote is no longer connected.
 */
uint64_t syscall_write(uint64_t desc, uint64_t ptr, uint64_t len);

/**
 * Read the next message page into the address at ptr. The address must point
 * to a memory chunk at least one page large. I will be making sure it's mapped,
 * broseph.
 */
uint64_t syscall_read(uint64_t desc, uint64_t ptr);

/**
 * Waits until a messages comes in on *some* socket (or an interrupt, if you're
 * into that kind of thing). Returns the next waiting file descriptor, or
 * FD_INVAL if, for some reason, the thread was woken up without cause. You must
 * read all the data from this fd once you get it, because it will be popped
 * from the task's pending queue.
 */
uint64_t syscall_poll();

