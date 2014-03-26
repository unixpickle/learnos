#ifndef __ANSCHEDULER_SOCKET_H__
#define __ANSCHEDULER_SOCKET_H__

#include "types.h"

#define ANSCHEDULER_SOCKET_MSG_MAX 0x10

/**
 * Creates a new socket and assigns it to the current task.
 * @return The socket link in the task's sockets linked list. NULL if any
 * part of socket allocation failed. This socket will be referenced.
 * @critical
 */
socket_desc_t * anscheduler_socket_new();

/**
 * Finds the socket for a socket descriptor in the current task. The returned
 * socket will be referenced.
 * @critical
 */
socket_desc_t * anscheduler_socket_for_descriptor(uint64_t desc);

/**
 * Gets a socket which has pending data on it. The returned socket will be
 * referenced. If NULL is returned, no sockets are currently pending.
 * @critical
 */
socket_desc_t * anscheduler_socket_next_pending();

/**
 * Reference a socket. Returns false if the socket has been closed.
 * @critical
 */
bool anscheduler_socket_reference(socket_desc_t * socket);

/**
 * Dereference a socket. If the socket has been closed and the retain count
 * reaches zero, this may cause the underlying socket to be closed, or
 * trigger an asynchronous message to the other end of the socket.
 * @critical This could lead to a socket shutdown, which may call
 * anscheduler_socket_msg(). See that function's @critical section for more.
 */
void anscheduler_socket_dereference(socket_desc_t * socket);

/**
 * Send a message to the socket.
 * @param socket A referenced socket link.
 * @param msg The message to push to the socket.
 *
 * @return true when the message was sent, false when the buffer was full.
 * When false is returned, you are responsible for freeing the message.
 * When the message is sent successfully, the reference to `socket` is
 * also consumed. You should not hold any references or locks across this
 * method.
 *
 * @critical -> @noncritical -> @critical consider this the END of the
 * critical section. If/when it returns, it will be the beginning of a new
 * critical section. This means: do not hold any locks or references across
 * this call.
 */
bool anscheduler_socket_msg(socket_desc_t * socket,
                            socket_msg_t * msg);

/**
 * Triggers an asynchronous message send. If you use this, you will have no
 * way of knowing if the message ever went through or not. Thus, this method
 * will free `msg` for you, but it will not consume the reference to socket.
 * @critical All the work will be done in a kernel thread, so this is fast.
 */
void anscheduler_socket_msg_async(socket_desc_t * socket,
                                  socket_msg_t * msg);

/**
 * Allocates a socket message with specified data. Maximum length for the
 * data is 0xfe8 bytes. May return NULL if the message could not be 
 * allocated.
 * @critical
 */
socket_msg_t * anscheduler_socket_msg_data(const void * data, uint64_t len);

/**
 * Returns the next message on the queue, or NULL if no messages are pending.
 * @param socket A referenced socket link.
 * @critical
 */
socket_msg_t * anscheduler_socket_read(socket_desc_t * socket);

/**
 * Connects a socket to a different task. No references will be conusmed if
 * this function returns false.
 *
 * @param socket A referenced socket link. The reference will be consumed.
 * @param task A referenced task. The reference will be consumed.
 * @return true if connect succeeded, false otherwise
 * @critical -> @noncritical -> @critical
 */
bool anscheduler_socket_connect(socket_desc_t * socket, task_t * task);

/**
 * Closes a socket from the current task.  This may not cause the underlying
 * socket to be terminated, but it *will* notify all connected endpoints of
 * the disconnect.
 * @param socket A referenced socket link. Once this is dereferenced, the
 * socket close notification will go through, and the socket may potentially
 * be freed all together.
 * @critical
 */
void anscheduler_socket_close(socket_desc_t * socket, uint64_t code);

/**
 * Finds the remote task which is connected to this socket. If the task is
 * found, it is referenced and returned. Otherwise, NULL is returned.
 * @critical
 */
task_t * anscheduler_socket_remote(socket_desc_t * socket);

#endif
