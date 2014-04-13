#include <pthread.h>

int * __errno() {
  return &pthread_current()->errno;
}

