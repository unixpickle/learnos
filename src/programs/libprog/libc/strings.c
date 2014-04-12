#include <strings.h>

void bzero(void * buf, size_t size) {
  char * ptr = (char *)buf;
  while (size--) *(ptr++) = 0;
}

