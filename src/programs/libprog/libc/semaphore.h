#include <pthread_cond.h>
#include <pthread_mutex.h>

typedef struct {
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  int64_t count;
} __attribute__((packed)) sem_t;

int sem_init(sem_t * sem, int pshared, unsigned int value);
int sem_destroy(sem_t * sem);

int sem_wait(sem_t * sem);
int sem_trywait(sem_t * sem);
int sem_post(sem_t * sem);
int sem_getvalue(sem_t * sem, int * val);

