#include <stdint.h>

#define IOAPIC_REG_VER 0x1
#define IOAPIC_IRQ_VECTORS {0xec, 0xe4, 0, 0x94, 0x8c, 0x84, 0x7c, 0x74, 0xd4, 0xcc, 0xc4, 0xbc, 0xb4, 0xac, 0xa4, 0x9c}

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

