#ifndef __ANALLOC_H__
#define __ANALLOC_H__

#include "anbtree.h"

typedef struct {
  void * mem;
  anbtree_ptr tree;
  uint64_t page;
  uint64_t depth;
} __attribute__((packed)) analloc_struct_t;

typedef analloc_struct_t * analloc_t;

/**
 * Creates a new allocator that encloses a certain buffer of memory.
 * @param alloc - The allocator to be used.
 * @param ptr - The buffer to use
 * @param total - The total length of the buffer pointed to by `ptr`
 * @param used - Sometimes, a kernel or program wishes to use an allocator,
 *               but already has some data at the beginning of the desired
 *               memory chunk.  The `used` argument specifies the number
 *               of bytes at the beginning of the buffer pointed to by `ptr`
 *               that should be permanently "allocated".
 * @param page - The size of the smallest unit of data measure.
 * @return 1 if the creation was successful, 0 on error.
 */
uint8_t analloc_with_chunk(analloc_t alloc,
                           void * ptr,
                           uint64_t total,
                           uint64_t used,
                           uint64_t page);

/**
 * Allocates a chunk of memory in an allocator.
 * @param alloc - The allocator to use.
 * @param sizeInOut - On input, this should be the amount of bytes that the
 *                    user wishes to allocate.  On output, this is the number
 *                    of bytes actually allocated.  This will either be larger
 *                    than the length requested, or 0 on error.
 * @param high - 1 to allocate a high chunk, 0 for a low chunk
 * @return A pointer to the allocated data, or (void *)0 on error.
 */
void * analloc_alloc(analloc_t alloc, uint64_t * sizeInOut, uint8_t high);

/**
 * Free a chunk of memory.
 * @param alloc - The allocator to use.
 * @param buffer - The buffer to be freed.  This must be a value which was
 *                 previously returned by analloc_alloc().
 * @param length - The length of the buffer being freed.  This must either
 *                 be the length passed to `sizeInOut`, or the size returned
 *                 from it when analloc_alloc() was called.
 */
void analloc_free(analloc_t alloc, void * buffer, uint64_t length);

/**
 * Resize a chunk of memory.
 * @param alloc - The allocator to use.
 * @param buffer - The buffer to be resized.
 * @param length - The current length of the buffer.
 * @param newLen - The requested new length of the buffer.  On return, this
 *                 will be >= the input value, or 0 on reallocation error.
 * @param high - See `high` on analloc_alloc().
 * @return a pointer to newly sized data, or (void *)0 on error.
 * @discussion If an error occurs, the buffer for which a resize was requested
 * will not be free'd so that future attempts may be made.
 */
void * analloc_realloc(analloc_t alloc,
                       void * buffer,
                       uint64_t length,
                       uint64_t * newLen,
                       uint8_t high);

/**
 * Returns the size of an allocated chunk in memory.
 * @param buffer - The buffer pointing to the beginning of the chunk. If
                   `buffer` is not allocated, behavior is undefined.
 */
uint64_t analloc_mem_size(analloc_t alloc, void * buffer);

/**
 * Pass in a pointer to any part of a buffer allocated with analloc, and get
 * back the pointer to the *beginning* of the buffer.
 */
void * analloc_mem_start(analloc_t alloc, void * buffer);

#endif