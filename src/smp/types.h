#ifndef __TASK_TYPES_H__
#define __TASK_TYPES_H__

#include "queue.h"
#include <anidxset.h>

typedef struct task_t task_t;
typedef struct thread_t thread_t;
typedef struct cpu_t cpu_t;

struct task_t {
  task_t * nextTask; // for linked list

  uint64_t pid;
  uint64_t uid;

  uint64_t pml4Lock;
  page_t pml4;

  uint64_t threadsLock;
  thread_t * firstThread;

  uint64_t stacksLock;
  anidxset_root_t stacks;

  uint64_t socketsLock;
  socket_t * firstSocket;

  uint64_t descriptorsLock;
  anidxset_root_t descriptors;

  bool isActive;
} __attribute__((packed));

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

  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
} __attribute__((packed)) state_t;

struct thread_t {
  thread_t * nextThread; // for linked list

  uint64_t stack; // index from 0 to 0xfffff
  state_t state;

  task_t * task;

  uint64_t statusLock;
  uint8_t status;
  bool isSystem;
  uint8_t reserved[6];

  uint64_t interruptMask;
  uint64_t nextTimestamp; // 0 = not waiting on timer

  thread_t * queueNext;
  thread_t * queueLast;
}

struct cpu_t {
  cpu_t * next; // linked list
  page_t baseStack;

  task_t * task;
  thread_t * thread;

  uint64_t lastTimeout; // last LAPIC timer period setting
  
  tss_t * tss;
  uint32_t cpuId;
  uint16_t tssSelector;
} __attribute__((packed));

#endif
