#ifndef __ANMEM_CONFIG_H__
#define __ANMEM_CONFIG_H__

#include "types.h"

/**
 * Configure the anmem allocator. This must be called after you have mapped
 * physical memory to contiguous virtual memory.
 *
 * @param config Information about the physical memory's topology.
 *
 * @param mem The allocators array should point to a static fixed-size array.
 * The maximum field should be the maximum number of allocators that may be
 * stored in the array. If more allocators than this are necessary, the
 * operation will fail and false will be returned.
 *
 * @param maxControllable A number n such that 2^n is the maximum number of
 * pages which you require to be `analloc` pages. The value I currently plan
 * to use here is 16, for a maximum of 0x10000 pages, or 268,435,456 bytes.
 *
 * @param pageSkip The number of initial pages to skip because they are
 * reserved for page tables or some system use. Usually, this will include the
 * first megabyte (0x100 pages).
 *
 * @return false if more than the acceptable number of allocators are needed,
 * or true if the configuration was successful.
 */
bool anmem_configure(anmem_config_t * config,
                     anmem_t * mem,
                     uint64_t maxControllable,
                     uint64_t pageSkip);

/**
 * Call this after calling anmem_configure(). This will actually initialize
 * the allocators which were reserved by anmem_configure().
 * @return a `false` return means a fatal error.
 */
bool anmem_init_structures(anmem_t * mem);

/**
 * Get the number of pages which are being used for linear, aligned
 * memory allocation. Ideally, but not always, this will be equal to 2^n
 * where n is the value you passed for `maxControllable` to anmem_configure.
 */
uint64_t anmem_analloc_count(anmem_t * mem);

#endif