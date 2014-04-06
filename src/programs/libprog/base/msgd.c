#include <base/system.h>
#include <string.h>
#include "msgd.h"

typedef struct {
  uint64_t numTries; // starts at an argument, when 0 = fail
  uint64_t shouldFail; // 1 = give up on this one!
  uint64_t msgdSock;
  uint64_t shutdown;

  // progress indicator
  uint64_t hasBeenVerified; // 1 = done this one
  uint64_t currentFd; // the current socket being verified
  const char * curName;
} conn_session_t;

static void _conn_verify_succ(conn_session_t * info);
static void _conn_verify_fail(conn_session_t * info);
static void _conn_result_found(conn_session_t * info, msgd_result_t * res);
static void _conn_result_not_found(conn_session_t * info);
static void _conn_closed(conn_session_t * info);

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

void msgd_connect_services(uint64_t count,
                           const char ** names,
                           uint64_t * sockets,
                           uint64_t attempts) {
  conn_session_t session = {0, 0, 0, 0, 0, 0, NULL};
  msgd_funcs_t funcs = {
    &session,
    (void (*)(void *))_conn_verify_succ,
    (void (*)(void *))_conn_verify_fail,
    (void (*)(void *, msgd_result_t *))_conn_result_found,
    (void (*)(void *))_conn_result_not_found,
    (void (*)(void *))_conn_closed
  };
  session.msgdSock = msgd_connect();
  if (!(session.msgdSock + 1)) {
    return; // we've gotten 0
  }
  msgd_register_client(session.msgdSock);

  uint64_t i;
  for (i = 0; i < count; i++) {
    if (session.shutdown) break;
    session.numTries = attempts;
    session.shouldFail = 0;
    session.hasBeenVerified = 0;
    session.curName = names[i];
    msgd_lookup_service(session.msgdSock, names[i]);
    while (1) {
      uint64_t sock = sys_poll();
      if (sock != session.msgdSock) continue;
      msgd_handle_packets(sock, &funcs);
      if (session.hasBeenVerified) {
        sockets[i] = session.currentFd;
        break;
      } else if (session.shouldFail) {
        sockets[i] = 0xffffffffffffffffL;
        break;
      }
    }
  }
  sys_close(session.msgdSock);
}

static void _conn_verify_succ(conn_session_t * info) {
  info->hasBeenVerified = 1;
}

static void _conn_verify_fail(conn_session_t * info) {
  sys_close(info->currentFd);
  info->shouldFail = 1;
}

static void _conn_result_found(conn_session_t * info, msgd_result_t * res) {
  uint64_t sock = sys_open();
  if (!(sock + 1)) {
    info->shouldFail = 1;
    return;
  }
  if (!sys_connect(sock, res->pid)) {
    sys_close(sock);
    info->numTries--;
    if (!info->numTries) {
      info->shouldFail = 1;
    } else {
      msgd_lookup_service(info->msgdSock, info->curName);
    }
    return;
  }
  info->currentFd = sock;
  msgd_verify_service(info->msgdSock, res->serviceId);
}

static void _conn_result_not_found(conn_session_t * info) {
  if (!--(info->numTries)) {
    info->shouldFail = 1;
    return;
  }
  sys_sleep(0x8000);
  msgd_lookup_service(info->msgdSock, info->curName);
}

static void _conn_closed(conn_session_t * info) {
  info->shouldFail = (info->shutdown = 1);
}

