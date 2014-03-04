#include <anlock.h>
#include <stdint.h>

typedef struct {
  page_t firstTask;
  page_t nextTask; // 0 means start over
  uint64_t nextPID;
  uint64_t listLock;
} __attribute__((packed)) task_list_t;

typedef struct {
  uint64_t retainCount;
  uint64_t pid;
  uint64_t uid;
  page_t nextTask; // for task_list_t

  page_t pml4; // physical page of PML4
  uint64_t pml4Lock;

  page_t firstThread;
  uint64_t threadsLock;

  page_t firstSocket;
  uint64_t socketsLock;

  bool isCleanup;
} __attribute__((packed)) task_t;

typedef struct {
  uint64_t rsp;
  uint64_t rbp;
  uint64_t cr3;
  uint64_t rip;
  uint64_t flags;
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
} __attribute__((packed)) state_t;

typedef struct {
  uint64_t retainCount;
  page_t task;
  page_t nextThread;

  uint64_t stackIndex;
  state_t state;
  uint8_t isRunning;
} __attribute__((packed)) thread_t;

/**
 * Adds a thread to a task.
 */
void task_add_thread(page_t task, void * rip);

/**
 * Starts a task pointed to with a page index.
 */
void task_start(page_t task);

/**
 * Removes a task from the task list, reducing it's retain count and the retain
 * counts of all its threads.
 */
void task_term(page_t task);

/**
 * @return a retained task or 0 if not found.
 * @discussion This function locks the task list for you, so you don't
 * need to worry about it.
 */
page_t task_find(uint64_t pid);

/**
 * Releases a task. This operation is atomic and may result
 * in the task's entire resources being released.
 * When a task is finally at retain count 0, a new task is
 * created which specifically exists to clean it up. This new
 * task takes place in the same address space for ease of use.
 */
void task_release(page_t task);

/**
 * Release a thread. This may result in the thread being cleaned up
 * and furthermore in the owning task being deleted.
 */
void thread_release(page_t thread);

/**
 * Uses a task's page tables to find the physical page that a virtual page in
 * the tasks address space points to.
 */
page_t task_page_lookup(page_t task, page_t virt);

/**
 * Maps a virtual page to a physical page.
 */
bool task_page_map(page_t task, page_t virt, page_t phys, bool user);

/**
 * Zero's out a page's entry in the page table. This may additionally free
 * various page tables if possible.
 */
void task_page_unmap(page_t task, page_t virt);

/**
 * Notifies all running instances of the process that the page tables have
 * changed and the TLB should be cleared.
 */
void task_pages_changed();

