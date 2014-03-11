#include "cpu_list.h"

void configure_cpu(uint64_t page);
void task_loop();
void smp_entry(uint64_t page); // calls both configure_cpu() and task_loop()

