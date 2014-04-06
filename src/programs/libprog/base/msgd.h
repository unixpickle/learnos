#ifndef __MSGD_H__
#define __MSGD_H__

#include <stdtype.h>

enum {
  MSG_TYPE_DRAIN,
  MSG_TYPE_SERVICE_INIT,
  MSG_TYPE_CLIENT_INIT,
  MSG_TYPE_LOOKUP,
  MSG_TYPE_VERIFY,
  MSG_TYPE_EXISTS,
  MSG_TYPE_NOT_EXISTS,
  MSG_TYPE_FOUND,
  MSG_TYPE_NOT_FOUND
};

typedef struct {
  uint64_t type; // MSG_TYPE_FOUND
  uint64_t serviceId;
  uint64_t pid;
} __attribute__((packed)) msgd_result_t;

typedef struct {
  void * userInfo;
  void (* verifySuccess)(void * userInfo);
  void (* verifyFailed)(void * userInfo);
  void (* resultFound)(void * userInfo, msgd_result_t * result);
  void (* resultNotFound)(void * userInfo);
  void (* closed)(void * userInfo);
} msgd_funcs_t;

uint64_t msgd_connect();
void msgd_register_service(uint64_t con, const char * name);
void msgd_register_client(uint64_t con);
void msgd_lookup_service(uint64_t con, const char * name);
void msgd_verify_service(uint64_t con, uint64_t servId);
void msgd_handle_packets(uint64_t fd, msgd_funcs_t * funcs);

/**
 * Must be called at the beginnig of the application's lifecycle. This will poll
 * a whole lot, so you shouldn't have any other important sockets open at this
 * point.
 */
void msgd_connect_services(uint64_t count,
                           const char ** names,
                           uint64_t * sockets,
                           uint64_t attempts);

#endif
