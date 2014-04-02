#include <keyedbits/buff_decoder.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void test_decode_int();
void test_decode_string();
void test_decode_double();
void test_decode_data();
void test_decode_array();
void test_decode_dictionary();

static void _test_double(const char * str, double value);
static void _test_inval_double(const char * str);
static void _verify_hey_data(const void * buffer, uint64_t len);
static void _verify_data_overflow(const void * buffer, uint64_t len);

int main() {
  test_decode_int();
  test_decode_string();
  test_decode_double();
  test_decode_data();
  test_decode_array();
  test_decode_dictionary();
  return 0;
}

void test_decode_int() {
  printf("testing integer decode...");
  const char * number32 = "\xA6\x37\x13\x37\x13";
  const char * number64 = "\xC6\x37\x13\x37\x90\x78\x56\x34\x12";
  
  kb_buff_t buff;
  kb_header_t header;
  
  // Try 32-bit number
  
  kb_buff_initialize_decode(&buff, (void *)number32, 5);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 6);
  assert(header.reserved == 0);
  assert(header.lenLen == 1);
  
  int64_t number;
  result = kb_buff_read_int(&buff, header.lenLen, &number);
  assert(result);
  assert(number == 0x13371337);
  
  result = kb_buff_read_header(&buff, &header);
  assert(!result);
  
  // Try 64-bit number
  
  kb_buff_initialize_decode(&buff, (void *)number64, 9);
  result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 6);
  assert(header.reserved == 0);
  assert(header.lenLen == 2);
  
  result = kb_buff_read_int(&buff, header.lenLen, &number);
  assert(result);
  assert(number == 0x1234567890371337);
  
  result = kb_buff_read_header(&buff, &header);
  assert(!result);
  
  printf(" passed\n");
}

void test_decode_string() {
  printf("testing string decode...");
  char * buffer = "\x81" "alex rocks!";
  kb_buff_t buff;
  kb_header_t header;
  
  kb_buff_initialize_decode(&buff, (void *)buffer, strlen(buffer) + 1);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 1);
  assert(header.nullTerm);
  
  const char * out = NULL;
  uint64_t len;
  result = kb_buff_read_string(&buff, &out, &len);
  assert(result);
  assert(len == 11);
  assert(!strcmp(out, "alex rocks!"));
  
  result = kb_buff_read_header(&buff, &header);
  assert(!result);
  
  printf(" passed!\n");
}

void test_decode_double() {
  _test_double("3", 3);
  _test_double("3.14159265", 3.14159265);
  _test_double("0.31415e1", 3.1415);
  _test_double("0.31415e+1", 3.1415);
  _test_double("31.415e-1", 3.1415);
  _test_double("-2.1e+10", -21000000000.0);
  _test_double("-3e-2", -0.03);
  _test_inval_double("-");
  _test_inval_double("-3.");
  _test_inval_double("3.");
  _test_inval_double("3.22e");
  _test_inval_double("3.2e-+1");
  _test_inval_double("3.2e1-1");
  _test_inval_double("e1");
}

void test_decode_data() {
  const char * buff1 = "\x05\x03" "hey";
  const char * buff2 = "\x25\x03\x00" "hey";
  const char * buff3 = "\x45\x03\x00\x00" "hey";
  const char * buff4 = "\x65\x03\x00\x00\x00" "hey";

  _verify_hey_data(buff1, 5);
  _verify_hey_data(buff2, 6);
  _verify_hey_data(buff3, 7);
  _verify_hey_data(buff4, 8);
  
  _verify_data_overflow(buff1, 4);
  _verify_data_overflow(buff2, 5);
  _verify_data_overflow(buff3, 6);
  _verify_data_overflow(buff4, 7);
}

void test_decode_array() {
  printf("testing arrays (sort of)...");
  // encoded form of [null, new Buffer("hey"), 3.14]
  const char * buffer = "\x82\x04\x05\x03" "hey" "\x87" "3.14\x00\x00";
  
  kb_buff_t buff;
  kb_header_t header;
  
  kb_buff_initialize_decode(&buff, (void *)buffer, 14);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 2);
  assert(header.nullTerm);
  
  // NOTE: for now, there's not much more testing I can do, since reading an
  // array is as simple as reading the objects *in* the array
  printf(" passed!\n");
}

void test_decode_dictionary() {
  printf("testing dictionary decode...");
  const char * buffer = "\x83" "nam\xe5" "\x81" "alex\x00\x00";
  kb_buff_t buff;
  kb_header_t header;
  
  kb_buff_initialize_decode(&buff, (void *)buffer, 12);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 3);
  assert(header.nullTerm);
  
  char key[64];
  result = kb_buff_read_key(&buff, key, 64);
  assert(result);
  assert(!strcmp(key, "name"));
  
  result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 1);
  assert(header.nullTerm);
  
  const char * nameStr = NULL;
  uint64_t len;
  result = kb_buff_read_string(&buff, &nameStr, &len);
  assert(result);
  assert(len == 4);
  assert(!strcmp(nameStr, "alex"));
  
  result = kb_buff_read_key(&buff, key, 64);
  assert(result);
  assert(key[0] == 0);
  
  result = kb_buff_read_header(&buff, &header);
  assert(!result);
  
  printf(" passed!\n");
}

static void _test_double(const char * str, double value) {
  printf("testing double \"%s\"...", str);
  char ptr[64];
  sprintf(ptr, "\x87%s", str);
  
  kb_buff_t buff;
  kb_header_t header;
  
  kb_buff_initialize_decode(&buff, (void *)ptr, strlen(str) + 2);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 7);
  assert(header.nullTerm);
  
  double d;
  result = kb_buff_read_double(&buff, &d);
  assert(result);
  assert(fabs(d - value) < fabs(value / 100000000.0));
  
  result = kb_buff_read_header(&buff, &header);
  assert(!result);
  
  printf(" passed!\n");
}

static void _test_inval_double(const char * str) {
  printf("testing (invalid) double %s...", str);
  char ptr[64];
  sprintf(ptr, "\x87%s", str);
  kb_buff_t buff;
  kb_header_t header;
  kb_buff_initialize_decode(&buff, (void *)ptr, strlen(str) + 2);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 7);
  assert(header.nullTerm);
  
  double d;
  result = kb_buff_read_double(&buff, &d);
  assert(!result);
  
  printf(" passed!\n");
}

static void _verify_hey_data(const void * buffer, uint64_t len) {
  printf("verifying data of length 0x%llx...", len);
  kb_buff_t buff;
  kb_header_t header;
  kb_buff_initialize_decode(&buff, (void *)buffer, len);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 5);
  
  uint8_t * ptr;
  uint64_t theLen;
  result = kb_buff_read_data(&buff,
                             header.lenLen,
                             (const void **)&ptr,
                             &theLen);
  assert(result);
  assert(theLen == 3);
  assert(ptr[0] == 'h' && ptr[1] == 'e' && ptr[2] == 'y');
    
  result = kb_buff_read_header(&buff, &header);
  assert(!result);
  
  printf(" passed!\n");
}

static void _verify_data_overflow(const void * buffer, uint64_t len) {
  printf("verifying data overflow of len 0x%llx...", len);
  kb_buff_t buff;
  kb_header_t header;
  kb_buff_initialize_decode(&buff, (void *)buffer, len);
  bool result = kb_buff_read_header(&buff, &header);
  assert(result);
  assert(header.typeField == 5);
  
  uint8_t * ptr;
  uint64_t theLen;
  result = kb_buff_read_data(&buff,
                             header.lenLen,
                             (const void **)&ptr,
                             &theLen);
  assert(!result);
  printf(" passed!\n");
}
