#include "queue.h"
#include <anlock.h>

static task_queue_t queue;

void task_queue_initialize() {
  queue.first = NULL;
  queue.last = NULL;
  anlock_initialize(&queue.lock);
}

void task_queue_lock() {
  anlock_lock(&queue.lock);
}

void task_queue_unlock() {
  anlock_unlock(&queue.lock);
}

void task_queue_push(task_queue_item_t * item) {
  item->next = NULL;
  item->last = queue.last;
  if (queue.last) {
    queue.last->next = item;
  } else {
    queue.first = item;
  }
  queue.last = item;
}

void task_queue_push_first(task_queue_item_t * item) {
  item->last = NULL;
  item->next = queue.first;
  if (queue.first) queue.first->last = item;
  queue.first = item;
}

task_queue_item_t * task_queue_pop() {
  task_queue_item_t * first = queue.first;
  queue.first = first->next;
  if (queue.first) {
    queue.first->last = NULL;
  } else {
    queue.last = NULL;
  }
  return first;
}

void task_queue_remove(task_queue_item_t * item) {
  if (item->last) {
    item->last->next = item->next;
  } else {
    queue.first = item->last;
  }
  if (item->next) {
    item->next->last = item->last;
  } else {
    queue.last = item->next;
  }
}

