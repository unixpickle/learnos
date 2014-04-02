#ifndef __LIBKERN_STRING_H__
#define __LIBKERN_STRING_H__

#include <stdint.h>
#include <stddef.h>

size_t strlen(const char * str);
int strcmp(const char * str1, const char * str2);
int memcmp(const void * ptr1, const void * ptr2, size_t len);
void * memcpy(void * dest, const void * src, size_t len);

#endif
