#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <msgd.h>

void main() {
  uint64_t sock = msgd_connect();
  msgd_register_service(sock, "timed");

  while (1) {
    uint64_t fd = sys_poll();
    if (!(fd + 1)) continue;
  }
}

