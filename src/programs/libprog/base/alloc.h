#ifndef __ALLOC_H__
#define __ALLOC_H__

/**
 * This provides an (IPC Based) allocation mechanism. If the current process
 * identifier is 0, then memory will be allocated synchronously with system
 * calls. Otherwise, a socket will be opened to PID 0 and KeyedBits will be
 * used to request allocations.
 */

#include <stdbool.h>
#include <stdint.h>

#define ALLOC_DATA_BASE ((void *)0x10200000000)

/**
 * Request that new pages of usable memory or swap be mapped into the current
 * task's address space.
 * @param pageIndex The index of the page in the data section. The first page is
 * page 0, then page 1, etc.
 * @param count The number of pages to unmap.
 * @return false when memory cannot be allocated.
 * @discussion Only one call to this can be made at a time, and this method will
 * do nothing pertaining to synchronization for you!
 */
bool alloc_pages(uint64_t pageIndex, uint64_t count);

/**
 * Request that pages of usable memory be *unmapped* and thus free'd from the
 * current program's address space.
 * @param pageIndex See alloc_pages().
 * @param count The number of pages to unmap.
 * @return false when memory cannot be allocated.
 * @discussion See alloc_pages().
 */
bool free_pages(uint64_t pageIndex, uint64_t count);

#endif
