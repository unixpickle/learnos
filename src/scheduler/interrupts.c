#include "interrupts.h"
#include <anscheduler/functions.h>
#include <anscheduler/loop.h>

static void _lapic_timer_stub(void * unused);

void handle_lapic_interrupt() {
  anscheduler_timer_set_far();
  thread_t * thread = anscheduler_cpu_get_thread();
  if (thread) {
    anscheduler_save_return_state(thread, NULL, _lapic_timer_stub);
  } else {
    _lapic_timer_stub(NULL);
  }
}

static void _lapic_timer_stub(void * unused) {
  anscheduler_loop_resign();
}
