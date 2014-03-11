#include "ref.h"

void ref_initialize(void * ptr, void (*dealloc)(void * ptr)) {
  ref_obj_t * obj = (ref_obj_t *)ptr;
  obj->count = 1;
  obj->dealloc = dealloc;
}

void ref_release(void * ptr) {
  if (!ptr) return;
  ref_obj_t * obj = (ref_obj_t *)ptr;
  if (!__sync_sub_and_fetch(&obj->count, 1)) {
    ref_dealloc(ptr);
  }
}

void * ref_retain(void * ptr) {
  if (!ptr) return NULL;
  ref_obj_t * obj = (ref_obj_t *)ptr;
  __sync_add_and_fetch(&obj->count, 1);
  return ptr;
}

void ref_dealloc(void * ptr) {
  if (!ptr) return;
  ref_obj_t * obj = (ref_obj_t *)ptr;
  obj->dealloc(ptr);
}

