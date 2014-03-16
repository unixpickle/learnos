#include "types.h"

void pids_initialize();
uint64_t pids_next();
void pids_release(uint64_t pid);

void tasks_lock();
void tasks_add(task_t * task);
void tasks_remove(task_t * task);
task_t * tasks_find(uint64_t pid);
void tasks_iterate(uint64_t value, void (* iterator)(uint64_t v, task_t * t));
void tasks_unlock();

