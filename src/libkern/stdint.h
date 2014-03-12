#ifndef __LIBKERN_STDINT_H__
#define __LIBKERN_STDINT_H__

// these are standard for Linux, Mac, and Windows
typedef unsigned char bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint64_t page_t;

// these are just nice to have for verbosity
#define true 1
#define false 0
#define NULL (void *)0

#endif
