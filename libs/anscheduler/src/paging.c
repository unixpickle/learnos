#include <anscheduler/paging.h>
#include <anscheduler/loop.h>
#include <anscheduler/task.h>
#include <anscheduler/functions.h>

static thread_t * pagerThread __attribute__((aligned(8))) = 0;
static uint64_t lock __attribute__((aligned(8))) = 0;
static page_fault_t * firstFault = 0;

typedef struct {
  void * ptr;
  uint64_t flags;
} fault_info_t;

static void _push_page_fault(fault_info_t * _info);
static bool _push_page_fault_cont(fault_info_t info);

void anscheduler_page_fault(void * ptr, uint64_t _flags) {
  task_t * task = anscheduler_cpu_get_task();
  if (!task) anscheduler_abort("kernel thread caused page fault!");
  
  fault_info_t info;
  info.flags = _flags;
  info.ptr = ptr;
  
  if (_flags & ANSCHEDULER_PAGE_FAULT_PRESENT) {
    // there is nothing we can do here; it's up to the system pager to choose.
    anscheduler_cpu_stack_run(&info, (void (*)(void *))_push_page_fault);
  }
  
  anscheduler_lock(&task->vmLock);
  bool shouldAllocate = false; // overrides shouldFault
  bool shouldFault = true; // if false, try again!
  
  uint16_t flags;
  uint64_t faultPage = ((uint64_t)ptr) >> 12;
  uint64_t entry = anscheduler_vm_lookup(task->vm, faultPage, &flags);
  if ((flags & ANSCHEDULER_PAGE_FLAG_UNALLOC) && !entry) {
    shouldAllocate = true;
  } else if (flags & ANSCHEDULER_PAGE_FLAG_PRESENT) {
    // it is possible that some other thread allocated this page and we
    // simply didn't have it in this CPUs TLB yet...
    if (flags & ANSCHEDULER_PAGE_FLAG_USER) {
      if (flags & ANSCHEDULER_PAGE_FLAG_WRITE) {
        if (!(_flags & ANSCHEDULER_PAGE_FAULT_WRITE)) {
          shouldFault = false;
        }
      } else {
        shouldFault = false;
      }
    }
  }
  
  if (shouldAllocate) {
    flags = ANSCHEDULER_PAGE_FLAG_USER
      | ANSCHEDULER_PAGE_FLAG_PRESENT
      | ANSCHEDULER_PAGE_FLAG_WRITE;
    void * ptr = anscheduler_alloc(0x1000);
    anscheduler_zero(ptr, 0x1000);
    uint64_t physAlloc = anscheduler_vm_physical(((uint64_t)ptr) >> 12);
    anscheduler_vm_map(task->vm, faultPage, physAlloc, flags);
  } else if (shouldFault) {
    anscheduler_unlock(&task->vmLock);
    anscheduler_cpu_stack_run(&info, (void (*)(void *))_push_page_fault);
  }
  
  anscheduler_unlock(&task->vmLock);
  anscheduler_thread_run(task, anscheduler_cpu_get_thread());
}

thread_t * anscheduler_pager_get() {
  return pagerThread;
}

void anscheduler_pager_set(thread_t * thread) {
  pagerThread = thread;
}

page_fault_t * anscheduler_pager_read() {
  while (1) {
    anscheduler_cpu_lock();
    anscheduler_pager_lock();
    page_fault_t * fault = firstFault;
    if (fault) firstFault = fault->next;
    anscheduler_pager_unlock();
    if (!fault) {
      anscheduler_cpu_unlock();
      return NULL;
    }
    
    if (!anscheduler_task_reference(fault->task)) {
      anscheduler_task_dereference(fault->task);
      anscheduler_free(fault);
      anscheduler_cpu_unlock();
      continue;
    }
    // the thing was already referenced anyway
    anscheduler_task_dereference(fault->task);
    anscheduler_cpu_unlock();
    return fault;
  }
}

void anscheduler_pager_lock() {
  anscheduler_lock(&lock);
}

void anscheduler_pager_unlock() {
  anscheduler_unlock(&lock);
}

bool anscheduler_pager_waiting() {
  return firstFault != NULL;
}

static void _push_page_fault(fault_info_t * _info) {
  bool result = _push_page_fault_cont(*_info);

  if (!result) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_MEMORY);
  }
  
  // no other thread got woken up, so we should run the loop
  anscheduler_loop_run();
}

static bool _push_page_fault_cont(fault_info_t info) {
  if (!pagerThread) return false;
  
  // get the current task and then make sure it doesn't get pushed again
  task_t * curTask = anscheduler_cpu_get_task();
  thread_t * curThread = anscheduler_cpu_get_thread();
  if (!curTask) {
    anscheduler_abort("a system thread caused a page fault");
  }
  
  anscheduler_cpu_set_task(NULL);
  anscheduler_cpu_set_thread(NULL);
  
  page_fault_t * fault = anscheduler_alloc(sizeof(page_fault_t));
  if (!fault) {
    anscheduler_abort("failed to allocate fault object");
  }
  fault->task = curTask;
  fault->thread = curThread;
  fault->ptr = info.ptr;
  fault->flags = info.flags;
  
  anscheduler_pager_lock();
  fault->next = firstFault;
  firstFault = fault;
  anscheduler_pager_unlock();
  
  // wakeup the system pager if possible
  bool result = __sync_fetch_and_and(&pagerThread->isPolling, 0);
  if (result) {
    task_t * task = pagerThread->task;
    if (!anscheduler_task_reference(task)) {
      anscheduler_abort("system pager may never die!");
    }
    anscheduler_loop_switch(task, pagerThread);
  }
  return true;
}
