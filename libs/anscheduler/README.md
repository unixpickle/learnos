# anscheduler

The purpose of this project is to provide an abstract task scheduler that is testable and can be integrated into operating systems.

**Note:** Although the scheduler *is* architecture independent, it does rely on some features, such as the existance of GCC's __sync builtins.  Additionally, the scheduler assumes a page size of 4K.  If your architecture uses pages smaller than this, it will have to map pages in bulk (for example, if it uses 1K pages instead of 4K pages).

### Features

* Automatic time slicing (with abstracted preemption)
* Inter-process communication via "sockets"
* Kill and launch tasks and threads any time
* Interrupt-forwarding for one "interrupt daemon" task
* Support for dynamic on-request stack allocation

### Upcoming Features

* Heap allocation for tasks
* Heap paging with designated paging daemon

# Subtree

The `anidxset` subtree was setup as follows:

    git remote add anidxset https://github.com/unixpickle/anidxset.git
    git fetch anidxset
    git checkout -b anidxset anidxset/master
    git checkout master
    git read-tree --prefix=lib/anidxset/ -u anidxset

To pull upstream changes from `anidxset`, use:

    git checkout anidxset
    git pull
    git checkout master
    git merge --squash -s subtree --no-commit anidxset

# Tests

The `test/` directory contains some tests for this scheduler. Because the scheduler involves lots of context switching, the test is architecture specific to x86-64. You will see that the test code has lots of inline assembly, and even some 64-bit NASM source.
