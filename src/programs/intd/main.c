#include <stdio.h>
#include <msgd.h>
#include <string.h>

typedef struct {
  uint64_t fd;
  uint64_t mask;
  uint64_t waiting;
} __attribute__((packed)) client_t;

void handle_fd(uint64_t fd);
client_t * client_lookup(uint64_t fd);
client_t * client_add();
void client_remove(client_t * client);

static uint64_t clientCount = 0;
static client_t * clients = NULL;

int main() {
  client_t _clients[0x40];
  clients = _clients;

  sys_wants_interrupts();

  // connect to name server
  uint64_t fd = msgd_connect();
  if (!(fd + 1)) {
    printf("[intd]: error: could not connect to msgd\n");
    return 1;
  }
  msgd_register_service(fd, "intd");

  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_fd(fd);
    uint64_t ints = sys_get_interrupts();
    uint64_t i;
    for (i = 0; i < clientCount; i++) {
      uint64_t mask = clients[i].mask & ints;
      mask |= clients[i].waiting & ints;
      clients[i].waiting = 0;
      if (mask) {
        if (!sys_write(clients[i].fd, &mask, 8)) {
          clients[i].waiting |= mask;
        }
      }
    }
  }

  return 0;
}

void handle_fd(uint64_t fd) {
  msg_t msg;

  if (sys_remote_uid(fd) != 0) {
    sys_close(fd);
    return;
  }

  client_t * client = client_lookup(fd);
  if (!client) {
    if (clientCount == 0x40) {
      printf("[ERROR]: too many intd clients\n");
      sys_close(fd);
      return;
    }
    client = client_add();
    client->fd = fd;
    client->mask = 0;
  }

  while (sys_read(fd, &msg)) {
    if (msg.type == 2 || (msg.type == 1 && msg.len != 8)) {
      sys_close(fd);
      client_remove(client);
      return;
    } else if (msg.type == 0) continue;
    void * body = msg.message;
    client->mask = *((uint64_t *)body);
    if (client->waiting) {
      uint64_t sendMask = client->waiting & client->mask;
      client->waiting = 0;
      if (sendMask) {
        if (!sys_write(fd, &sendMask, 8)) {
          client->waiting = sendMask;
        }
      }
    }
  }
}

client_t * client_lookup(uint64_t fd) {
  uint64_t i;
  for (i = 0; i < clientCount; i++) {
    if (clients[i].fd == fd) return &clients[i];
  }
  return NULL;
}

client_t * client_add() {
  return &clients[clientCount++];
}

void client_remove(client_t * client) {
  uint64_t i;
  for (i = 0; i < clientCount; i++) {
    if (&clients[i] > client) {
      clients[i - 1] = clients[i];
    }
    clientCount--;
  }
}

