#ifndef __TASK_H__
#define __TASK_H__

#include <ref.h>
#include <anidxset.h>
#include "socket.h"

typedef struct task_t task_t;
typedef struct thread_t thread_t;

typedef struct {
  uint64_t lock;
  task_t * firstTask; // strong
  task_t * nextTask; // strong

  uint64_t pidsLock;
  anidxset_root_t pids;
} __attribute__((packed)) tasks_root_t;

struct task_t {
  ref_obj_t ref;

  // each link has a strong reference to the next node
  task_t * nextTask; // strong

  // process identifier and user identifier, just like UNIX bro
  uint64_t pid;
  uint64_t uid;

  // every process has its own page table setup
  uint64_t pml4Lock;
  page_t pml4; // physical page of PML4

  // this lock applies to both ...Thread fields
  uint64_t threadsLock;
  thread_t * firstThread; // strong
  thread_t * nextThread; // strong
  uint64_t threadStacksLock;
  anidxset_root_t threadStacks;

  uint64_t firstSocketLock;
  socket_t * firstSocket; // strong
  uint64_t socketDescsLock;
  anidxset_root_t socketDescs; // like file descriptors, bro

  // if this is 0, then the process is being shut-down
  bool isActive;
} __attribute__((packed));

struct state_t {
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
} __attribute__((packed));

struct thread_t {
  ref_obj_t ref;

  thread_t * nextThread;

  uint64_t stackIndex;
  struct state_t state;

  task_t * containingTask; // strong
  uint8_t isRunning;
  uint8_t isSystem; // if 1, this thread can't be killed
} __attribute__((packed));

void * task_idxset_alloc();
void task_idxset_free(void * ptr);

void * task_idxset_alloc_safe();
void task_idxset_free_safe(void * ptr);

/**
 * Whenever the kernel thread of a process holds any locks, it must be in a
 * critical section. This means that the CPU core will not start to execute
 * another task while the lock is held.
 */
void task_critical_start();

/**
 * Exits a critical section.
 */
void task_critical_stop();

/**
 * Initialize the task subsystem. This should only be called once at boot and
 * never again.
 */
void tasks_initialize();

/**
 * Allocates a new task with base resources.
 * @discussion This function must be called from a critical section.
 */
task_t * task_create();

/**
 * Adds a task to the task list. This function does not consume the reference
 * to task, so you must do that later.
 * @discussion You know the drill, call from a critical section.
 */
void task_list_add(task_t * task);

/**
 * Deallocates the *critical* resources of the task. This should only
 * be called once the task has finished its shutdown sequence.
 * Really, this should only be called when the task's retain count reaches 0.
 * @discussion Just like task_create, you must call this from a critical
 * section.
 */
void task_dealloc(task_t * task);

/**
 * Uses the task list to find the next task and thread to execute.
 * @discussion This must be called from a critical section.
 */
bool task_get_next_job(task_t ** task, thread_t ** thread);

#endif

