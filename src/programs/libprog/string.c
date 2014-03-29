#include "string.h"

void memcpy(void * dest, const void * src, uint64_t len) {
  uint8_t * _d = dest;
  const uint8_t * _s = src;
  while (len--) {
    (*(_d++)) = *(_s++);
  }
}

uint64_t strlen(const char * str) {
  uint64_t res = 0;
  while ((str++)[0]) res++;
  return res;
}

bool strequal(const char * str1, const char * str2) {
  while ((*str1) && (*str2)) {
    if ((*(str1++)) != (*(str2++))) return false;
  }
  return ((*str1) == (*str2));
}

uint64_t xtoi(const char * str) {
  uint64_t result = 0;
  while (*str) {
    char ch = *(str++);
    uint64_t place = 0;
    if (ch >= '0' && ch <= '9') {
      place = ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
      place = 10 + (ch - 'a');
    } else if (ch >= 'A' && ch <= 'F') {
      place = 10 + (ch - 'A');
    } else break;
    result <<= 4;
    result |= place;
  }
  return result;
}

