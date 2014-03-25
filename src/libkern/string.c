#include "string.h"

uint64_t strlen(const char * str) {
  uint64_t len = 0;
  while (str[len]) len++;
  return len;
}

bool strequal(const char * str1, const char * str2) {
  uint64_t i, l1 = strlen(str1);
  if (strlen(str2) != strlen(str1)) return false;
  for (i = 0; i < l1; i++) {
    if (str1[i] != str2[i]) return false;
  }
  return true;
}

bool memequal(const void * b1, const void * b2, uint64_t len) {
  const uint8_t * a = (const uint8_t *)b1;
  const uint8_t * b = (const uint8_t *)b2;
  while (len--) {
    if (a[0] != b[0]) return false;
    a++;
    b++;
  }
  return true;
}

void memcpy(void * _dest, const void * src, uint64_t len) {
  const uint8_t * source = src;
  uint8_t * dest = _dest;
  for (; len > 0; len--) {
    (*dest) = *source;
  }
}

