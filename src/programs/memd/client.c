#include "client.h"
#include <stdlib.h>
#include <assert.h>
#include <base/system.h>

static client_t * clients = NULL;
static uint64_t clientCount = 0;

client_t * client_get(uint64_t fd) {
  uint64_t i;
  for (i = 0; i < clientCount; i++) {
    if (clients[i].fd == fd) {
      return &clients[i];
    }
  }
  clientCount++;
  clients = realloc(clients, sizeof(client_t) * clientCount);
  client_t * cli = &clients[clientCount - 1];
  cli->fd = fd;
  cli->pid = sys_remote_pid(fd);
  cli->pageCount = 0;
  cli->pages = NULL;
  return cli;
}

client_t * client_find(uint64_t pid) {
  uint64_t i;
  for (i = 0; i < clientCount; i++) {
    if (clients[i].pid == pid) {
      return &clients[i];
    }
  }
  return false;
}

void client_delete(client_t * client) {
  clientCount--;
  if (!clientCount) {
    free(clients);
    clients = NULL;
    return;
  }

  uint64_t index = (((uint64_t)client) - ((uint64_t)clients))
    / sizeof(client_t);
  uint64_t i;
  for (i = 0; i <= clientCount; i++) {
    if (i > index) clients[i - 1] = clients[i];
  }
  clients = realloc(clients, sizeof(client_t) * clientCount);
  assert(clients != NULL);
}

