# anidxset

This is a small tool for efficiently maintaining a set of unsigned 64-bit integers in an operating system kernel which uses page-by-page allocation. Applications of this include but are not limited to: storing a list of available file descriptors; keeping track of which blocks of memory are free in a task; quickly finding available process identifiers.

# Usage

Create a new `anidxset_root_t` structure by calling `anidxset_initialize()`.  The second argument is a function pointer to an allocation function.  The allocator should allocate a 4KB page and return its address.  The third argument is a function pointer to a freeing function.  The free function should take an address returned by the allocator and relinquish ownership of it.

To get a new index, call `anidxset_get` and pass in the `anidxset_root_t` pointer. Initially, indexes will be returned in order.  Once you have called `anidxset_put` to relinquish indexes, the relinquished indexes will first be returned before new sequential ones are thrown into the mix.

A note on `anidxset_put()`.  If your allocator function fails to allocate a page and returns `(void *)0`, then `anidxset_put()` may fail and return `0` to indicate the error.  Otherwise it returns `1`.
