#include <stdio.h>
#include <libkern_base.h>
#include <kernpage.h>

#include <interrupts/lapic.h>
#include <interrupts/acpi.h>
#include <interrupts/pit.h>
#include <interrupts/basic.h>

#include <scheduler/cpu.h>
#include <anscheduler/loop.h>
#include <anscheduler/task.h>
#include <anscheduler/thread.h>
#include <syscall/config.h>

#include "proc_init.h"

extern void GDT64_pointer();
extern void proc_entry();
extern void proc_entry_end();
extern void _binary_keyboard_build_keyboard_bin_start();
extern void _binary_keyboard_build_keyboard_bin_end();
extern void _binary_ticktock_build_ticktock_bin_start();
extern void _binary_ticktock_build_ticktock_bin_end();

static void copy_init_code();
static void initialize_cpu(uint32_t lapicId);

static bool lapic_startup(void * unused, acpi_entry_lapic * entry);
static bool x2apic_startup(void * unused, acpi_entry_x2apic * entry);

static void start_task(void * ptr, uint64_t len);

void smp_initialize() {
  print("initializing basic scheduling structures...\n");
  print_initialize();
  gdt_initialize();
  copy_init_code();

  page_t page = kernpage_alloc_virtual();
  if (!page) die("failed to allocate kernel stack");
  cpu_add_current(page);

  print("initializing xAPIC's...\n");
  acpi_madt_iterate_type(0, NULL, (madt_iterator_t)lapic_startup);
  print("initializing x2APIC's...\n");
  acpi_madt_iterate_type(9, NULL, (madt_iterator_t)x2apic_startup);

  disable_interrupts();
  print("starting bootstrap tasks...\n");

  uint64_t taskEnd = (uint64_t)(_binary_keyboard_build_keyboard_bin_end);
  uint64_t taskStart = (uint64_t)(_binary_keyboard_build_keyboard_bin_start);
  start_task((void *)taskStart, taskEnd - taskStart);

  taskEnd = (uint64_t)(_binary_ticktock_build_ticktock_bin_end);
  taskStart = (uint64_t)(_binary_ticktock_build_ticktock_bin_start);
  // start_task((void *)taskStart, taskEnd - taskStart);

  proc_run_scheduler();
}

static void copy_init_code() {
  uint64_t len = (uint64_t)proc_entry_end - (uint64_t)proc_entry;
  uint64_t i;
  uint8_t * source = (uint8_t *)proc_entry;
  uint8_t * dest = (uint8_t *)PROC_INIT_PTR;
  print("startup code is ");
  printHex(len);
  print(" bytes.\n");
  for (i = 0; i < len; i++) {
    dest[i] = source[i];
  }
}

static bool lapic_startup(void * unused, acpi_entry_lapic * entry) {
  if (!(entry->flags & 1)) {
    print("There was a disabled CPU with ID ");
    printHex(entry->apicId);
    print("\n");
  }
  if (!(entry->flags & 1)) return 1;
  initialize_cpu(entry->apicId);
  return 1;
}

static bool x2apic_startup(void * unused, acpi_entry_x2apic * entry) {
  if (!(entry->flags & 1)) return 1;
  initialize_cpu(entry->x2apicId);
  return 1;
}

static void initialize_cpu(uint32_t cpuId) {
  if (cpuId == lapic_get_id()) return;
  print("initializing APIC with ID 0x");
  printHex(cpuId);
  print("... ");

  lapic_clear_errors();
  // send the INIT IPI with trigger=level and mode=0b101
  lapic_send_ipi(cpuId, 0, 5, 1, 1); // assert the IPI
  pit_sleep(1);
  lapic_send_ipi(cpuId, 0, 5, 0, 1); // de-assert the IPI
  pit_sleep(1);

  uint8_t vector = (uint8_t)(PROC_INIT_PTR >> 12);

  // send the STARTUP IPI with trigger=edge and delivery mode=110
  lapic_clear_errors();
  lapic_send_ipi(cpuId, vector, 6, 1, 0);
  pit_sleep(20);

  if (cpu_lookup(cpuId)) {
    print("[OK]\n");
    return;
  }

  lapic_clear_errors();
  lapic_send_ipi(cpuId, vector, 6, 1, 0);
  pit_sleep(20);

  if (cpu_lookup(cpuId)) print("[OK]\n");
  else print("[FAILED]\n");
}

static void start_task(void * ptr, uint64_t len) {
  task_t * task = anscheduler_task_create(ptr, len);
  anscheduler_task_launch(task);


  thread_t * thread = anscheduler_thread_create(task);
  uint64_t stackStart = ANSCHEDULER_TASK_USER_STACKS_PAGE
    + (thread->stack << 8);

  thread->state.cr3 = (uint64_t)task->vm;
  thread->state.rsp = (stackStart + 0x100) << 12;
  thread->state.rbp = (stackStart + 0x100) << 12;
  thread->state.flags = 0x200;
  thread->state.rip = ANSCHEDULER_TASK_CODE_PAGE << 12;
  thread->state.cs = 0x1b;
  thread->state.ss = 0x23;
  syscall_initialize_thread(thread);

  anscheduler_thread_add(task, thread);
  anscheduler_task_dereference(task);
}

