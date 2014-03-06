#include "thread.h"
#include <kernpage.h>

thread_t * thread_create_user(task_t * task, void * rip) {
  kernpage_lock();
  page_t mainPage = kernpage_alloc_virtual();
  page_t kStack = kernpage_alloc_virtual();
  kernpage_unlock();
  if (!mainPage || !kStack) {
    kernpage_lock();
    if (mainPage) kernpage_free_virtual(mainPage);
    if (kStack) kernpage_free_virtual(kStack);
    kernpage_unlock();
    return NULL;
  }
  thread_t * thread = (thread_t *)(mainPage << 12);
  ref_initialize(&thread, (void (*)(void *))thread_dealloc);
  // todo: great things, here!
  return thread;
}

thread_t * thread_create_first(task_t * task,
                               void * rip,
                               void * program,
                               uint64_t len) {
  return NULL; // NYI
}

void thread_dealloc(thread_t * thread) {
  // NYI lolol
}

void thread_configure_user_stack(void * rip) {
  // NYI
}

void thread_configure_user_program(void * rip, void * program, uint64_t len) {
  // NYI
}

