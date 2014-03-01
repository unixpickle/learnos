#include <stdint.h>
#include <anpages.h>

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
uint64_t kernpage_calculate_virtual(uint64_t phys);

/**
 * Calculates the physical page that a virtual page maps to.
 */
uint64_t kernpage_calculate_physical(uint64_t virt);

/**
 * Returns `true` if a virtual page is mapped, `false` if not.
 */
bool kernpage_is_mapped(uint64_t virt);

/**
 * Maps a virtual page to point to a physical page.
 */
bool kernpage_map(uint64_t virt, uint64_t phys);

/**
 * Does a backwards search through the kernel page tables to find a virtual
 * page which maps to a physical page.
 */
bool kernpage_lookup_virtual(uint64_t phys, uint64_t * virt);

/**
 * Uses the underlying anpages structure to allocate a new virtual page of 
 * memory.
 */
uint64_t kernpage_alloc_virtual();

/**
 * Uses the underlying anpages structure to free a virtual page of memory.
 */
void kernpage_free_virtual(uint64_t virt);

/**
 * Copies a piece of data from physical memory to virtual memory. This may be
 * slow, so watch out.
 */
void kernpage_copy_physical(void * dest, const void * physical, uint64_t len);