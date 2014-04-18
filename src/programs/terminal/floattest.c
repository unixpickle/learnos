#include <stdio.h>

void command_floattest() {
  float number = 10.0f;
  float pi = 3.141592f;
  printf("floor(pi*10) = 0x%x\n", (int)(number * pi));
}
