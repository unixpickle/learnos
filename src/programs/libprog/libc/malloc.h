#include <stdint.h>
#include <stddef.h>

void free(void * buf);
void * malloc(size_t size);
int posix_memalign(void ** ptr, size_t align, size_t size);
void * realloc(void * ptr, size_t size);

