# anlock

This library provides a queue-free, very lightweight locking mechanism for user applications and OS kernels. The lock mechanism uses atomic operations on 64-bit values to provide ordered, first-in-first-out mutual exclusion for multi-processor applicaitons.

# Usage

To initialize a lock, create a pointer to a 64-bit value and cast it to `anlock_t`.  This variable (which we will call `lock`) is what you pass to all anlock functions.  In order to 0 out the lock, use `anlock_initialize(lock)`.  Then, to obtain the lock, use `anlock_lock(lock)` and to unlock it use `anlock_unlock(lock)`.

You will note that `anlock_lock` uses 100% CPU while it waits for a lock.  This is fine if your locks will only take a few microseconds to obtain, but it becomes problematic if a resource is required for a significant amount of time.  To prevent this issue, use `anlock_lock_waiting(lock, data, function)`.  This will constantly call `function(data)` while waiting for the lock.  If you wish to use less CPU, do some sort of sleep inside your `function`.

# The algorithm

The algorithm used by `anlock` is very straightforward.

A lock is represented as a 64-bit integer.  The lower 32-bits stores the number of threads currently attempting to use the lock (including the thread that currently owns the lock).  The upper 32-bits gets incremented every time the lock is released, and may rap around back to zero if the lock is heavily used.

When a thread wants to seize the lock, it uses an atomic fetch-and-add instruction to increment the lock's value and get the old value (let's call the old value x).  As long as less than 2^32 threads are trying to seize the lock at once, the add part of this instruction will simply increment the lower 32-bits&ndash;i.e. the threads waiting field.  Now for the fun part: the seizing thread creates a 32-bit integer out of the upper half of x, and adds to this value the lower 32-bits of x.  Finally, the seizing thread loops, constantly reading the new value of the lock, waiting until the upper 32-bits is the sum it just calculated.  When they are equal, the lock is owned by the current thread.

The reason that this whole proceedure works is because of the nature of *unlocking*.  When a thread wishes to release a lock, it uses an atomic addition to increment the value of the lock by 0xffffffff.  This effectively subtracts one from the threads waiting field while adding one (because of carrying) to the threads completed field.

# Drawbacks

While anlock does provide a good FIFO locking mechanism, the lack of a waiting queue does have its drawbacks.  For one thing, a waiting thread has no way of withdrawing its interest in the lock.  This could be bad, for instance, if the kernel thread of a process in an OS is waiting for a lock to obtain a resource and the process is killed while this is happenening.  With anlock, the kernel thread must wait until it obtains the lock, and only *then* can the process be properly terminated.

Because of the operand size of 64-bits, anlock only supports up to 2^32 simultaneous clients for each lock.  This should not be a problem on today's computers (in fact, 32-bit locks would have probably been fine), but it may be a problem some day.

One other potential disadvantage with this locking mechanism is that is does not support recursion.  In order to support recursive locks, there must be some sort of thread structure in place in the very least.

# Advantages

One obvious advantage of a queue-free lock is that it is extremely lightweight.  Since an anlock is 64-bits, it is very easy to initialize locks in memory and use them at whim.

This lock has the advantage over spinlocks that it is FIFO.  This means that a thread will never wait for a lock while other threads constantly seize and release the lock again and again.
