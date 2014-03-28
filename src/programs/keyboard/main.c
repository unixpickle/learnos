#include <stdio.h>
#include <msgd.h>

static uint64_t * clients = NULL;
static uint64_t clientCount = 0;
static uint64_t intd = 0;
static bool lastWasE0 = false;

static uint8_t controlCount = 0;
static uint8_t shiftCount = 0;
static uint8_t altCount = 0;

static char buffer[0x20];
static uint8_t bufferCount = 0;

void client_handle(uint64_t fd);
void handle_intd();

static uint64_t get_next_byte();
static void handle_byte(uint8_t byte);
static void handle_key_code(uint64_t code);
static char apply_modifiers(char input);
static void buffer_put(char ch);
static void buffer_flush();

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
    if (bufferCount > 0) {
      buffer_flush();
    }

    uint64_t fd = sys_poll();
    if (!(fd + 1)) continue;
    if (fd != intd) {
      client_handle(fd);
    } else {
      handle_intd();
    }
  }

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
    // top row
    {0x1b, 0x1}, {'1', 0x2}, {'2', 0x3}, {'3', 0x4}, {'4', 0x5}, {'5', 0x6},
    {'6', 0x7}, {'7', 0x8}, {'8', 0x9}, {'9', 0xa}, {'0', 0xb}, {'[', 0xc},
    {']', 0xd}, {'\b', 0xe},
    // second row (mainly)
    {'\t', 0xf}, {'\'', 0x10}, {',', 0x11}, {'.', 0x12}, {'p', 0x13},
    {'y', 0x14}, {'f', 0x15}, {'g', 0x16}, {'c', 0x17}, {'r', 0x18},
    {'l', 0x19}, {'/', 0x1a}, {'=', 0x1b}, {'\n', 0x1c},
    // home row (mainly)
    {'a', 0x1e}, {'o', 0x1f}, {'e', 0x20}, {'u', 0x21}, {'i', 0x22},
    {'d', 0x23}, {'h', 0x24}, {'t', 0x25}, {'n', 0x26}, {'s', 0x27},
    {'-', 0x28}, {'`', 0x29},
    // bottom row (mainly)
    {'\\', 0x2b}, {';', 0x2c}, {'q', 0x2d}, {'j', 0x2e}, {'k', 0x2f},
    {'x', 0x30}, {'b', 0x31}, {'m', 0x32}, {'w', 0x33}, {'v', 0x34},
    {'z', 0x35},
    // numpad
    {'1', 0x4f}, {'2', 0x50}, {'3', 0x51}, {'4', 0x4b}, {'5', 0x4c},
    {'6', 0x4d}, {'7', 0x47}, {'8', 0x48}, {'9', 0x49}, {'-', 0x4a},
    {'+', 0x4e}, {'0', 0x52}, {'.', 0x53},
    // extra
    {' ', 0x39}, {'\n', 0x3a}
  };
  if (code == 0x2a || code == 0x36) {
    shiftCount |= (code == 0x2a ? 1 : 2);
  } else if (code == 0xaa || code == 0xb6) {
    shiftCount &= ~(code == 0xaa ? 1 : 2);
  } else if (code == 0x1d || code == 0x1de0) {
    controlCount |= (code == 0x1d ? 1 : 2);
  } else if (code == 0x9d || code == 0x9de0) {
    controlCount &= ~(code == 0x9d ? 1 : 2);
  } else if (code == 0x38 || code == 0x38e0 ) {
    altCount |= (code == 0x38 ? 1 : 2);
  } else if (code == 0xb8 || code == 0xb8e0)  {
    altCount &= ~(code == 0xb8 ? 1 : 2);
  } else {
    char baseChar = 0;
    int i;
    for (i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
      if (codes[i].code == code) {
        baseChar = codes[i].ascii;
        break;
      }
    }
    if (!baseChar) return;
    baseChar = apply_modifiers(baseChar);
    if (!baseChar) return;
    buffer_put(baseChar);
  }
}

static char apply_modifiers(char input) {
  struct {
    char orig;
    char shifted;
  } shifts[] = {
    {'-', '_'}, {'\\', '|'}, {'[', '{'}, {']', '}'}, {'[', '{'}, {'\'', '"'},
    {',', '<'}, {'.', '>'}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'},
    {'5', '%'}, {'6', '^'}, {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'},
    {'`', '~'}, {'=', '+'}, {'/', '?'}
  };
  struct {
    char orig;
    char result;
  } controls[] = {
    {'[', 27}, {'?', 0x7f}
  };

  if (altCount) return 0;

  // apply shift
  if (shiftCount) {
    if (input >= 'a' && input <= 'z') {
      input = (input + 'A' - 'a');
    } else {
      int i;
      for (i = 0; i < sizeof(shifts) / sizeof(shifts[0]); i++) {
        if (shifts[i].orig == input) {
          input = shifts[i].shifted;
          break;
        }
      }
    }
  }

  // apply control
  if (controlCount) {
    if (input >= 'a' && input <= 'z') {
      return input - 'a' + 1;
    } else if (input >= 'A' && input <= 'Z') {
      return input - 'A' + 1;
    }
    // figure it out from the controls array
    int i;
    for (i = 0; i < sizeof(controls) / sizeof(controls[0]); i++) {
      if (controls[i].orig == input) {
        return controls[i].result;
      }
    }
    return 0;
  }
  return input;
}

static void buffer_put(char ch) {
  if (bufferCount == 0x20) {
    buffer_flush();
  }
  buffer[bufferCount++] = ch;
}

static void buffer_flush() {
  uint64_t i;
  for (i = 0; i < clientCount; i++) {
    sys_write(clients[i], buffer, bufferCount);
  }
  bufferCount = 0;
}

