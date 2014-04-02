#ifndef __KEYEDBITS_TYPES_H__
#define __KEYEDBITS_TYPES_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  unsigned typeField : 3;
  unsigned reserved : 2;
  unsigned lenLen : 2;
  unsigned nullTerm : 1;
} __attribute__((packed)) kb_header_t;

/**
 * For encoding/decoding in a fixed-size buffer.
 */
typedef struct {
  void * buff;
  uint64_t len;
  uint64_t off;
} __attribute__((packed)) kb_buff_t;

typedef enum {
  KeyedBitsTypeTerminator = 0,
  KeyedBitsTypeString = 1,
  KeyedBitsTypeArray = 2,
  KeyedBitsTypeDictionary = 3,
  KeyedBitsTypeNull = 4,
  KeyedBitsTypeData = 5,
  KeyedBitsTypeInteger = 6,
  KeyedBitsTypeFloat = 7
} KeyedBitsType;

#endif