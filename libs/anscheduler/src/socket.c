#include <anscheduler/socket.h>
#include <anscheduler/functions.h>
#include <anscheduler/loop.h>
#include <anscheduler/task.h>
#include "socketlist.h"

typedef struct {
  socket_msg_t * message;
  socket_desc_t * descriptor; // referenced
} msginfo_t;

/**
 * @critical
 */
static socket_desc_t * _create_descriptor(socket_t * socket,
                                          task_t * task,
                                          bool isConnector);

/**
 * @noncritical Run from a kernel thread, though.
 */
static void _socket_hangup(socket_desc_t * socket);

/**
 * Send a socket message to the other end of a socket.
 * @param dest A referenced socket. When a socket is referenced, you °know*
 * that the destination task will not be deallocated, although it may be
 * killed.
 * @critical
 */
static bool _push_message(socket_desc_t * dest, socket_msg_t * msg);

/**
 * Wakes up the task for a socket descriptor. If the task has been killed,
 * this will not complete it's job. No matter what, the passed descriptor's
 * reference will be consumed.
 * @critical -> @noncritical -> @critical
 */
static void _wakeup_endpoint(socket_desc_t * dest);

/**
 * @noncritical Run from a kernel thread
 */
static void _async_msg(msginfo_t * info);

/**
 * @noncritical
 */
static void _socket_free(socket_t * socket);

/**
 * @critical
 */
static void _switch_continuation(void * th);

socket_desc_t * anscheduler_socket_new() {
  socket_t * socket = anscheduler_alloc(sizeof(socket_t));
  if (!socket) return NULL;
  anscheduler_zero(socket, sizeof(socket_t));
  return _create_descriptor(socket, anscheduler_cpu_get_task(), true);
}

socket_desc_t * anscheduler_socket_for_descriptor(uint64_t desc) {
  task_t * task = anscheduler_cpu_get_task();
  return anscheduler_descriptor_find(task, desc);
}

socket_desc_t * anscheduler_socket_next_pending() {
  task_t * task = anscheduler_cpu_get_task();
  return anscheduler_task_pop_pending(task);
}

bool anscheduler_socket_reference(socket_desc_t * socket) {
  anscheduler_lock(&socket->closeLock);
  if (socket->isClosed) {
    anscheduler_unlock(&socket->closeLock);
    return false;
  }
  socket->refCount++;
  anscheduler_unlock(&socket->closeLock);
  return true;
}

void anscheduler_socket_dereference(socket_desc_t * socket) {
  anscheduler_lock(&socket->closeLock);
  if (socket->isClosed && !socket->refCount) {
    anscheduler_unlock(&socket->closeLock);
    return;
  }
  if (!(--socket->refCount) && socket->isClosed) {
    anscheduler_unlock(&socket->closeLock);
    
    // we know the task is still alive because the socket is still in the
    // task's socket list as of now, and the task cannot die until
    // every socket it owns has died.
    anscheduler_task_not_pending(socket->task, socket);
    anscheduler_descriptor_delete(socket->task, socket);
    
    // now, we don't know the task is alive, but it doesn't matter anymore
    anscheduler_loop_push_kernel(socket, (void (*)(void *))_socket_hangup);
    return;
  }
  anscheduler_unlock(&socket->closeLock);
}

bool anscheduler_socket_msg(socket_desc_t * socket,
                            socket_msg_t * msg) {
  // gain a reference to the other end of the socket
  socket_desc_t * otherEnd = NULL;
  socket_t * sock = socket->socket;
  anscheduler_lock(&sock->connRecLock);
  if (socket->isConnector) {
    otherEnd = sock->receiver;
  } else {
    otherEnd = sock->connector;
  }
  if (otherEnd) {
    otherEnd = anscheduler_socket_reference(otherEnd) ? otherEnd : NULL;
  }
  anscheduler_unlock(&sock->connRecLock);
  
  if (!otherEnd) return false;
  if (!_push_message(otherEnd, msg)) {
    anscheduler_socket_dereference(otherEnd);
    return false;
  }
  
  anscheduler_socket_dereference(socket); // cannot hold a ref across this
  _wakeup_endpoint(otherEnd);
  return true;
}

void anscheduler_socket_msg_async(socket_desc_t * socket,
                                  socket_msg_t * msg) {
  // retain the socket until we send the message
  if (!anscheduler_socket_reference(socket)) {
    anscheduler_free(msg);
    return;
  }
  
  msginfo_t * info = anscheduler_alloc(sizeof(msginfo_t));
  if (!info) {
    anscheduler_abort("failed to allocate async message info");
  }
  
  info->message = msg;
  info->descriptor = socket;
  anscheduler_loop_push_kernel(info, (void (*)(void *))_async_msg);
}

socket_msg_t * anscheduler_socket_msg_data(const void * data, uint64_t len) {
  if (len >= 0xfe8) return NULL;
  socket_msg_t * msg = anscheduler_alloc(sizeof(socket_msg_t));
  if (!msg) return NULL;
  
  msg->type = ANSCHEDULER_MSG_TYPE_DATA;
  msg->len = len;
  const uint8_t * source = (const uint8_t *)data;
  int i;
  for (i = 0; i < len; i++) {
    msg->message[i] = source[i];
  }
  
  return msg;
}

socket_msg_t * anscheduler_socket_read(socket_desc_t * dest) {
  uint64_t * lock, * count;
  socket_msg_t ** first, ** last;
  if (dest->isConnector) {
    lock = &dest->socket->forConnectorLock;
    count = &dest->socket->forConnectorCount;
    first = &dest->socket->forConnectorFirst;
    last = &dest->socket->forConnectorLast;
  } else {
    lock = &dest->socket->forReceiverLock;
    count = &dest->socket->forReceiverCount;
    first = &dest->socket->forReceiverFirst;
    last = &dest->socket->forReceiverLast;
  }
  
  anscheduler_lock(lock);
  if ((*count) == 0) {
    anscheduler_unlock(lock);
    return NULL;
  }
  socket_msg_t * res = *first;
  if (!res->next) {
    (*first) = ((*last) = NULL);
  } else {
    (*first) = res->next;
  }
  (*count)--;
  res->next = NULL;
  anscheduler_unlock(lock);
  return res;
}

bool anscheduler_socket_connect(socket_desc_t * socket, task_t * task) {
  if (__sync_fetch_and_or(&socket->socket->hasBeenConnected, 1)) {
    return false;
  }
  
  // generate another link
  socket_desc_t * link = _create_descriptor(socket->socket, task, false);
  if (!link) {
    return false;
  }
  
  anscheduler_task_dereference(task);
  anscheduler_socket_dereference(link);
  
  socket_msg_t * msg = anscheduler_alloc(sizeof(socket_msg_t));
  if (!msg) {
    anscheduler_abort("failed to allocate connect message");
  }
  msg->type = ANSCHEDULER_MSG_TYPE_CONNECT;
  msg->len = 0;
  
  // if we couldn't send the message, we'd need to release our resources
  if (!anscheduler_socket_msg(socket, msg)) {
    anscheduler_abort("failed to send connect message");
  }
  return true;
}

void anscheduler_socket_close(socket_desc_t * socket, uint64_t code) {
  anscheduler_lock(&socket->closeLock);
  if (!socket->isClosed) socket->closeCode = code;
  socket->isClosed = true;
  anscheduler_unlock(&socket->closeLock);
}

task_t * anscheduler_socket_remote(socket_desc_t * socket) {
  socket_desc_t * otherEnd = NULL;
  socket_t * sock = socket->socket;
  anscheduler_lock(&sock->connRecLock);
  if (socket->isConnector) {
    otherEnd = sock->receiver;
  } else {
    otherEnd = sock->connector;
  }
  if (otherEnd) {
    otherEnd = anscheduler_socket_reference(otherEnd) ? otherEnd : NULL;
  }
  anscheduler_unlock(&sock->connRecLock);
  if (!otherEnd) return NULL;
  bool res = anscheduler_task_reference(otherEnd->task);
  return res ? otherEnd->task : NULL;
}

static socket_desc_t * _create_descriptor(socket_t * socket,
                                          task_t * task,
                                          bool isConnector) {
  socket_desc_t * desc = anscheduler_alloc(sizeof(socket_desc_t));
  if (!desc) return NULL;
  
  anscheduler_zero(desc, sizeof(socket_desc_t));
  desc->socket = socket;
  desc->task = task;
  desc->refCount = 1;
  desc->isConnector = isConnector;
  
  anscheduler_lock(&task->descriptorsLock);
  desc->descriptor = anidxset_get(&task->descriptors);
  anscheduler_unlock(&task->descriptorsLock);
  
  anscheduler_lock(&socket->connRecLock);
  if (isConnector) {
    socket->connector = desc;
  } else {
    socket->receiver = desc;
  }
  anscheduler_unlock(&socket->connRecLock);
  
  anscheduler_descriptor_set(task, desc);
  return desc;
}

static void _socket_hangup(socket_desc_t * socket) {
  anscheduler_cpu_lock();
  
  // generate the hangup message
  socket_t * sock = socket->socket;
  
  // If the other socket is still open, we can write to it. Otherwise, we set
  // ourselves as NULL and check if we should free the entire socket.
  socket_desc_t * otherEnd = NULL;
  bool shouldFree = false;
  anscheduler_lock(&sock->connRecLock);
  if (socket->isConnector) {
    otherEnd = sock->receiver;
  } else {
    otherEnd = sock->connector;
  }
  if (otherEnd) {
    otherEnd = anscheduler_socket_reference(otherEnd) ? otherEnd : NULL;
  }
  if (!otherEnd) {
    // hangup now
    if (socket->isConnector) sock->connector = NULL;
    else sock->receiver = NULL;
    shouldFree = sock->receiver == sock->connector;
  }
  anscheduler_unlock(&sock->connRecLock);
  
  if (!otherEnd) {
    anscheduler_free(socket);
    if (shouldFree) {
      anscheduler_cpu_unlock();
      _socket_free(sock);
      anscheduler_cpu_lock();
    }
  } else {
    socket_msg_t * msg = anscheduler_alloc(sizeof(socket_msg_t));
    if (!msg) {
      anscheduler_abort("failed to allocate close message!");
    }
    msg->type = ANSCHEDULER_MSG_TYPE_CLOSE;
    msg->len = 8;
    (*((uint64_t *)msg->message)) = socket->closeCode;
    
    _push_message(otherEnd, msg);
    _wakeup_endpoint(otherEnd);
    
    // by this point, the other end may have freed up everything
    anscheduler_lock(&sock->connRecLock);
    if (socket->isConnector) sock->connector = NULL;
    else sock->receiver = NULL;
    shouldFree = sock->receiver == sock->connector;
    anscheduler_unlock(&sock->connRecLock);
    
    anscheduler_free(socket);
    if (shouldFree) { 
      anscheduler_cpu_unlock();
      _socket_free(sock);
      anscheduler_cpu_lock();
    }
  }
  
  anscheduler_loop_delete_cur_kernel();
}

static bool _push_message(socket_desc_t * dest, socket_msg_t * msg) {
  uint64_t * lock, * count;
  socket_msg_t ** first, ** last;
  if (dest->isConnector) {
    lock = &dest->socket->forConnectorLock;
    count = &dest->socket->forConnectorCount;
    first = &dest->socket->forConnectorFirst;
    last = &dest->socket->forConnectorLast;
  } else {
    lock = &dest->socket->forReceiverLock;
    count = &dest->socket->forReceiverCount;
    first = &dest->socket->forReceiverFirst;
    last = &dest->socket->forReceiverLast;
  }
  
  anscheduler_lock(lock);
  if (msg->type == ANSCHEDULER_MSG_TYPE_DATA) {
    if (*count >= ANSCHEDULER_SOCKET_MSG_MAX) {
      anscheduler_unlock(lock);
      return false;
    }
  }
  
  // add it to the linked list
  (*count)++;
  msg->next = NULL;
  if (!(*last)) {
    (*last) = ((*first) = msg);
  } else {
    (*last)->next = msg;
    (*last) = msg;
  }
  anscheduler_unlock(lock);
  return true;
}

static void _wakeup_endpoint(socket_desc_t * dest) {
  if (!anscheduler_task_reference(dest->task)) {
    anscheduler_socket_dereference(dest);
    return;
  }
  
  task_t * task = dest->task;
  anscheduler_task_pending(task, dest);
  anscheduler_socket_dereference(dest);
  
  anscheduler_lock(&task->threadsLock);
  thread_t * thread = task->firstThread;
  while (thread) {
    if (__sync_fetch_and_and(&thread->isPolling, 0)) {
      anscheduler_unlock(&task->threadsLock);
      thread_t * curThread = anscheduler_cpu_get_thread();
      anscheduler_save_return_state(curThread, thread, _switch_continuation);
      return;
    }
    thread = thread->next;
  }
  anscheduler_unlock(&task->threadsLock);
  
  // no polling threads were found
  anscheduler_task_dereference(dest->task);
}

static void _async_msg(msginfo_t * _info) {
  anscheduler_cpu_lock();
  
  msginfo_t info = *_info;
  anscheduler_free(_info);
  if (!anscheduler_socket_msg(info.descriptor, info.message)) {
    anscheduler_free(info.message);
    anscheduler_socket_dereference(info.descriptor);
  }
  anscheduler_loop_delete_cur_kernel();
}

static void _socket_free(socket_t * socket) {
  // free for connector messages
  while (true) {
    anscheduler_cpu_lock();
    if (!socket->forConnectorFirst) {
      anscheduler_cpu_unlock();
      break;
    }
    socket_msg_t * msg = socket->forConnectorFirst;
    socket->forConnectorFirst = msg->next;
    anscheduler_free(msg);
    anscheduler_cpu_unlock();
  }
  
  // free for receiver messages
  while (true) {
    anscheduler_cpu_lock();
    if (!socket->forReceiverFirst) {
      anscheduler_cpu_unlock();
      break;
    }
    socket_msg_t * msg = socket->forReceiverFirst;
    socket->forReceiverFirst = msg->next;
    anscheduler_free(msg);
    anscheduler_cpu_unlock();
  }
  
  anscheduler_cpu_lock();
  anscheduler_free(socket);
  anscheduler_cpu_unlock();
}

static void _switch_continuation(void * th) {
  thread_t * thread = (thread_t *)th;
  anscheduler_loop_switch(thread->task, thread);
}
