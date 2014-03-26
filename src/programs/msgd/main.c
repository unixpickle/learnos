#include <stdio.h>
#include <msgd.h>
#include <string.h>

typedef struct {
  uint64_t fd;
  uint64_t serviceId;
  char name[0xf0];
} __attribute__((packed)) service_t;

typedef struct {
  uint64_t fd;
} __attribute__((packed)) client_t;

void handle_fd(uint64_t fd);
service_t * service_lookup(uint64_t fd);
service_t * service_lookup_id(uint64_t servId);
client_t * client_lookup(uint64_t fd);

bool service_msg(service_t * service, msg_t * msg);
bool client_msg(client_t * client, msg_t * msg);
bool anonymous_msg(uint64_t fd, msg_t * msg);

void client_req_verify(client_t * cli, uint64_t servId);
void client_req_search(client_t * cli, const char * name);

void client_remove(client_t * client);
void service_remove(service_t * service);
client_t * client_add();
service_t * service_add();

static uint64_t serviceId = 0;

static uint64_t serviceCount = 0;
static service_t * services = NULL;

static uint64_t clientCount = 0;
static client_t * clients = NULL;

int main() {
  // until I actually do heap allocation
  service_t _services[0x40];
  services = _services;
  client_t _clients[0x40];
  clients = _clients;

  printf("message server running!\n");

  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_fd(fd);
  }
  return 0;
}

void handle_fd(uint64_t fd) {
  msg_t msg;

  // figure out what role the FD plays
  service_t * service = service_lookup(fd);
  client_t * client = service ? NULL : client_lookup(fd);

  while (sys_read(fd, &msg)) {
    if (service) {
      if (!service_msg(service, &msg)) return;
    } else if (client) {
      if (!client_msg(client, &msg)) return;
    } else {
      if (!anonymous_msg(fd, &msg)) return;
      if (msg.type == 1) {
        service = service_lookup(fd);
        if (!service) client = client_lookup(fd);
      }
    }
  }
}

service_t * service_lookup(uint64_t fd) {
  uint64_t i;
  for (i = 0; i < serviceCount; i++) {
    if (services[i].fd == fd) return &services[i];
  }
  return NULL;
}

service_t * service_lookup_id(uint64_t servId) {
  uint64_t i;
  for (i = 0; i < serviceCount; i++) {
    if (services[i].serviceId == servId) return &services[i];
  }
  return NULL;
}

client_t * client_lookup(uint64_t fd) {
  uint64_t i;
  for (i = 0; i < clientCount; i++) {
    if (clients[i].fd == fd) return &clients[i];
  }
  return NULL;
}

bool service_msg(service_t * service, msg_t * msg) {
  if (msg->type == 2 || (msg->type == 1 && msg->len < 8)) {
    sys_close(service->fd);
    service_remove(service);
    return false;
  }
  // right now, there's no data this could possibly be, but in the future there
  // might be, and the software is probing for features.
  return true;
}

bool client_msg(client_t * client, msg_t * msg) {
  if (msg->type == 2 || (msg->type == 1 && msg->len < 8)) {
    sys_close(client->fd);
    client_remove(client);
    return false;
  }
  void * msgData = (void *)msg->message;
  uint64_t type = *((uint64_t *)msgData);
  if (type == MSG_TYPE_VERIFY) {
    if (msg->len != 0x10) {
      sys_close(client->fd);
      client_remove(client);
      return false;
    }
    uint64_t serviceId = ((uint64_t *)msgData)[1];
    client_req_verify(client, serviceId);
  } else if (type == MSG_TYPE_LOOKUP) {
    if (msg->len >= 0xf8) {
      sys_close(client->fd);
      client_remove(client);
      return false;
    }

    // get the name
    char name[0xf0];
    memcpy(name, msg->message + 8, msg->len - 8);
    name[msg->len - 8] = 0;
    client_req_search(client, name);
  }
  return true;
}

bool anonymous_msg(uint64_t fd, msg_t * msg) {
  if (msg->type == 0) return true;
  if (msg->type == 2) {
    sys_close(fd);
    return false;
  }
  if (msg->type == 1 && msg->len != 8) {
    sys_close(fd);
    return false;
  }

  void * msgData = (void *)msg->message;
  uint64_t type = *((uint64_t *)msgData);
  if (type == MSG_TYPE_SERVICE_INIT) {
    if (msg->len >= 0xf8 || msg->len == 8) {
      sys_close(fd);
      return false;
    }
    if (sys_remote_pid(fd) != 0) {
      printf("[ERROR]: non-privileged task %x tried to register service\n",
             sys_remote_pid(fd));
      sys_close(fd);
      return false;
    }

    service_t * serv = service_add();
    serv->fd = fd;
    serv->serviceId = ++serviceId;
    // the name will be up to 0xef bytes long with a 1 byte NULL termination
    memcpy(serv->name, msg->message + 8, msg->len - 8);
    serv->name[msg->len - 8] = 0;
    return true;
  } else if (type == MSG_TYPE_CLIENT_INIT) {
    client_t * cli = client_add();
    cli->fd = fd;
    return true;
  }

  sys_close(fd);
  return false;
}

void client_req_verify(client_t * cli, uint64_t servId) {
  uint64_t pid = sys_remote_pid(cli->fd);
  if (!(pid + 1)) return;
  service_t * serv = service_lookup_id(servId);
  uint64_t type = serv ? MSG_TYPE_EXISTS : MSG_TYPE_NOT_EXISTS;
  sys_write(cli->fd, &type, 8);
}

void client_req_search(client_t * cli, const char * name) {
  service_t * serv = NULL;

  uint64_t i;
  for (i = 0; i < serviceCount; i++) {
    if (strequal(services[i].name, name)) {
      serv = &services[i];
      break;
    }
  }

  if (!serv) {
    uint64_t type = MSG_TYPE_NOT_FOUND;
    sys_write(cli->fd, &type, 8);
  } else {
    uint64_t pid = sys_remote_pid(serv->fd);
    if (pid + 1) {
      msgd_result_t result = {MSG_TYPE_FOUND, serv->serviceId, pid};
      sys_write(cli->fd, &result, 0x18);
    } else {
      uint64_t type = MSG_TYPE_NOT_FOUND;
      sys_write(cli->fd, &type, 8);
    }
  }
}

void client_remove(client_t * client) {
  uint64_t i;
  for (i = 0; i < clientCount; i++) {
    if (&clients[i] > client) {
      clients[i - 1] = clients[i];
    }
  }
  clientCount--;
}

void service_remove(service_t * service) {
  uint64_t i;
  for (i = 0; i < serviceCount; i++) {
    if (&services[i] > service) {
      services[i - 1] = services[i];
    }
  }
  serviceCount--;
}

client_t * client_add() {
  return &clients[clientCount++];
}

service_t * service_add() {
  return &services[serviceCount++];
}

