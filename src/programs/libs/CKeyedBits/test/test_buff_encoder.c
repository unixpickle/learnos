#include <keyedbits/buff_encoder.h>
#include <keyedbits/buff_decoder.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

void test_encode_int();
void test_encode_string();
void test_encode_double();
void test_encode_data();
void test_encode_key();

static void _test_double_v1(double d);

int main() {
  test_encode_int();
  test_encode_string();
  test_encode_double();
  test_encode_data();
  test_encode_key();
  return 0;
}

void test_encode_int() {
  printf("testing encode int...");
  uint8_t buffer[9];
  
  kb_buff_t buff;
  
  // test encode positive 64-bit
  kb_buff_initialize_encode(&buff, buffer, 9);
  bool result = kb_buff_write_int(&buff, 0x181234567L);
  assert(result);
  assert(buff.off == 9);
  assert(buffer[0] == 0xC6);
  assert(buffer[1] == 0x67);
  assert(buffer[2] == 0x45);
  assert(buffer[3] == 0x23);
  assert(buffer[4] == 0x81);
  assert(buffer[5] == 0x1);
  assert(buffer[6] == buffer[7] == buffer[8] == 0);
  
  // test encode negative 64-bit
  kb_buff_initialize_encode(&buff, buffer, 9);
  result = kb_buff_write_int(&buff, -0x123456789L);
  assert(result);
  assert(buff.off == 9);
  assert(buffer[0] == 0xC6);
  assert(*((uint64_t *)(buffer + 1)) == 0xFFFFFFFEDCBA9877L);
  
  // test encode positive 32-bit
  kb_buff_initialize_encode(&buff, buffer, 5);
  result = kb_buff_write_int(&buff, 0x7FFFFFFF);
  assert(result);
  assert(buff.off == 5);
  assert(buffer[0] == 0xA6);
  assert(buffer[1] == 0xFF);
  assert(buffer[2] == 0xFF);
  assert(buffer[3] == 0xFF);
  assert(buffer[4] == 0x7F);
  
  // test encode negative 32-bit
  kb_buff_initialize_encode(&buff, buffer, 5);
  result = kb_buff_write_int(&buff, -0x1234);
  assert(result);
  assert(buff.off == 5);
  assert(buffer[0] == 0xA6);
  assert(*((uint32_t *)(buffer + 1)) == 0xFFFFEDCCL);
  
  printf(" passed!\n");
}

void test_encode_string() {
  printf("testing encode string...");
  
  uint8_t buffer[5];
  kb_buff_t buff;
  
  kb_buff_initialize_encode(&buff, buffer, 5);
  bool result = kb_buff_write_string(&buff, "hey");
  assert(result);
  assert(buffer[0] == 0x81);
  assert(!strcmp((char *)&buffer[1], "hey"));
  
  kb_buff_initialize_encode(&buff, buffer, 5);
  result = kb_buff_write_string(&buff, "heya");
  assert(!result);
  
  printf(" passed!\n");
}

void test_encode_double() {
  _test_double_v1(123.123);
  _test_double_v1(3.141592);
  _test_double_v1(0.123);
  _test_double_v1(-3.141592);
  _test_double_v1(2.0);
  _test_double_v1(-2.0);
  _test_double_v1(-0.3);
}

void test_encode_data() {
  printf("testing data encode... ");
  
  char * buffer = (char *)malloc(0x1000005);
  kb_buff_t buff;
  
  bzero(buffer, 5);
  
  kb_buff_initialize_encode(&buff, buffer, 5);
  bool result = kb_buff_write_data(&buff, "hey", 3);
  assert(result);
  assert(buff.off == 5);
  assert(buffer[0] == 5);
  assert(buffer[1] == 3);
  assert(buffer[2] == 'h');
  assert(buffer[3] == 'e');
  assert(buffer[4] == 'y');
  
  bzero(buffer, 0x103);
  
  kb_buff_initialize_encode(&buff, buffer, 0x103);
  result = kb_buff_write_data(&buff, buffer, 0x100);
  assert(result);
  assert(buff.off == 0x103);
  assert(buffer[0] == 0x25);
  assert(buffer[1] == 0);
  assert(buffer[2] == 1);
  
  bzero(buffer, 0x10004);
  
  kb_buff_initialize_encode(&buff, buffer, 0x10004);
  result = kb_buff_write_data(&buff, buffer, 0x10000);
  assert(result);
  assert(buff.off == 0x10004);
  assert(buffer[0] == 0x45);
  assert(buffer[1] == 0);
  assert(buffer[2] == 0);
  assert(buffer[3] == 1);
  assert(buffer[4] == 0x45);
  
  bzero(buffer, 0x1000005);
  
  kb_buff_initialize_encode(&buff, buffer, 0x1000005);
  result = kb_buff_write_data(&buff, buffer, 0x1000000);
  assert(result);
  assert(buff.off == 0x1000005);
  assert(buffer[0] == 0x65);
  assert(buffer[1] == 0);
  assert(buffer[2] == 0);
  assert(buffer[3] == 0);
  assert(buffer[4] == 1);
  assert(buffer[5] == 0x65);
  
  // make sure it doesn't allow overflow 8
  kb_buff_initialize_encode(&buff, buffer, 2);
  result = kb_buff_write_data(&buff, buffer, 1);
  assert(!result);
  
  // make sure it doesn't allow overflow 16
  kb_buff_initialize_encode(&buff, buffer, 0x102);
  result = kb_buff_write_data(&buff, buffer, 0x100);
  assert(!result);
  
  // make sure it doesn't allow overflow 24
  kb_buff_initialize_encode(&buff, buffer, 0x10003);
  result = kb_buff_write_data(&buff, buffer, 0x10000);
  assert(!result);
  
  // make sure it doesn't allow overflow 32
  kb_buff_initialize_encode(&buff, buffer, 0x1000004);
  result = kb_buff_write_data(&buff, buffer, 0x1000000);
  assert(!result);
  
  free(buffer);
  printf(" passed!\n");
}

void test_encode_key() {
  printf("testing encode key...");
  
  uint8_t buffer[4];
  kb_buff_t buff;
  
  kb_buff_initialize_encode(&buff, buffer, 4);
  bool result = kb_buff_write_key(&buff, "name");
  assert(result);
  assert(buffer[0] == 'n');
  assert(buffer[1] == 'a');
  assert(buffer[2] == 'm');
  assert(buffer[3] == ('e' | 0x80));
  
  kb_buff_initialize_encode(&buff, buffer, 3);
  result = kb_buff_write_key(&buff, "name");
  assert(!result);
  
  printf(" passed!\n");
}

static void _test_double_v1(double d) {
  printf("testing KB v1 double %lf...", d);
  uint8_t buffer[128];
  kb_buff_t buff;
  
  // set buffer to all 1's so it's not gonna give a fake NULL-termination
  uint64_t i;
  for (i = 0; i < 128; i++) {
    buffer[i] = 0xff;
  }
  
  kb_buff_initialize_encode(&buff, buffer, 128);
  bool result = kb_buff_write_double_v1(&buff, d);
  assert(result);
  assert(buffer[0] == 0x87);
  
  kb_buff_initialize_decode(&buff, buffer + 1, buff.off - 1);
  double readD;
  result = kb_buff_read_double(&buff, &readD);
  assert(result);
  
  assert(fabs(readD - d) < fabs(d / 100000.0));
  printf(" passed!\n");
}
