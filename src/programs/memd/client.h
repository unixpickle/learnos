#include <stdlib.h>
#include <stdbool.h>

/**
 * Represents a connected memd client.
 */
typedef struct {
  uint64_t fd;
  uint64_t pid;

  uint64_t pageCount;
  uint64_t * pages;
} __attribute__((packed)) client_t;

/**
 * Lookup a client for a given file descriptor. If one does not exist, it will
 * be created. May return NULL on certain race conditions.
 */
client_t * client_get(uint64_t fd);

/**
 * Lookup an existing client via PID.
 */
client_t * client_find(uint64_t pid);

/**
 * Delete a client.
 */
void client_delete(client_t * client);

