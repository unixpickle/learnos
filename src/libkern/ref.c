#include "ref.h"

void ref_initialize(void * ptr, void (*dealloc)(void * ptr)) {
  ref_obj_t * obj = (ref_obj_t *)ptr;
  obj->count = 1;
  obj->dealloc = dealloc;
}

void ref_release(void * ptr) {
  ref_obj_t * obj = (ref_obj_t *)ptr;
  if (!__sync_sub_and_fetch(&obj->count, 1)) {
    ref_dealloc(ptr);
  }
}

uint64_t ref_retain(void * ptr) {
  ref_obj_t * obj = (ref_obj_t *)ptr;
  return __sync_add_and_fetch(&obj->count, 1);
}

void ref_dealloc(void * ptr) {
  ref_obj_t * obj = (ref_obj_t *)ptr;
  obj->dealloc(ptr);
}

