#include "client.h"
#include <stdlib.h>
#include <assert.h>

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
  cli->pageCount = 0;
  cli->pages = NULL;
  return cli;
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

bool client_req_pages(client_t * cli, uint64_t start, uint64_t len) {
  // TODO: enforce some sort of limits here
  if (start != cli->pageCount) return false;

  // allocate room for more pages from page faults
  cli->pageCount += len;
  cli->pages = (uint64_t *)realloc(cli->pages, 8 * cli->pageCount);
  assert(cli->pages != NULL);
  uint64_t i;
  for (i = start; i < start + len; i++) {
    cli->pages[i] = 0;
  }
  return true;
}

bool client_del_pages(client_t * cli, uint64_t start, uint64_t len) {
  // TODO: enforce some sort of limits here
  if (start + len != cli->pageCount) return false;

  // deallocate some pages
  cli->pageCount -= len;
  cli->pages = (uint64_t *)realloc(cli->pages, 8 * cli->pageCount);

  // TODO: here, unmap the pages
  return true;
}

