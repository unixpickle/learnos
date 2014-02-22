#include <shared/types.h>

void lapic_initialize();
bool lapic_is_available(); // check for x2APIC
void lapic_enable();

