/**
 * The cpu_ functions helps manage the CPU list and access CPU specific fields.
 */

struct cpu_t {
  cpu_t * next; // linked list
  page_t baseStack;

  task_t * task;
  thread_t * thread;

  tss_t * tss;
  uint32_t cpuId;
  uint16_t tssSelector;

  // code which runs when syscall happens; should push rsp and rcx etc.
  uint8_t syscallCode[32];
} __attribute__((packed));

/**
 * Returns the number of CPUs on this system.
 */
uint64_t cpu_count();

/**
 * Adds a CPU to the CPU list. This does not lock anything, so this may not be
 * called after the CPU list has been populated and is being actively used.
 */
void cpu_add(cpu_t * cpu);

/**
 * Gets all info about the current CPU and adds it to the CPU list.
 */
void cpu_add_current(page_t stack);

/**
 * Finds a CPU in the CPU list with a given APIC ID.
 */
cpu_t * cpu_lookup(uint32_t ident);

/**
 * Finds the CPU in the list with the current local APIC ID.
 */
cpu_t * cpu_current();

/**
 * Returns the pointer to the highest part of the CPU's dedicated kernel stack.
 */
void * cpu_dedicated_stack();

/**
 * Notifies every CPU running a task to task-switch early.
 */
void cpu_notify_task_dead(task_t * task);

void anscheduler_cpu_lock();

void anscheduler_cpu_unlock();

task_t * anscheduler_cpu_get_task();

thread_t * anscheduler_cpu_get_thread();

void anscheduler_cpu_set_task(task_t * task);

void anscheduler_cpu_set_thread(thread_t * thread);

void anscheduler_cpu_notify_invlpg(task_t * task);

void anscheduler_cpu_notify_dead(task_t * task);

void anscheduler_cpu_stack_run(void * arg, void (* fn)(void *));

void anscheduler_cpu_halt();

