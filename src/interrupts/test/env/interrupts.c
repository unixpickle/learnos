#include "interrupts.h"
#include "threading.h"
#include "context.h"
#include <anscheduler/loop.h>

static void _resign_continuation(void * unused);

void antest_handle_timer_interrupt() {
  thread_t * th = anscheduler_cpu_get_thread();
  if (!th) {
    _resign_continuation(NULL);
  } else {
    anscheduler_save_return_state(th, NULL, _resign_continuation);
  }
}

static void _resign_continuation(void * unused) {
  antest_get_current_cpu_info()->isLocked = true;
  anscheduler_loop_resign();
}
