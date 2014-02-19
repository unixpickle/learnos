#include <shared/types.h>

/**
 * This must be called during initialization of the kernel and never again.
 */
void kernpage_initialize();

/**
 * Prevents further changes to the kernel page table and physical address
 * space. This should be called once during kernel startup after all
 * needed DMA buffers have been reserved.
 */
void kernpage_lockdown();

/**
 * Uses LAST_PAGE to compute the next free *physical* page in memory.
 * @return 0 on error
 */
uint64_t kernpage_next_physical();

/**
 * Calculates the virtual page index for a physical page index.
 * @return 0 on error
 */
uint64_t kernpage_calculate_virtual(uint64_t physicalPage);

/**
 * Calculates the physical page index for a virtual page.
 * @return 0 on error
 */
uint64_t kernpage_calculate_physical(uint64_t virtualPage);

/**
 * Allocates the next physical page and returns its index.
 * @return 0 on error
 */
uint64_t kernpage_allocate_physical();

/**
 * Allocates the next contiguous chain of `count` pages of physical
 * memory and returns the index of the first such page.
 * @return 0 on error
 */
uint64_t kernpage_allocate_contiguous(uint64_t count);

/**
 * @return true if virtualPage is already mapped, false otherwise.
 */
bool kernpage_is_mapped(uint64_t virtualPage);

/**
 * Maps virtualPage to the physical address physicalPage.
 * @return false on failure to allocate new page tables, true otherwise.
 */
bool kernpage_map(uint64_t virtualPage, uint64_t physicalPage);

/**
 * Returns the next virtual page that is not mapped.
 */
uint64_t kernpage_next_virtual();

