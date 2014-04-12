# analloc

This is a completely stand-alone memory allocator which uses [Buddy memory allocation](http://en.wikipedia.org/wiki/Buddy_memory_allocation) under the hood. The point of *analloc* is to provide a flexible, lightweight allocator to operating system kernels.

# Usage

Initializing *analloc* is a simple matter. The usage is roughly as follows:

 * Reserve a chunk of physical memory for use with *analloc*. This chunk may begin with some pre-allocated memory.
 * Allocate some memory for an `analloc_t` structure. This memory may be part of the pre-allocated memory in the physical memory chunk you plan to use for *analloc*.
 * Call `analloc_with_chunk()`. You pass in
   * a pointer to the `analloc_t` structure
   * a pointer to the absolute beginning of the buffer
   * the number of bytes *used* by your pre-allocated data at the beginning of the buffer. If you use another location to store the `analloc_t`, this may very well be 0.
   * the *page* size. This specifies the minimum amount of bytes the user should be able to allocate. See the note on *Page Size* below for more.
 * Calculate the number of bytes actually used by *analloc*.  This is equal to `context->page << context->depth` where `context` is your `analloc_t`.

To allocate some data, use `analloc_alloc()`, to free data use `analloc_free()`, and to resize data, use `analloc_realloc()`.

# Things to Note

### Page Size

When you initialize an allocator, you specify a `page` argument.  Whenever a call to `analloc_alloc()` or `analloc_realloc()` succeeds, the allocated chunk of memory will have length of the form `2^n * page` where `n` is some non-negative integer.  This means, for instance, that if your page size is 4096, and you try to allocate 4097 bytes, you will be given a chunk of 8192 bytes.  This will not be a problem in my operating system, where mainly single pages will be allocated at once.

### Synchronization

The `analloc_t` structure is designed for single-threaded access.  If you plan on having multiple threads all capable of allocating memory (as I do in my OS), you will need a separate mutex system.

### I don't want to store the damn size

Call `analloc_mem_size` and pass in a pointer to get its size.  I still encourage keeping track of sizes if you can, since that reduces clock cycles.

# License

This software is under the GNU GPL v1.
