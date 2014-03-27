#include <stdio.h>
#include <msgd.h>

static uint64_t get_next_byte();
//static uint32_t scancode_in();
//static bool byte_out(uint8_t byte);

static uint64_t * clients = NULL;
static uint64_t clientCount = 0;

static uint64_t intd = 0;

void client_handle(uint64_t fd);
void handle_intd();
void handle_interrupt();

int main() {
  uint64_t _clients[0x40];
  clients = _clients;

  char * intdName = "intd";
  msgd_connect_services(1, (const char **)&intdName, &intd, 3);
  if (!(intd + 1)) {
    printf("[keyboard]: error: failed to connect to intd\n");
  }
  printf("[keyboard]: connected to interrupt daemon.\n");
  
  uint64_t sock = msgd_connect();

  if (!(sock + 1)) {
    printf("[ERROR]: failed to register keyboard driver.\n");
    return 0;
  }
  msgd_register_service(sock, "keyboard");

  while (1) {
    while (1) {
      uint64_t val = get_next_byte();
      if (val > 0xff) break;
      printf("[keyboard]: byte %x\n", val);
    }

    uint64_t fd = sys_poll();
    if (!(fd + 1)) continue;
    if (fd != intd) {
      client_handle(fd);
    } else {
      handle_intd();
    }
  }

/**

  // set scan code set 2
  if (byte_out(0xf0)) {
    printf("writing out scan code set\n");
    byte_out(1);
  }

  while (1) {
    uint32_t input = scancode_in();
    if (input == 0x1c) {
      printf("A");
    } else printf("unknown scancode %x\n", input);
  }
**/
  return 0;
}

void client_handle(uint64_t fd) {
  msg_t msg;
  while (sys_read(fd, &msg)) {
    if (msg.type == 0) {
      if (clientCount >= 0x40) {
        printf("[WARNING]: keyboard driver connection max!\n");
        sys_close(fd);
        return;
      }
      clients[clientCount++] = fd;
      continue;
    } else if (msg.type == 2) {
      uint64_t i, hasFound = 0;
      for (i = 0; i < clientCount; i++) {
        if (hasFound) {
          clients[i - 1] = clients[i];
        } else if (clients[i] == fd) {
          hasFound = true;
        }
      }
      if (hasFound) clientCount--;
      sys_close(fd);
      return;
    }
    // TODO: here, read whatever keyboard commands they send
  }
}

void handle_intd() {
  msg_t msg;
  while (sys_read(intd, &msg)) {
    if (msg.type == 2) {
      printf("[keyboard]: error: intd disconnected us!\n");
      sys_exit();
    }
    if (msg.len != 8) continue;
    void * msgData = msg.message;
    uint64_t mask = *((uint64_t *)msgData);
    if (mask | 2) {
      handle_interrupt();
    }
  }
}

void handle_interrupt() {
  printf("Got keyboard interrupt.\n");
}

static uint64_t get_next_byte() {
  uint8_t byte = sys_in(0x64, 1);
  if (byte & 1) {
    return sys_in(0x60, 1);
  }
  return 0x100;
}

/*
static uint32_t scancode_in() {
  uint8_t code = get_next_byte();
  if (code == 0xe0) {
    uint8_t nextCode = get_next_byte();
    if (nextCode == 0xf0) {
      uint8_t nextNext = get_next_byte();
      uint8_t nextNextNext = get_next_byte();
      return code | (nextCode << 8) | (nextNext << 16) | (nextNextNext << 24);
    } else {
      return code | (nextCode << 8) | (get_next_byte() << 16);
    }
  } else if (code == 0xf0) {
    return code | (get_next_byte() << 8);
  } else {
    return code;
  }
}

static bool byte_out(uint8_t byte) {
  // wait up to 100 ms to write a byte
  uint64_t startTime = sys_get_time();
  bool ready = false;
  while (sys_get_time() < startTime + 100) {
    if (sys_in(0x64) & 1) {
      ready = true;
      break;
    }
  }
  if (!ready) return false;
  sys_out(0x60, byte);
  return true;
}
*/

