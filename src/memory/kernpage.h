#ifndef __KERNPAGE_H__
#define __KERNPAGE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t size;
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
} __attribute__((packed)) mmap_info;

typedef struct {
  uint64_t start;
  uint64_t length;
} __attribute__((packed)) kernpage_info;

/**
 * Initializes the kernel page tables to map to the entire physical address
 * space.
 */
void kernpage_initialize();

/**
 * Calculates the virtual page which maps to a physical page.
 */
page_t kernpage_calculate_virtual(page_t phys);

/**
 * Calculates the physical page that a virtual page maps to.
 */
page_t kernpage_calculate_physical(page_t virt);

/**
 * Gets the last virtual address that is mapped in the kernpage tables.
 */
page_t kernpage_last_virtual();

/**
 * Returns `true` if a virtual page is mapped, `false` if not.
 */
bool kernpage_is_mapped(page_t virt);

/**
 * Maps a virtual page to point to a physical page.
 */
bool kernpage_map(page_t virt, page_t phys);

/**
 * Does a backwards search through the kernel page tables to find a virtual
 * page which maps to a physical page.
 */
bool kernpage_lookup_virtual(page_t phys, page_t * virt);

/**
 * Uses the underlying anpages structure to allocate a new virtual page of 
 * memory.
 */
page_t kernpage_alloc_virtual();

/**
 * Uses the underlying anpages structure to free a virtual page of memory.
 */
void kernpage_free_virtual(page_t virt);

/**
 * Allocate a chunk of aligned physical memory that is `pageCount` pages long.
 */
uint64_t kernpage_alloc_pci(uint64_t pageCount);

/**
 * Free a chunk of aligned physical memory that is `pageCount` pages long.
 */
void kernpage_free_pci(uint64_t addr, uint64_t pageCount);

/**
 * Returns the number of pages allocated with kernpage_alloc_virtual().
 */
uint64_t kernpage_count_allocated();

/**
 * Copies a piece of data from physical memory to virtual memory. This may be
 * slow, so watch out.
 */
void kernpage_copy_physical(void * dest, const void * physical, uint64_t len);

#endif