#include "idt.h"
#include "lapic.h"
#include "basic.h"
#include "inttable.h"
#include <debug.h>
#include <shared/addresses.h> // for PIT
#include <stdio.h>
#include <anscheduler/interrupts.h>
#include <anscheduler/functions.h>
#include <scheduler/interrupts.h>
#include <kernpage.h>
#include <scheduler/cpu.h>
#include <anlock.h>

static void _initialize_idt(idt_entry_t * ptr);
static void _call_page_fault(uint64_t args);
static void _idt_continuation(uint64_t irq);

static bool schedEnabled = false;
static void * unknownLowerHalf = NULL;
static void * unknownUpperHalf = NULL;

void configure_dummy_idt() {
  idt_pointer * idtr = (idt_pointer *)IDTR_PTR;
  idtr->limit = 0x1000 - 1;
  idtr->virtualAddress = IDT_PTR;

  // create entries for low IRQs
  idt_entry_t * table = (idt_entry_t *)IDT_PTR;
  _initialize_idt((idt_entry_t *)table);

  idt_entry_t lowerEntry = IDT_ENTRY_INIT(((uint64_t)handle_dummy_lower), 0x8e);
  idt_entry_t upperEntry = IDT_ENTRY_INIT(((uint64_t)handle_dummy_upper), 0x8e);

  int i;
  for (i = 0x8; i <= 0xf; i++) {
    table[i] = lowerEntry;
  }
  for (i = 0x70; i <= 0x78; i++) {
    table[i] = upperEntry;
  }

  load_idtr((void *)idtr);
}

void configure_global_idt() {
  volatile idt_pointer * idtr = (volatile idt_pointer *)IDTR_PTR;
  idtr->limit = 0x1000 - 1;
  idtr->virtualAddress = IDT_PTR;

  idt_entry_t * table = (idt_entry_t *)IDT_PTR;
  _initialize_idt((idt_entry_t *)table);

  // setup the interrupt handlers
  uint64_t callRoutines[4] = {
    (uint64_t)&handle_interrupt_exception,
    (uint64_t)&handle_interrupt_exception_code,
    (uint64_t)&handle_interrupt_irq,
    (uint64_t)&handle_interrupt_ipi,
  };

  int i;
  for (i = 0; i < sizeof(gIDTHandlers) / sizeof(int_handler_t); i++) {
    uint64_t handler = (uint64_t)&gIDTHandlers[i];
    uint64_t flags = 0x8e;

    // set the correct method
    gIDTHandlers[i].function = callRoutines[gIDTHandlers[i].function];
    
    idt_entry_t entry = IDT_ENTRY_INIT(handler, flags);
    table[gIDTHandlers[i].argument] = entry;
  }

  load_idtr((void *)idtr);
}

void int_interrupt_exception(uint64_t vec) {
  ensure_critical();
  uint64_t retAddr;
  __asm__("mov 0x98(%%rbp), %0" : "=r" (retAddr));
  print("Got exception vector ");
  printHex(vec);
  print(" from 0x");
  printHex(retAddr);
  print(" with thread 0x");
  printHex((uint64_t)anscheduler_cpu_get_thread());
  print("\n");

  __asm__("cli\nhlt");
  // TODO: here, save task state and terminate it
}

void int_interrupt_exception_code(uint64_t vec, uint64_t code) {
  ensure_critical();
  uint64_t retAddr;
  __asm__("mov 0xa0(%%rbp), %0" : "=r" (retAddr));
  if (vec == 0xe) {
    thread_t * thread = anscheduler_cpu_get_thread();
    if (thread) {
      anscheduler_save_return_state(thread, (void *)code,
                                    (void (*)(void *))_call_page_fault);
      return;
    }
  }
  print("Got exception vector ");
  printHex(vec);
  print(" with code ");
  printHex(code);
  print(" from ");
  printHex(retAddr);
  print(" with thread 0x");
  printHex((uint64_t)anscheduler_cpu_get_thread());
  print("\n");
  __asm__("cli\nhlt");
  // TODO: here, save task state and terminate it, or do a page fault
}

void int_interrupt_irq(uint64_t vec) {
  ensure_critical();
  if (lapic_is_in_service(vec)) {
    lapic_send_eoi();
  }
  if (vec == 0x20 || vec == 0x22) {
    PIT_TICK_COUNT++;
  } else if (vec == 0x30) {
    schedEnabled = true;
    handle_lapic_interrupt();
  } else if (vec >= 0x20 && vec < 0x30 && schedEnabled) {
    thread_t * thread = anscheduler_cpu_get_thread();
    uint64_t irq = vec - 0x20;
    if (!thread) {
      _idt_continuation(irq);
    } else {
      anscheduler_save_return_state(thread, (void *)irq,
                                    (void (*)(void *))_idt_continuation);
    }
  }
}

void int_interrupt_ipi(uint64_t vec) {
  if (vec == 0x31) {
    print("GOT PAGE IPI\n");
  }
  if (lapic_is_in_service(vec)) {
    lapic_send_eoi();
  }
}

void int_interrupt_unknown(uint64_t vec) {
  /*
  uint64_t retAddr1, retAddr2;
  __asm__("mov 0x98(%%rbp), %0" : "=r" (retAddr1));
  __asm__("mov 0xa0(%%rbp), %0" : "=r" (retAddr2));
  anlock_lock(&unknownLock);
  print("UNKNOWN INTERRUPT ");
  printHex(vec);
  print(" task ");
  printHex((uint64_t)anscheduler_cpu_get_task());
  print(" thread ");
  printHex((uint64_t)anscheduler_cpu_get_thread());
  print(" cpu ");
  printHex(cpu_current()->cpuId);
  print(" ret1=0x");
  printHex(retAddr1);
  print(" ret2=0x");
  printHex(retAddr2);
  print(" is ISR ");
  printHex((uint64_t)lapic_is_in_service(vec));
  print("\n");
  anlock_unlock(&unknownLock);
  */
  if (lapic_is_in_service(vec)) {
    lapic_send_eoi();
  }
}

static void _initialize_idt(idt_entry_t * ptr) {
  int i;
  if (!unknownLowerHalf) {
    unknownLowerHalf = (void *)(kernpage_alloc_virtual() << 12L);
    unknownUpperHalf = (void *)(kernpage_alloc_virtual() << 12L);
    for (i = 0; i < 0x100; i++) {
      void * codePtr = i < 0x80 ? unknownLowerHalf + (i << 5L)
                                : unknownUpperHalf + ((i - 0x80L) << 5L);
      int_handler_t * handler = codePtr;
      int_handler_t good = INT_HANDLER_INIT(0, 0);
      (*handler) = good;
      handler->function = (uint64_t)handle_interrupt_unknown;
      handler->argument = i;
    }
  }
  for (i = 0; i < 0x100; i++) {
    void * codePtr = i < 0x80 ? unknownLowerHalf + (i << 5)
                              : unknownUpperHalf + ((i - 0x80) << 5);
    idt_entry_t entry = IDT_ENTRY_INIT((uint64_t)codePtr, 0x8e);
    ptr[i] = entry;
  }
}

static void _call_page_fault(uint64_t args) {
  uint64_t addr;
  __asm__("mov %%cr2, %0" : "=r" (addr));
  anscheduler_page_fault((void *)addr, args);
}

static void _idt_continuation(uint64_t irq) {
  anscheduler_irq(irq);
  if (anscheduler_cpu_get_thread()) {
    anscheduler_thread_run(anscheduler_cpu_get_task(),
                           anscheduler_cpu_get_thread());
  } else {
    return;
  }
}

