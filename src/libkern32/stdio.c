#include <shared/addresses.h>
#include "basic.h"
#include "stdio.h"

#define cursorInfo ((unsigned short *)CURSOR_INFO)
#define buffer ((unsigned char *)SCREEN_BUFFER)

static void setPosition(unsigned short x, unsigned short y);
static void scrollUp();

void print32(const char * str) {
  unsigned short x = cursorInfo[0], y = cursorInfo[1];
  // print each character like it might be your last!
  while (str[0]) {
    unsigned char theChar = str[0];
    str++;
    if (theChar == '\n') {
      y++;
      x = 0;
    } else {
      int loc = x + (SCREEN_WIDTH * y);
      buffer[loc * 2] = theChar;
      buffer[loc * 2 + 1] = 0x0a;
      x++;
      if (x >= SCREEN_WIDTH) {
        x = 0;
        y++;
      }
    }
    while (y >= SCREEN_HEIGHT) {
      scrollUp();
      y--;
    }
  }
  setPosition(x, y);
}

void printHex32(unsigned int number) {
  const char * chars = "0123456789ABCDEF";
  unsigned char buf[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char len = 0, i;
  do {
    unsigned char nextDig = (unsigned char)(number & 0xf);
    buf[len++] = chars[nextDig];
    number >>= 4;
  } while (number > 0);
  for (i = 0; i < len / 2; i++) {
    unsigned char a = buf[len - i - 1];
    buf[len - i - 1] = buf[i];
    buf[i] = a;
  }
  print32(buf);
}

static void setPosition(unsigned short x, unsigned short y) {
  cursorInfo[0] = x;
  cursorInfo[1] = y;
  unsigned short position = (y * SCREEN_WIDTH) + x;
  // tell the VGA index register we are sending the `low` byte
  outb32(0x3D4, 0x0f);
  outb32(0x3D5, (unsigned char)(position & 0xff));
  // and now send the `high` byte
  outb32(0x3D4, 0x0e);
  outb32(0x3D5, (unsigned char)((position >> 8) & 0xff));
}

static void scrollUp() {
  // copy the buffer into itself, one line up
  int i;
  for (i = 0; i < 2 * SCREEN_WIDTH * (SCREEN_HEIGHT - 1); i++) {
    buffer[i] = buffer[i + SCREEN_WIDTH];
  }
  // clear the bottom line
  // TODO: see if i can do for (; ...)
  for (i = i; i < 2 * SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
    buffer[i] = 0;
  }
}

