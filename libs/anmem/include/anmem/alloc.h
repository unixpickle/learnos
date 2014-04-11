#ifndef __ANMEM_ALLOC_H__
#define __ANMEM_ALLOC_H__

#include "types.h"

/**
 * Allocate `len` physical pages, aligned to a `len` pages boundary.
 * @return If no region could be allocated (either because of alignment or
 * space issues) NULL is returned. Otherwise, a pointer is returned. The
 * pointer is in virtual space.
 */
void * anmem_alloc_aligned(anmem_t * mem, uint64_t len);

/**
 * Free an aligned physical buffer of a certain length in virtual space.
 */
void anmem_free_aligned(anmem_t * mem, void * buffer, uint64_t len);

/**
 * Allocate a page (aligned) segment of memory from the fastest source.
 */
void * anmem_alloc_page(anmem_t * mem);

/**
 * Free a page of memory which was allocated with anmem_alloc_page().
 */
void anmem_free_page(anmem_t * mem, void * page);

#endif