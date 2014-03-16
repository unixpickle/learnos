task_t * task_alloc();
thread_t * thread_alloc(task_t * task);

void thread_setup_first(thread_t * thread, void * program, uint64_t len);
void thread_setup(thread_t * thread, void * rip);

/**
 * Should be called from a critical section.
 */
void task_add_thread(task_t * task, thread_t * thread);

