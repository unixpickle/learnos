#include <stdio.h>

void printf_impl(const uint64_t * arguments) {
  const char * string = (const char *)arguments[0];
  int argOffset = 1;
  for (; *string; string++) {
    if ((*string) == '%') {
      char nextChar = *(++string);
      if (nextChar == '%' || !nextChar) {
        sys_print("%");
      } else {
        uint64_t nextArg = arguments[argOffset++];
        print_argument(nextChar, nextArg);
      }
    } else {
      char buff[2] = {*string, 0};
      sys_print(buff);
    }
  }
}

void print_argument(char type, uint64_t arg) {
  if (type == 'x') {
    print_hex(arg, false);
  } else if (type == 'X') {
    print_hex(arg, true);
  } else if (type == 's') {
    sys_print((const char *)arg);
  }
}

void print_hex(uint64_t number, bool caps) {
  const char * chars = "0123456789ABCDEF";
  if (!caps) chars = "0123456789abcdef";
  unsigned char buf[32];
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
  buf[len] = 0;
  sys_print((const char *)buf);
}

