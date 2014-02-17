#include <shared/types.h>
#include <shared/addresses.h>

/**
 * Returns whether a page index in physical memory has been
 * identity mapped.
 */
bool kernpage_is_mapped(unsigned long page);

/**
 * Identity maps a page in physical memory. This will increase
 * the value at END_PAGE_INFO and may overwrite structures
 * directly after the page info if it exists, so this is
 * only to be called early on in kernel execution.
 */
void kernpage_create_identity_map(unsigned long page);

