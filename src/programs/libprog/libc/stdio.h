#include <base/system.h>

void printf(const char * str, ...);
void printf_impl(const uint64_t * arguments);
void print_argument(char type, uint64_t arg);
void print_hex(uint64_t arg, bool caps);

