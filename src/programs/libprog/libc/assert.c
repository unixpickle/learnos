#include <stdio.h>
#include <stdlib.h>

void __assert(const char * msg, const char * file, int line) {
  printf("Assertion failure: \"%s\" in %s line %d\n", msg, file, line);
  abort();
}
