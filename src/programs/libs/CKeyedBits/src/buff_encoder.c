#include <keyedbits/buff_encoder.h>

static int _read_dec_string(char * output, uint64_t max, uint64_t dec);
static bool _write_byte(kb_buff_t * kb, uint8_t byte);

void kb_buff_initialize_encode(kb_buff_t * kb, void * buff, uint64_t len) {
  kb->buff = buff;
  kb->off = 0;
  kb->len = len;
}

bool kb_buff_write_int(kb_buff_t * kb, int64_t number) {
  if (number <= 0x7FFFFFFFL && number >= -0x80000000L) {
    // use 32 bits
    if (kb->off + 5 > kb->len) return false;
    *((uint8_t *)(kb->buff + kb->off)) = 0xA6;
    *((int32_t *)(kb->buff + kb->off + 1)) = (int32_t)number;
    kb->off += 5;
  } else {
    // use 64 bits
    if (kb->off + 9 > kb->len) return false;
    *((uint8_t *)(kb->buff + kb->off)) = 0xC6;
    *((int64_t *)(kb->buff + kb->off + 1)) = number;
    kb->off += 9;
  }
  
  return true;
}

bool kb_buff_write_string(kb_buff_t * kb, const char * str) {
  if (kb->off + 1 >= kb->len) return false;
  *((uint8_t *)(kb->buff + kb->off)) = 0x81;
  kb->off++;
  while (kb->off < kb->len) {
    char ch = *(str++);
    *((char *)(kb->buff + kb->off)) = ch;
    kb->off++;
    if (!ch) return true;
  }
  return false;
}

bool kb_buff_write_double_v1(kb_buff_t * kb, double d) {
  char _result[64];
  char * result = &_result[1];
  
  // encode the part before and after the decimals
  uint64_t topPart = (uint64_t)(d < 0 ? -d : d);
  uint64_t bottomPart = (uint64_t)(((d < 0 ? -d : d) - (double)topPart)
    * 10000000000.0);
  while (!(bottomPart % 10) && bottomPart) bottomPart /= 10;
  
  // generate the string
  int res = _read_dec_string(result, 63, topPart);
  if (res < 0) return false;
  uint64_t total = res;
  if (bottomPart) {
    result[res] = '.';
    int nextRes = _read_dec_string(&result[res + 1], 63 - (res + 1), 
                                   bottomPart);
    if (nextRes < 0) return false;
    total += nextRes + 1;
  }
  
  // make negative if needed
  if (d < 0) {
    total++;
    result -= 1;
    result[0] = '-';
  }
  
  // verify the range
  if (total + 2 + kb->off > kb->len) return false;
  
  *((uint8_t *)(kb->buff + kb->off)) = 0x87;
  kb->off++;
  
  uint64_t i;
  for (i = 0; i <= total; i++) {
    *((char *)(kb->buff + kb->off)) = result[i];
    kb->off++;
  }
  
  return true;
}

bool kb_buff_write_double_v2(kb_buff_t * kb, double d) {
  // TODO: this
  return kb_buff_write_double_v1(kb, d);
}

bool kb_buff_write_data(kb_buff_t * kb, const void * data, uint32_t len) {
  uint8_t * buff = kb->buff + kb->off;
  uint32_t i;
  
  if (len < 0x100) {
    if (kb->off + len + 2 > kb->len) return false;
    buff[0] = 0x05;
    buff[1] = (uint8_t)len;
    for (i = 0; i < len; i++) {
      buff[i + 2] = *((uint8_t *)(data + i));
    }
    kb->off += 2 + len;
  } else if (len < 0x10000) {
    if (kb->off + len + 3 > kb->len) return false;
    buff[0] = 0x25;
    *((uint16_t *)(buff + 1)) = (uint16_t)len;
    for (i = 0; i < len; i++) {
      buff[i + 3] = *((uint8_t *)(data + i));
    }
    kb->off += 3 + len;
  } else if (len < 0x1000000) {
    if (kb->off + len + 4 > kb->len) return false;
    buff[0] = 0x45;
    buff[1] = (uint8_t)len;
    buff[2] = (uint8_t)(len >> 8);
    buff[3] = (uint8_t)(len >> 0x10);
    for (i = 0; i < len; i++) {
      buff[i + 4] = *((uint8_t *)(data + i));
    }
    kb->off += 4 + len;
  } else {
    if (kb->off + len + 5 > kb->len) return false;
    buff[0] = 0x65;
    *((uint32_t *)(buff + 1)) = len;
    for (i = 0; i < len; i++) {
      buff[i + 5] = *((uint8_t *)(data + i));
    }
    kb->off += 5 + len;
  }
  
  return true;
}

bool kb_buff_write_key(kb_buff_t * kb, const char * key) {
  uint64_t i;
  for (i = 0; kb->off < kb->len; i++, kb->off++) {
    uint8_t ch = key[i];
    if (ch & 0x80) return false;
    
    if (!key[i + 1]) {
      *((uint8_t *)(kb->buff + kb->off)) = ch | 0x80;
      kb->off++;
      return true;
    }
    
    *((uint8_t *)(kb->buff + kb->off)) = ch;
  }
  return false;
}

bool kb_buffer_write_null(kb_buff_t * kb) {
  return _write_byte(kb, 0x04);
}

bool kb_buff_write_array(kb_buff_t * kb) {
  return _write_byte(kb, 0x82);
}

bool kb_buff_write_dict(kb_buff_t * kb) {
  return _write_byte(kb, 0x83);
}

bool kb_buff_write_terminator(kb_buff_t * kb) {
  return _write_byte(kb, 0);
}

static int _read_dec_string(char * output, uint64_t max, uint64_t dec) {
  if (max < 2) return -1;
  if (dec == 0) {
    output[0] = '0';
    output[1] = 0;
    return 1;
  }
  
  int i, len;
  for (len = 0; len + 1 < max; len++) {
    if (!dec) break;
    uint64_t digit = dec % 10;
    dec /= 10;
    for (i = len; i > 0; i--) {
      output[i] = output[i - 1];
    }
    output[0] = '0' + (char)digit;
  }
  if (dec) return -1;
  output[len] = 0;
  
  return len;
}

static bool _write_byte(kb_buff_t * kb, uint8_t byte) {
  if (kb->off >= kb->len) return false;
  *((uint8_t *)(kb->buff + kb->off)) = byte;
  kb->off++;
  return true;
}
