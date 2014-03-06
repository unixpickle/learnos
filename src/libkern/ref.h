#ifndef __LIBKERN_REF_H__
#define __LIBKERN_REF_H__

#include <stdint.h>

typedef struct {
  uint64_t count;
  void (* dealloc)(void * ptr);
} __attribute__((packed)) ref_obj_t;

void ref_initialize(void * ptr, void (*dealloc)(void * ptr));
void ref_release(void * ptr);
uint64_t ref_retain(void * ptr);
void ref_dealloc(void * ptr);

#endif
