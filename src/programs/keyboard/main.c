#include <stdio.h>
#include <msgd.h>

static uint64_t * clients = NULL;
static uint64_t clientCount = 0;
static uint64_t intd = 0;
static bool lastWasE0 = false;

void client_handle(uint64_t fd);
void handle_intd();

static uint64_t get_next_byte();
static void handle_byte(uint8_t byte);
static void handle_key_code(uint64_t code);

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

  uint64_t mask = 2;
  sys_write(intd, &mask, 8);

  while (1) {
    while (1) {
      uint64_t val = get_next_byte();
      if (val > 0xff) break;
      handle_byte((uint8_t)val);
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
    uint64_t mask = 2;
    sys_write(intd, &mask, 8);
  }
}

static uint64_t get_next_byte() {
  uint8_t byte = sys_in(0x64, 1);
  if (byte & 1) {
    return sys_in(0x60, 1);
  }
  return 0x100;
}

static void handle_byte(uint8_t byte) {
  if (lastWasE0) {
    lastWasE0 = false;
    handle_key_code(0xe0 | (byte << 8));
  } else {
    if (byte == 0xe0) {
      lastWasE0 = true;
    } else {
      handle_key_code(byte);
    }
  }
}

static void handle_key_code(uint64_t code) {
  // process the key code
  struct {
    char ascii;
    uint64_t code;
  } codes[] = {
  };
  printf("[keyboard]: scan code %x\n", code);
}

