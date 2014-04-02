#include <keyedbits/buff_decoder.h>

static bool _parse_int(const char * buff, int * number);

void kb_buff_initialize_decode(kb_buff_t * kb, void * buff, uint64_t len) {
  kb->buff = buff;
  kb->len = len;
  kb->off = 0;
}

bool kb_buff_read_header(kb_buff_t * kb, kb_header_t * header) {
  if (kb->off == kb->len) return false;
  (*header) = *((const kb_header_t *)(kb->buff + kb->off));
  kb->off++;
  return true;
}

bool kb_buff_read_string(kb_buff_t * kb, const char ** out, uint64_t * len) {
  uint64_t i;
  (*out) = kb->buff + kb->off;
  for (i = 0; kb->off < kb->len; i++) {
    const char ch = *((const char *)(kb->buff + kb->off));
    kb->off++;
    if (!ch) {
      (*len) = i;
      return true;
    }
  }
  return false;
}

bool kb_buff_read_double(kb_buff_t * kb, double * out) {
  const char * string;
  uint64_t len;
  if (!kb_buff_read_string(kb, &string, &len)) return false;
  
  double value = 0;
  bool negative = false;
  bool decimal = false;
  double decScale = 0.1;
  uint64_t i;
  
  // parse the string before the possible exponent
  for (i = 0; i < len; i++) {
    char ch = string[i];
    if (ch == 'e' || !ch) break;
    if (ch == '.' && !decimal) {
      decimal = true;
    } else if (ch >= '0' && ch <= '9') {
      double number = (double)(ch - '0');
      if (decimal) {
        value += decScale * number;
        decScale /= 10.0;
      } else {
        value *= 10.0;
        value += number;
      }
    } else if (ch == '-' && i == 0) {
      negative = true;
    } else {
      return false;
    }
  }
  
  // check if the string ended properly
  if (i == 0) return false;
  if (string[i - 1] == '.' || string[i - 1] == '-') return false;
  if (string[i] != 'e') {
    (*out) = value * (negative ? -1.0 : 1.0);
    return true;
  }
  
  // read the exponent
  int exponent = 0;
  if (!_parse_int(&string[i + 1], &exponent)) return false;
  
  double multiple = 1.0;
  double product = 10.0;
  bool negExp = exponent < 0;
  if (negExp) exponent *= -1;
  
  while (exponent) {
    if (exponent & 1) multiple *= product;
    product *= product;
    exponent >>= 1;
  }
  
  if (negExp) multiple = 1.0 / multiple;
  
  (*out) = value * (negative ? -1.0 : 1.0) * multiple;
  return true;
}

bool kb_buff_read_int(kb_buff_t * kb, uint8_t lenLen, int64_t * out) {
  // For now, I am making the assumption that you are on an x86 or a different
  // system that uses little endian (i.e. not ARM, probably)
  
  if (lenLen == 1) {
    // 32-bit integer
    if (kb->off + 4 > kb->len) return false;
    int32_t buf = *((const int32_t *)(kb->buff + kb->off));
    kb->off += 4;
    (*out) = (int64_t)buf;
    return true;
  } else if (lenLen == 2) {
    // 64-bit integer
    if (kb->off + 8 > kb->len) return false;
    (*out) = *((const int64_t *)(kb->buff + kb->off));
    kb->off += 8;
    return true;
  }
  return false;
}

bool kb_buff_read_data(kb_buff_t * kb,
                       uint8_t lenLen,
                       const void ** start,
                       uint64_t * lenOut) {
  if (lenLen + 1 + kb->off > kb->len) return false;
  uint32_t len = 0;
  if (lenLen == 0) {
    len = (uint32_t) *((const uint8_t *)(kb->buff + kb->off));
  } else if (lenLen == 1) {
    len = (uint32_t) *((const uint16_t *)(kb->buff + kb->off));
  } else if (lenLen == 2) {
    len = (uint32_t) *((const uint16_t *)(kb->buff + kb->off));
    uint8_t next = *((const uint8_t *)(kb->buff + kb->off + 2));
    len |= ((uint32_t)next) << 0x10;
  } else if (lenLen == 3) {
    len = *((const uint32_t *)(kb->buff + kb->off));
  }
  
  (*start) = kb->buff + kb->off + lenLen + 1;
  kb->off += lenLen + 1;
  if (len + kb->off > kb->len) return false;
  kb->off += len;
  
  (*lenOut) = len;
  return true;
}

bool kb_buff_read_key(kb_buff_t * kb, char * out, uint64_t max) {
  uint64_t i;
  for (i = 0; kb->off < kb->len && i < max; i++) {
    uint8_t ch = *((const uint8_t *)(kb->buff + kb->off));
    kb->off++;
    if (ch & 0x80) {
      if (i + 1 >= max) return false;
      out[i] = ch ^ 0x80;
      out[i + 1] = 0;
      return true;
    }
    out[i] = ch;
  }
  return true;
}

static bool _parse_int(const char * buff, int * number) {
  int value = 0;
  bool negative = false;
  uint64_t i;
  for (i = 0; buff[i]; i++) {
    if (buff[i] == '-' && i == 0) {
      negative = true;
    } else if (buff[i] == '+' && i == 0) {
      continue;
    } else if (buff[i] >= '0' && buff[i] <= '9') {
      int num = (int)(buff[i] - '0');
      value *= 10;
      value += num;
    } else {
      return false;
    }
  }

  if (i == 0) return false;
  if (buff[i] == '-' || buff[i] == '+') return false;
  (*number) = value * (negative ? -1 : 1);
  return true;
}
