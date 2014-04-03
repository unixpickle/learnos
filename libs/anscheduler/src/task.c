#include <anscheduler/task.h>
#include <anscheduler/functions.h>
#include <anscheduler/loop.h> // for kernel threads
#include <anscheduler/thread.h> // for deallocation
#include <anscheduler/socket.h> // for socket closing
#include <anscheduler/paging.h>
#include "util.h" // for idxset
#include "pidmap.h"

/**
 * @critical
 */
static bool _map_first_4mb(task_t * task);

/**
 * @critical
 */
static void _generate_kill_job(task_t * task);

/**
 * @noncritical
 */
static void _free_task_method(task_t * task);

/**
 * @noncritical
 */
static void _close_task_sockets_async(task_t * task);

/**
 * @critical
 */
static void _task_exit(void * codeVal);

/**
 * @critical
 */
static void _resign_continuation(void * unused);

/******************
 * Implementation *
 ******************/

task_t * anscheduler_task_create() {
  // allocate memory for task structure
  task_t * task = anscheduler_alloc(sizeof(task_t));
  if (!task) return NULL;
  anscheduler_zero(task, sizeof(task_t));
  
  task->refCount = 1;
  
  if (!(task->vm = anscheduler_vm_root_alloc())) {
    anscheduler_free(task);
    return NULL;
  }
  
  if (!anscheduler_idxset_init(&task->descriptors)) {
    anscheduler_vm_root_free(task->vm);
    anscheduler_free(task);
    return NULL;
  }
  
  if (!anscheduler_idxset_init(&task->stacks)) {
    anidxset_free(&task->descriptors);
    anscheduler_vm_root_free(task->vm);
    anscheduler_free(task);
    return NULL;
  }
  
  if (!_map_first_4mb(task)) {
    anidxset_free(&task->descriptors);
    anidxset_free(&task->stacks);
    anscheduler_vm_root_free(task->vm);
    anscheduler_free(task);
    return NULL;
  }
  
  task->pid = anscheduler_pidmap_alloc_pid();
  return task;
}

void anscheduler_task_launch(task_t * task) {
  anscheduler_task_reference(task);
  
  // add it to the PID list
  anscheduler_pidmap_set(task);
  
  anscheduler_lock(&task->threadsLock);
  thread_t * thread = task->firstThread;
  while (thread) {
    anscheduler_loop_push(thread);
    thread = thread->next;
  }
  anscheduler_unlock(&task->threadsLock);
  anscheduler_task_dereference(task);
}

void anscheduler_task_kill(task_t * task, uint64_t reason) {
  // you cannot kill the system pager
  if (anscheduler_pager_get()) {
    if (task == anscheduler_pager_get()->task) {
      return;
    }
  }
  
  // emulate a test-and-or operation
  anscheduler_lock(&task->killLock);
  if (task->isKilled) {
    anscheduler_unlock(&task->killLock);
    return;
  }
  task->isKilled = true;
  task->killReason = reason;
  anscheduler_unlock(&task->killLock);
  
  // the thing will always have a reference to it because that is a
  // requirement of our `task` argument!
  
  // if (ref) return; // wait for it to get released
  // _generate_kill_job(task);
}

bool anscheduler_task_reference(task_t * task) {
  // emulate a test-and-add operation
  anscheduler_lock(&task->killLock);
  if (task->isKilled) {
    anscheduler_unlock(&task->killLock);
    return false;
  }
  task->refCount++;
  anscheduler_unlock(&task->killLock);
  return true;
}

void anscheduler_task_dereference(task_t * task) {
  anscheduler_lock(&task->killLock);
  if (!(--task->refCount)) {
    if (task->isKilled) {
      anscheduler_unlock(&task->killLock);
      return _generate_kill_job(task);
    }
  }
  anscheduler_unlock(&task->killLock);
}

task_t * anscheduler_task_for_pid(uint64_t pid) {
  return anscheduler_pidmap_get(pid);
}

void anscheduler_task_exit(uint8_t code) {
  anscheduler_cpu_stack_run((void *)((long)code), _task_exit);
}

/******************
 * Helper Methods *
 ******************/

static bool _map_first_4mb(task_t * task) {
  uint64_t i;
  for (i = 0; i < 0x400; i++) {
    uint64_t flags = ANSCHEDULER_PAGE_FLAG_PRESENT
      | ANSCHEDULER_PAGE_FLAG_WRITE
      | ANSCHEDULER_PAGE_FLAG_GLOBAL;
    if (!anscheduler_vm_map(task->vm, i, i, flags)) return false;
  }
  return true;
}

static void _generate_kill_job(task_t * task) {
  // Note: By the time we get here, we know nothing is running our task and
  // nothing will ever run it again.  The loop may attempt to reference
  // threads that miraculously made it back into the queue, but these will
  // fail because the task has been killed.
  
  // Remove the task's threads from the queue. There is no need to lock the
  // threads lock anymore, because nothing could possibly be doing anything
  // to the threads array of this unreferenced task.
  thread_t * thread = task->firstThread;
  while (thread) {
    anscheduler_loop_delete(thread);
    thread = thread->next;
  }
  
  // Generate a kernel thread and pass it our task.
  anscheduler_loop_push_kernel(task, (void (*)(void *))_free_task_method);
}

static void _free_task_method(task_t * task) {
  anscheduler_cpu_lock();
  anscheduler_pidmap_unset(task);
  anscheduler_cpu_unlock();
  
  // wait for each socket to die so that we know nothing references the task
  _close_task_sockets_async(task);
  
  // free each thread and all its resources
  while (task->firstThread) {
    thread_t * thread = task->firstThread;
    task->firstThread = thread->next;
    anscheduler_thread_deallocate(task, thread);
    anscheduler_cpu_lock();
    void * stack = anscheduler_thread_kernel_stack(task, thread);
    anscheduler_free(stack);
    anscheduler_free(thread);
    anscheduler_cpu_unlock();
  }
  
  anscheduler_task_cleanup(task);
  anscheduler_vm_root_free_async(task->vm);
  
  anscheduler_cpu_lock();
  
  // free the general structures of the task
  anidxset_free(&task->stacks);
  anidxset_free(&task->descriptors);
  
  anscheduler_pidmap_free_pid(task->pid);
  anscheduler_free(task);
  
  anscheduler_loop_delete_cur_kernel();
}

static void _close_task_sockets_async(task_t * task) {
  anscheduler_cpu_lock();
  thread_t * thread = anscheduler_cpu_get_thread();
  
  int i;
  for (i = 0; i < 0x10; i++) {
    // free each socket with this descriptor hash (i is the hash, btw)
    while (1) {
      // reference the first socket
      anscheduler_lock(&task->socketsLock);
      socket_desc_t * desc = task->sockets[i];
      if (desc) {
        desc = (anscheduler_socket_reference(desc) ? desc : NULL);
      }
      anscheduler_unlock(&task->socketsLock);
      
      // break if there *is* no first socket
      if (!desc) break;
      
      // close the socket and wait for it to disappear from the list
      anscheduler_socket_close(desc, 1 | (task->killReason << 1));
      anscheduler_socket_dereference(desc);
      while (1) {
        // we know another socket in the sockets[] array won't be the address
        // of desc even though desc may be free'd, because if it's re-used
        // it'll have to be re-used for some *other* task! 
        anscheduler_lock(&task->socketsLock);
        if (task->sockets[i] != desc) {
          anscheduler_unlock(&task->socketsLock);
          break;
        }
        anscheduler_save_return_state(thread, NULL, _resign_continuation);
      }
    }
  }
  
  anscheduler_cpu_unlock();
}

static void _task_exit(void * codeVal) {
  task_t * task = anscheduler_cpu_get_task();
  anscheduler_cpu_set_task(NULL);
  anscheduler_cpu_set_thread(NULL);
  anscheduler_task_kill(task, (uint64_t)codeVal);
  anscheduler_task_dereference(task);
  anscheduler_loop_run();
}

static void _resign_continuation(void * unused) {
  anscheduler_loop_resign();
}
