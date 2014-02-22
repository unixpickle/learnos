#include <shared/types.h>

#define LAPIC_REG_APICID 0x20
#define LAPIC_REG_APICVER 0x30
#define LAPIC_REG_TASKPRIOR 0x80
#define LAPIC_REG_EOI 0xb0
#define LAPIC_REG_LDR 0xd0
#define LAPIC_REG_DFR 0xe0
#define LAPIC_REG_SPURIOUS 0xf0
#define LAPIC_REG_ESR 0x280
#define LAPIC_REG_ICRL 0x300
#define LAPIC_REG_ICRH 0x310
#define LAPIC_REG_LVT_TMR 0x320
#define LAPIC_REG_LVT_PERF 0x340
#define LAPIC_REG_LVT_LINT0 0x350
#define LAPIC_REG_LVT_LINT1 0x360
#define LAPIC_REG_LVT_ERR 0x370
#define LAPIC_REG_TMRINITCNT 0x380
#define LAPIC_REG_TMRCURRCNT 0x390
#define LAPIC_REG_TMRDIV 0x3e0
#define LAPIC_REG_LAST 0x38f
#define LAPIC_REG_SW_ENABLE 0x100
#define LAPIC_REG_CPUFOCUS 0x200

#define LAPIC_SETTING_DISABLE 0x1000
#define LAPIC_SETTING_NMI (4 << 8)
#define LAPIC_SETTING_TMR_PERIODIC 0x20000
#define LAPIC_SETTING_TMR_BASEDIV (1 << 20)

void lapic_initialize();
bool lapic_is_available();
bool lapic_is_x2_available();
void lapic_enable();
void lapic_send_eoi();

