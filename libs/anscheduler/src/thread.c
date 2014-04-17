#include <anscheduler/thread.h>
#include <anscheduler/task.h>
#include <anscheduler/functions.h>
#include <anscheduler/loop.h>
#include <anscheduler/interrupts.h>
#include <anscheduler/paging.h>

/**
 * @critical
 */
bool _alloc_kernel_stack(task_t * task, thread_t * thread);

/**
 * @critical
 */
bool _map_user_stack(task_t * task, thread_t * thread);

/**
 * @critical
 */
void _dealloc_kernel_stack(task_t * task, thread_t * thread);

/**
 * Call from the CPU dedicated stack while the CPU is registered as running
 * the task which the thread belongs to.
 * @critical
 */
void _finalize_thread_exit(thread_t * thread);

thread_t * anscheduler_thread_create(task_t * task) {
  thread_t * thread = anscheduler_alloc(sizeof(thread_t));
  if (!thread) return NULL;
  
  // allocate a stack index and make sure we didn't go over the thread max
  anscheduler_lock(&task->stacksLock);
  uint64_t stack = anidxset_get(&task->stacks);
  anscheduler_unlock(&task->stacksLock);
  if (stack >= 0x100000) {
    // we should not put it back in the idxset because that'll just
    // contribute to the problem.
    anscheduler_free(thread);
    return NULL;
  }
  
  // setup the thread structure
  anscheduler_zero(thread, sizeof(thread_t));
  thread->task = task;
  thread->stack = stack;
  
  if (!_alloc_kernel_stack(task, thread)) {
    anscheduler_lock(&task->stacksLock);
    anidxset_put(&task->stacks, stack);
    anscheduler_unlock(&task->stacksLock);
    anscheduler_free(thread);
    return NULL;
  }
  
  // map the user stack
  if (!_map_user_stack(task, thread)) {
    _dealloc_kernel_stack(task, thread);
    anscheduler_lock(&task->stacksLock);
    anidxset_put(&task->stacks, stack);
    anscheduler_unlock(&task->stacksLock);
    anscheduler_free(thread);
    return NULL;
  }
  
  return thread;
}

void anscheduler_thread_add(task_t * task, thread_t * thread) {
  // add the thread to the task's linked list
  anscheduler_lock(&task->threadsLock);
  thread_t * next = task->firstThread;
  if (next) next->last = thread;
  task->firstThread = thread;
  thread->last = NULL;
  thread->next = next;
  anscheduler_unlock(&task->threadsLock);
  
  // add the thread to the queue
  anscheduler_loop_push(thread);
}

bool anscheduler_thread_poll() {
  thread_t * thread = anscheduler_cpu_get_thread();
  task_t * task = anscheduler_cpu_get_task();
  
  anscheduler_lock(&task->pendingLock);
  if (task->firstPending != NULL) { 
    anscheduler_unlock(&task->pendingLock);
    return false;
  }
  
  // If there are no message packets, we check if there are interrupts or fault
  // signals; these are special cases in which we need to start polling while
  // a respective lock is held so that 
  if (thread == anscheduler_intd_get()) {
    anscheduler_intd_lock();
    if (!anscheduler_intd_waiting()) {
      thread->isPolling = 1;
      anscheduler_intd_unlock();
      anscheduler_unlock(&task->pendingLock);
      return true;
    }
    anscheduler_intd_unlock();
  } else if (thread == anscheduler_pager_get()) {
    anscheduler_pager_lock();
    if (!anscheduler_pager_waiting()) {
      thread->isPolling = 1;
      anscheduler_pager_unlock();
      anscheduler_unlock(&task->pendingLock);
      return true;
    }
    anscheduler_pager_unlock();
  } else {
    thread->isPolling = 1;
    anscheduler_unlock(&task->pendingLock);
    return true;
  }
  
  anscheduler_unlock(&task->pendingLock);
  return false;
}

void anscheduler_thread_exit() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_cpu_unlock();
  
  anscheduler_thread_deallocate(task, thread);
  
  // jump to the CPU stack in order to schedule another task and push a
  // kernel thread for the final dealloc stages.
  anscheduler_cpu_lock();
  anscheduler_cpu_stack_run(thread, (void (*)(void *))_finalize_thread_exit);
}

void anscheduler_thread_deallocate(task_t * task, thread_t * thread) {
  anscheduler_cpu_lock();
  anscheduler_intd_cmpnull(thread);
  anscheduler_cpu_unlock();
  
  uint64_t i;
  uint64_t firstPage = ANSCHEDULER_TASK_USER_STACKS_PAGE
    + (thread->stack << 8);
  
  // unmap all the memory
  for (i = 0; i < 0x100; i++) {
    uint64_t page = firstPage + i;
    anscheduler_cpu_lock();
    anscheduler_lock(&task->vmLock);
    uint16_t flags;
    uint64_t phyPage = anscheduler_vm_lookup(task->vm, page, &flags);
    if (flags & ANSCHEDULER_PAGE_FLAG_PRESENT) {
      flags ^= ANSCHEDULER_PAGE_FLAG_PRESENT;
      flags |= ANSCHEDULER_PAGE_FLAG_UNALLOC;
      // if UNALLOC is set but page != 0, then it needs to be free'd next
      anscheduler_vm_map(task->vm, page, phyPage, flags);
    } else if (flags && !page) {
      // the memory was never allocated to begin with
      anscheduler_vm_unmap(task->vm, page);
    }
    anscheduler_unlock(&task->vmLock);
    anscheduler_cpu_unlock();
  }
  
  // make sure no running instance will be able to access the memory anymore
  anscheduler_cpu_lock();
  anscheduler_cpu_notify_invlpg(task);
  anscheduler_cpu_unlock();
  
  // free all the memory
  for (i = 0; i < 0x100; i++) {
    uint64_t page = firstPage + i;
    anscheduler_cpu_lock();
    anscheduler_lock(&task->vmLock);
    uint16_t flags;
    uint64_t phyPage = anscheduler_vm_lookup(task->vm, page, &flags);
    if (phyPage || flags) {
      anscheduler_vm_unmap(task->vm, page);
    }
    anscheduler_unlock(&task->vmLock);
    if (phyPage && (flags & ANSCHEDULER_PAGE_FLAG_UNALLOC)) {
      uint64_t virPage = anscheduler_vm_virtual(phyPage);
      anscheduler_free((void *)(virPage << 12));
    }
    anscheduler_cpu_unlock();
  }
}

void * anscheduler_thread_kernel_stack(task_t * task, thread_t * thread) {
  uint64_t vPage = ANSCHEDULER_TASK_KERN_STACKS_PAGE + thread->stack;
  anscheduler_lock(&task->vmLock);
  uint16_t flags;
  uint64_t entry = anscheduler_vm_lookup(task->vm, vPage, &flags);
  anscheduler_unlock(&task->vmLock);
  
  if (flags & ANSCHEDULER_PAGE_FLAG_PRESENT) {
    return (void *)(anscheduler_vm_virtual(entry) << 12);
  }
  return NULL;
}

void * anscheduler_thread_interrupt_stack(thread_t * thread) {
  uint64_t page = thread->stack + ANSCHEDULER_TASK_KERN_STACKS_PAGE;
  return (void *)((page + 1) << 12);
}

void * anscheduler_thread_user_stack(thread_t * thread) {
  uint64_t page = (thread->stack << 8) + ANSCHEDULER_TASK_USER_STACKS_PAGE;
  return (void *)((page + 0x100) << 12);
}

bool _alloc_kernel_stack(task_t * task, thread_t * thread) {
  // allocate the kernel stack
  void * buffer = anscheduler_alloc(0x1000);
  if (!buffer) return false;
  
  // map the kernel stack
  uint64_t kPage = ANSCHEDULER_TASK_KERN_STACKS_PAGE + thread->stack;
  uint64_t phyPage = anscheduler_vm_physical(((uint64_t)buffer) >> 12);
  uint16_t flags = ANSCHEDULER_PAGE_FLAG_PRESENT
    | ANSCHEDULER_PAGE_FLAG_WRITE;
  
  anscheduler_lock(&task->vmLock);
  if (!anscheduler_vm_map(task->vm, kPage, phyPage, flags)) {
    anscheduler_unlock(&task->vmLock);
    anscheduler_free(buffer);
    return false;
  }
  anscheduler_unlock(&task->vmLock);
  return true;
}

bool _map_user_stack(task_t * task, thread_t * thread) {
  // map the user stack as to-allocate
  uint64_t flags = ANSCHEDULER_PAGE_FLAG_UNALLOC
    | ANSCHEDULER_PAGE_FLAG_USER
    | ANSCHEDULER_PAGE_FLAG_WRITE;
  uint64_t start = ANSCHEDULER_TASK_USER_STACKS_PAGE + (thread->stack << 8);
  
  uint64_t i;
  anscheduler_lock(&task->vmLock);
  
  for (i = 0; i < 0x100; i++) {
    uint64_t page = i + start;
    if (!anscheduler_vm_map(task->vm, page, 0, flags)) {
      // unmap all that we have mapped so far
      uint64_t j = 0;
      for (j = 0; j < i; j++) {
        page = j + start;
        anscheduler_vm_unmap(task->vm, page);
      }
      anscheduler_unlock(&task->vmLock);
      return false;
    }
  }
  
  anscheduler_unlock(&task->vmLock);
  return true;
}

void _dealloc_kernel_stack(task_t * task, thread_t * thread) {
  // get the phyPage and unmap it
  uint64_t page = ANSCHEDULER_TASK_KERN_STACKS_PAGE + thread->stack;
  anscheduler_lock(&task->vmLock);
  uint16_t flags;
  uint64_t phyPage = anscheduler_vm_lookup(task->vm, page, &flags);
  if (flags & ANSCHEDULER_PAGE_FLAG_PRESENT) {
    anscheduler_vm_unmap(task->vm, page);
  }
  anscheduler_unlock(&task->vmLock);
  
  if (flags & ANSCHEDULER_PAGE_FLAG_PRESENT) {
    uint64_t virPage = anscheduler_vm_virtual(phyPage);
    anscheduler_free((void *)(virPage << 12));
  }
}

void _finalize_thread_exit(thread_t * thread) {
  anscheduler_cpu_set_task(NULL);
  anscheduler_cpu_set_thread(NULL);
  
  task_t * task = thread->task;
  
  // unlink the thread
  anscheduler_lock(&task->threadsLock);
  if (!thread->last) {
    task->firstThread = thread->next;
    if (thread->next) thread->next->last = NULL;
  } else {
    if (thread->last) thread->last->next = thread->next;
    if (thread->next) thread->next->last = thread->last;
  }
  anscheduler_unlock(&task->threadsLock);
  
  anscheduler_lock(&task->stacksLock);
  anidxset_put(&task->stacks, thread->stack);
  anscheduler_unlock(&task->stacksLock);
  
  void * stack = anscheduler_thread_kernel_stack(task, thread);
  anscheduler_free(stack);
  anscheduler_free(thread);
  
  anscheduler_task_dereference(task);
  
  // Running the loop without a push means this thread will never be executed
  // again; just what we want. Additionally, doing this will leave the task
  // referenced, so that our kernel thread won't get screwed over.
  anscheduler_loop_run();
}
