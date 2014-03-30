#ifndef __ANMEM_TYPES_H__
#define __ANMEM_TYPES_H__

#include <anpages.h>
#include <analloc.h>
#include <anmem_consts.h> // int types, etc.

/**
 * Represents a section of memory which is controlled by a particular
 * allocation algorithm.
 */
typedef struct {
  uint8_t type; // 0 = anpages, 1 = analloc
  
  // the first page which this allocator covers
  unsigned long long start : 0x38;
  
  // the number of pages which this allocator covers
  uint64_t len;
  
  // used before allocating anything on this allocator
  uint64_t lock;
  
  union {
    anpages_struct_t anpagesRoot;
    analloc_struct_t anallocRoot;
  };
} __attribute__((packed)) anmem_section_t;

/**
 * The main type used by anmem. At configuration time, this structure is
 * initialized, and after that point it is passed around to various anmem
 * functions as needed.
 */
typedef struct {
  anmem_section_t * allocators;
  uint64_t count;
  uint64_t maximum;
} __attribute__((packed)) anmem_t;

/**
 * Represents an array of `structCount` structures, each of size `structSize`.
 * Each struct contains a 64-bit native endian page count at offset
 * `sizeOffset` in the struct. Additionally, each structure contains the
 * `physicalPageOffset` of this range of memory in the physical address space.
 * The pointer to the first struct is `structs`.
 */
typedef struct {
  void * structs;
  uint64_t sizeOffset;
  uint64_t physPageOffset;
  uint64_t structSize;
  uint64_t structCount;
} __attribute__((packed)) anmem_config_t;

typedef void (* anmem_free_t)(void * userInfo, void * ptr);

#endif