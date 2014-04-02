#include "string.h"

size_t strlen(const char * str) {
  size_t len = 0;
  while (str[len]) len++;
  return len;
}

int strcmp(const char * str1, const char * str2) {
  while ((*str1) != (*str2) && (*str1) && (*str2)) {
    str1++;
    str2++;
  }
  if ((*str1) == (*str2)) return 0;
  if ((*str1) > (*str2)) return 1;
  return -1;
}

int memcmp(const void * ptr1, const void * ptr2, size_t len) {
  size_t i;
  for (i = 0; i < len; i++) {
    uint8_t b1 = *((uint8_t *)(ptr1 + i));
    uint8_t b2 = *((uint8_t *)(ptr2 + i));
    if (b1 > b2) return 1;
    else if (b1 < b2) return -1;
  }
  return 0;
}

void * memcpy(void * _dest, const void * src, size_t len) {
  const uint8_t * source = src;
  uint8_t * dest = _dest;
  for (; len > 0; len--) {
    (*(dest++)) = *(source++);
  }
  return dest;
}
