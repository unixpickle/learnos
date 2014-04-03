#include <stdio.h>
#include <msgd.h>
#include <string.h>
#include <keyedbits/buff_decoder.h>
#include <keyedbits/buff_encoder.h>

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
bool process_connect_msg(uint64_t fd, msg_t * msg);

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

  printf("[msgd]: now running\n");

  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_fd(fd);
  }

  return 0;
}

void handle_fd(uint64_t fd) {
  msg_t msg;

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

  if (msg->type == 1 && msg->len < 8) {
    sys_close(fd);
    return false;
  }

  if (!process_connect_msg(fd, msg)) {
    sys_close(fd);
    return false;
  }

  return true;
}

bool process_connect_msg(uint64_t fd, msg_t * msg) {
  kb_buff_t buff;
  kb_header_t header;
  kb_buff_initialize_decode(&buff, msg->message, msg->len);
  if (!kb_buff_read_header(&buff, &header)) return false;
  if (header.typeField != KeyedBitsTypeDictionary) return false;
  char key[16];
  char name[0xf0];
  int role = 0; // 0 for invalid, 1 for service, 2 for client
  bool gotName = false;
  while (1) {
    if (!kb_buff_read_key(&buff, key, 64)) return false;
    if (!strcmp(key, "role") && !role) {
      if (!kb_buff_read_header(&buff, &header)) return false;
      if (header.typeField != KeyedBitsTypeString) return false;
      const char * str;
      uint64_t len;
      if (!kb_buff_read_string(&buff, &str, &len)) return false;
      if (!strcmp(str, "client")) {
        role = 2;
      } else if (!strcmp(str, "service")) {
        role = 1;
      } else return false;
    } else if (!strcmp(key, "name") && !gotName) {
      if (!kb_buff_read_header(&buff, &header)) return false;
      if (header.typeField != KeyedBitsTypeString) return false;
      const char * str;
      uint64_t len;
      if (!kb_buff_read_string(&buff, &str, &len)) return false;
      if (len > 0xef) return false;
      memcpy(name, str, len + 1);
      gotName = true;
    } else if (key[0] == 0) {
      break;
    } else return false;
  }
  if (role == 1 && !gotName) return false;
  else if (role == 2 && gotName) return false;

  // TODO: generate the appropriate client or service object
  return true;
}

void client_req_verify(client_t * cli, uint64_t servId) {
  uint64_t pid = sys_remote_pid(cli->fd);
  if (!(pid + 1)) return;
  service_t * serv = service_lookup_id(servId);
  if (serv) {
    if (!(sys_remote_pid(serv->fd) + 1)) {
      serv = NULL; // closed, but we haven't gotten the notification yet
    }
  }
  uint64_t type = serv ? MSG_TYPE_EXISTS : MSG_TYPE_NOT_EXISTS;
  sys_write(cli->fd, &type, 8);
}

void client_req_search(client_t * cli, const char * name) {
  service_t * serv = NULL;

  uint64_t i;
  for (i = 0; i < serviceCount; i++) {
    if (!strcmp(services[i].name, name)) {
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

