#include <stdio.h>
#include <libkern_base.h>
#include <interrupts/lapic.h>
#include <interrupts/acpi.h>
#include <interrupts/pit.h>
#include <interrupts/basic.h>
#include <smp/cpu.h>
#include <smp/gdt.h>
#include <smp/scheduler.h>
#include <smp/creation.h>
#include <smp/tasks.h>
#include <kernpage.h>

extern void GDT64_pointer();
extern void proc_entry();
extern void proc_entry_end();
extern void _binary_keyboard_build_keyboard_bin_start();
extern void _binary_keyboard_build_keyboard_bin_end();
extern void _binary_ticktock_build_ticktock_bin_start();
extern void _binary_ticktock_build_ticktock_bin_end();
extern void load_new_gdt();

static void copy_init_code();

static bool lapic_startup(void * unused, acpi_entry_lapic * entry);
static bool x2apic_startup(void * unused, acpi_entry_x2apic * entry);
static void initialize_cpu(uint32_t cpuId);

void proc_initialize(page_t stack);
void load_tss();

void smp_initialize() {
  print("initializing basic scheduling structures...\n");
  pids_initialize();
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

  print("starting bootstrap tasks...\n");

  disable_interrupts();

  uint64_t taskEnd = (uint64_t)(_binary_keyboard_build_keyboard_bin_end);
  uint64_t taskStart = (uint64_t)(_binary_keyboard_build_keyboard_bin_start);
  task_generate((void *)taskStart, taskEnd - taskStart);

  taskEnd = (uint64_t)(_binary_ticktock_build_ticktock_bin_end);
  taskStart = (uint64_t)(_binary_ticktock_build_ticktock_bin_start);
  task_generate((void *)taskStart, taskEnd - taskStart);

  load_new_gdt();
  load_tss();
  scheduler_task_loop();
}

void proc_initialize(page_t stack) {
  lapic_enable();
  lapic_set_defaults();
  lapic_set_priority(0);
  load_idtr((void *)IDTR_PTR);
  cpu_add_current(stack);
  load_new_gdt();
  load_tss();
  scheduler_task_loop();
}

void load_tss() {
  cpu_t * cpu = cpu_current();
  uint16_t currentTss;
  __asm__ ("str %0" : "=r" (currentTss));
  if (currentTss != cpu->tssSelector) {
    __asm__ ("ltr %0" : : "r" (cpu->tssSelector));
  }
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
