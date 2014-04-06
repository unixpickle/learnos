#include <stdio.h>
#include <base/system.h>
#include <base/msgd.h>
#include <structures/pci.h>

#define PCI_CMD_READ 0
#define PCI_CMD_WRITE 1

#define CONFIG_ADDRESS (uint16_t)0xCF8
#define CONFIG_DATA (uint16_t)0xCFC
#define PCI_ADDRESS(bus, slot, func, off) ((bus << 16) | (slot << 11) | (func << 8) | off | (1 << 31))

typedef struct {
  uint64_t fd;
} __attribute__((packed)) client_t;

void handle_fd(uint64_t fd);
client_t * client_lookup(uint64_t fd);
client_t * client_add();
void client_remove(client_t * client);
bool client_msg(client_t * client, msg_t * msg);

uint32_t pci_read(uint32_t addr);
void pci_write(uint32_t addr, uint32_t val);

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
    if (!client_msg(client, &msg)) {
      sys_close(fd);
      client_remove(client);
      return;
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

bool client_msg(client_t * client, msg_t * msg) {
  if (msg->len < 8) return false;
  void * buffer = (void *)msg->message;
  uint64_t type = *((uint64_t *)buffer);
  if (type == PCI_CMD_READ) {
    if (msg->len < 8 + sizeof(pci_address)) {
      return false;
    }
    pci_address * addr = buffer + 8;
    uint64_t addrNumber = PCI_ADDRESS(addr->bus, addr->device,
                                      addr->function, addr->reg);
    uint32_t result = pci_read((uint32_t)addrNumber);
    sys_write(client->fd, &result, 4);
  } else if (type == PCI_CMD_WRITE) {
    if (msg->len < 12 + sizeof(pci_address)) {
      return false;
    }
    uint32_t value = *((uint32_t *)(buffer + 8));
    pci_address * addr = buffer + 12;
    uint64_t addrNumber = PCI_ADDRESS(addr->bus, addr->device,
                                      addr->function, addr->reg);
    pci_write((uint32_t)addrNumber, value);
  } else {
    return false;
  }
  return true;
}

uint32_t pci_read(uint32_t addr) {
  sys_out(CONFIG_ADDRESS, addr, 4);
  return (uint32_t)sys_in(CONFIG_DATA, 4);
}

void pci_write(uint32_t addr, uint32_t val) {
  sys_out(CONFIG_ADDRESS, addr, 4);
  sys_out(CONFIG_DATA, val, 4);
}

