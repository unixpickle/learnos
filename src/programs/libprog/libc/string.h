#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void * memcpy(void * dest, const void * src, uint64_t len);
char * strcpy(char * dest, const char * str);
size_t strlen(const char * str);
int strcmp(const char * str1, const char * str2);
int memcmp(const void * ptr1, const void * ptr2, size_t len);
uint64_t xtoi(const char * str);

