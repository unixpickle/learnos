#include <stdint.h>

#define IOAPIC_REG_VER 0x1

typedef struct {
  unsigned vector : 8; // RW - processor register
  unsigned delmode : 3; // RW
  unsigned destmode : 1; // RW - determines type for destfield
  unsigned delstatus : 1; // RO
  unsigned intpol : 1; // RW - 0 = high active, 1 = low active
  unsigned remirr : 1; // RO
  unsigned triggermode : 1; // 1 = level sensitive, 0 = edge sensitive
  unsigned imask : 1; // 1 = prevent this interrupt
  unsigned long long reserved : 39; // set this to 0
  unsigned destfield : 8; // RW - APIC ID or "set of processors"
} __attribute__((packed)) ioapic_redirection;

void ioapic_initialize();

void ioapic_write_reg(uint8_t reg, uint32_t val);
uint32_t ioapic_read_reg(uint8_t reg);

uint32_t ioapic_get_version();
uint32_t ioapic_get_pin_count();
void ioapic_set_red_table(uint8_t index, ioapic_redirection entry);

