#include <keyedbits/validation.h>

bool kb_validate_header(kb_header_t * header) {
  if (header->reserved != 0) return false;
  switch (header->typeField) {
    case KeyedBitsTypeTerminator:
      if (header->lenLen || header->nullTerm) return false;
      break;
    case KeyedBitsTypeString:
    case KeyedBitsTypeDictionary:
    case KeyedBitsTypeArray:
    case KeyedBitsTypeFloat:
      if (header->lenLen || !header->nullTerm) return false;
      break;
    case KeyedBitsTypeInteger:
      if (header->lenLen != 1 && header->lenLen != 2) return false;
      break;
    case KeyedBitsTypeData:
      if (header->nullTerm) return false;
      break;
    default:
      break;
  }
  return true;
}