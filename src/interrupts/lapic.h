#include <stdint.h>

#define LAPIC_REG_APICID 0x2
#define LAPIC_REG_APICVER 0x3
#define LAPIC_REG_TASKPRIOR 0x8
#define LAPIC_REG_EOI 0xb
#define LAPIC_REG_LDR 0xd
#define LAPIC_REG_DFR 0xe
#define LAPIC_REG_SPURIOUS 0xf
#define LAPIC_REG_ESR 0x28
#define LAPIC_REG_ICRL 0x30
#define LAPIC_REG_ICRH 0x31
#define LAPIC_REG_LVT_TMR 0x32
#define LAPIC_REG_LVT_PERF 0x34
#define LAPIC_REG_LVT_LINT0 0x35
#define LAPIC_REG_LVT_LINT1 0x36
#define LAPIC_REG_LVT_ERR 0x37
#define LAPIC_REG_TMRINITCNT 0x38
#define LAPIC_REG_TMRCURRCNT 0x39
#define LAPIC_REG_TMRDIV 0x3e

#define LAPIC_SETTING_LAST 0x38f
#define LAPIC_SETTING_SW_ENABLE 0x100
#define LAPIC_SETTING_CPUFOCUS 0x200
#define LAPIC_SETTING_DISABLE 0x1000
#define LAPIC_SETTING_NMI (1 << 10)
#define LAPIC_SETTING_TMR_PERIODIC 0x20000
#define LAPIC_SETTING_TMR_BASEDIV (1 << 20)

void lapic_initialize();
void lapic_set_defaults();
bool lapic_is_available();
bool lapic_is_x2_available();
void lapic_enable();
uint32_t lapic_get_id();
void lapic_clear_errors();

uint64_t lapic_calculate_bus_speed();
uint64_t lapic_get_bus_speed();

void lapic_send_eoi();
void lapic_set_priority(uint8_t pri);
void lapic_set_register(uint16_t reg, uint64_t value);
uint64_t lapic_get_register(uint16_t reg);
void lapic_send_ipi(uint32_t cpu,
                    uint8_t vector,
                    uint8_t mode,
                    uint8_t level,
                    uint8_t trigger);
void lapic_timer_set(uint8_t vector, uint32_t count, uint32_t div);

