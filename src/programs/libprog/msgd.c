#include "msgd.h"
#include "system.h"
#include "string.h"

uint64_t msgd_connect() {
  uint64_t sock = sys_open();
  if (!(sock + 1)) return sock;
  if (!sys_connect(sock, 0)) {
    sys_close(sock);
    return 0xFFFFFFFFFFFFFFFFL;
  }
  return sock;
}

void msgd_register_service(uint64_t con, const char * name) {
  uint64_t len = strlen(name);
  if (len > 0xef) len = 0xef;
  char data[len + 8];
  void * dataRef = data;
  (*((uint64_t *)dataRef)) = MSG_TYPE_SERVICE_INIT;
  memcpy(data + 8, name, len);
  sys_write(con, data, len + 8);
}

void msgd_register_client(uint64_t con) {
  uint64_t type = MSG_TYPE_CLIENT_INIT;
  sys_write(con, &type, 8);
}

void msgd_lookup_service(uint64_t con, const char * name) {
  uint64_t len = strlen(name);
  if (len > 0xef) len = 0xef;
  char data[len + 8];
  void * dataRef = data;
  (*((uint64_t *)dataRef)) = MSG_TYPE_LOOKUP;
  memcpy(data + 8, name, len);
  sys_write(con, data, len + 8);
}

void msgd_verify_service(uint64_t con, uint64_t servId) {
  uint64_t buff[2] = {MSG_TYPE_VERIFY, servId};
  sys_write(con, buff, 0x10);
}

void msgd_handle_packets(uint64_t fd, msgd_funcs_t * funcs) {
  msg_t msg;
  const void * msgData = msg.message;
  const uint64_t * numFields = (const uint64_t *)msgData;

  while (sys_read(fd, &msg)) {
    if (msg.type == 2) {
      sys_close(fd);
      if (funcs->closed) funcs->closed(funcs->userInfo);
      break;
    }
    if (msg.len < 8) continue;

    uint64_t type = *numFields;
    if (type == MSG_TYPE_EXISTS) {
      if (funcs->verifySuccess) funcs->verifySuccess(funcs->userInfo);
    } else if (type == MSG_TYPE_NOT_EXISTS) {
      if (funcs->verifyFailed) funcs->verifyFailed(funcs->userInfo);
    } else if (type == MSG_TYPE_FOUND) {
      if (funcs->resultFound) {
        funcs->resultFound(funcs->userInfo, (msgd_result_t *)msgData);
      }
    } else if (type == MSG_TYPE_NOT_FOUND) {
      if (funcs->resultNotFound) funcs->resultNotFound(funcs->userInfo);
    }
  }
}

