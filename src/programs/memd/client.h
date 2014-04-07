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
 * be created.
 */
client_t * client_get(uint64_t fd);

/**
 * Lookup an existing client via PID.
 */
client_t * client_find(uint64_t pid);

/**
 * Delete a client for a given file descriptor.
 */
void client_delete(client_t * client);

/**
 * Allocate pages starting from `start` and going `len` long in the task's
 * address space.
 * @return If the task is requesting something unreasonable, false; otherwise,
 * true.
 */
bool client_req_pages(client_t * cli, uint64_t start, uint64_t len);

/**
 * Deallocate pages.
 * @return If the task is attempting to free memory it does not own or that is
 * not at the end of its page space, false; otherwise, true.
 */
bool client_del_pages(client_t * cli, uint64_t start, uint64_t len);

