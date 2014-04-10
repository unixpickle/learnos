# anmalloc

This is an easy-to-use libc compatible memory allocator.

# Dependencies

Here's a list:
 * `stdint.h` - requires `uint64_t`, `intptr_t`, etc.
 * `assert.h` - used to ensure safe memory management
 * `stdbool.h` - uses the standard `bool` type in C
 * `stddef.h` - uses the `NULL` constant
 * `string.h` - uses `memcpy()`
 * *analloc* - used for internal management of free buffers

Additionally, you must create an API for *anmalloc* to link against, and provide a header through the name `anmalloc_bindings`. Here's what you'll need:

The first two methods should follow the standard of the POSIX `brk` and `sbrk`:

    void * anmalloc_sbrk(intptr_t incr);
    int anmalloc_brk(const void * addr);

The next two methods should act like `pthread_mutex_lock` and `pthread_mutex_unlock` or similar.

**NOTE:** You may wish to use *anmalloc* in your POSIX threads implementation. If this is the case, you will want to provide a simpler locking mechanism for *anmalloc* which does **not** depend on *anmalloc*.

    typedef anmalloc_lock_t ...;
    #define ANMALLOC_LOCK_INITIALIZER ...
    
    void anmalloc_lock(anmalloc_lock_t * lock);
    void anmalloc_unlock(anmalloc_lock_t * lock);

With this interface, along with the libc dependencies listed above, *anmalloc* can work at your disposal! See the tests for detailed examples of usage.
