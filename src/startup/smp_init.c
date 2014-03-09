#include <stdio.h>
#include <libkern_base.h>
#include <interrupts/lapic.h>
#include <interrupts/acpi.h>
#include <interrupts/pit.h>
#include <smp/cpu_config.h>
#include <smp/gdt.h>
#include <smp/scheduler.h>
#include <kernpage.h>

extern void GDT64_pointer();
extern void proc_entry();
extern void proc_entry_end();
extern void bootstrap_task();
extern void bootstrap_task_end();

static void copy_init_code();

static bool lapic_startup(void * unused, acpi_entry_lapic * entry);
static bool x2apic_startup(void * unused, acpi_entry_x2apic * entry);
static void initialize_cpu(uint32_t cpuId);

void smp_initialize() {
  tasks_initialize();
  gdt_initialize();
  copy_init_code();
  cpu_list_initialize(lapic_get_id());
  acpi_madt_iterate_type(0, NULL, (madt_iterator_t)lapic_startup);
  acpi_madt_iterate_type(9, NULL, (madt_iterator_t)x2apic_startup);

  uint64_t taskEnd = ((uint64_t)bootstrap_task_end);
  uint64_t taskStart = ((uint64_t)bootstrap_task);
  kernpage_lock();
  uint64_t used = kernpage_count_allocated();
  kernpage_unlock();
  scheduler_generate_task((void *)taskStart, taskEnd - taskStart);
  kernpage_lock();
  used = kernpage_count_allocated() - used;
  kernpage_unlock();
  print("scheduler_generate_task used ");
  printHex(used);
  print(" pages to generate the first task\n");
  task_loop();
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

  if (cpu_list_lookup(cpuId)) {
    print("[OK]\n");
    return;
  }

  lapic_clear_errors();
  lapic_send_ipi(cpuId, vector, 6, 1, 0);
  pit_sleep(20);

  if (cpu_list_lookup(cpuId)) print("[OK]\n");
  else print("[FAILED]\n");
}

