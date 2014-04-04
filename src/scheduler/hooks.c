#include "code.h"

void anscheduler_task_cleanup(task_t * task) {
  code_task_cleanup(task->ui.code, task);
}

