#include <stdio.h>
#include <msgd.h>
#include <string.h>

#define PCI_CMD_READ 0
#define PCI_CMD_WRITE 1

typedef struct {
  uint64_t fd;
} __attribute__((packed)) client_t;

void handle_fd(uint64_t fd);
client_t * client_lookup(uint64_t fd);
client_t * client_add();
void client_remove(client_t * client);
void client_msg(client_t * client, msg_t * msg);

static uint64_t clientCount = 0;
static client_t * clients = NULL;

int main() {
  client_t _clients[0x40];
  clients = _clients;

  // connect to name server
  uint64_t fd = msgd_connect();
  if (!(fd + 1)) {
    printf("[pcid]: error: could not connect to msgd\n");
    return 1;
  }
  msgd_register_service(fd, "pcid");

  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_fd(fd);
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
      printf("[ERROR]: too many pcid clients\n");
      sys_close(fd);
      return;
    }
    client = client_add();
    client->fd = fd;
  }

  while (sys_read(fd, &msg)) {
    if (msg.type == 2 || (msg.type == 1 && msg.len != 8)) {
      sys_close(fd);
      client_remove(client);
      return;
    } else if (msg.type == 0) continue;
    client_msg(client, &msg);
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

void client_msg(client_t * client, msg_t * msg) {
  if (msg->len < 8) return;
  void * buffer = (void *)msg->message;
  uint64_t type = *((uint64_t *)buffer);
  if (type == PCI_CMD_READ) {
  } else if (type == PCI_CMD_WRITE) {
  } else {
  }
}

