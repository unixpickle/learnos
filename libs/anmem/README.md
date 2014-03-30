# Abstract

*anmem* combines my three other projects, [analloc](https://github.com/unixpickle/analloc), [anpages](https://github.com/unixpickle/anpages), and [anlock](https://github.com/unixpickle/analloc) to make a multipurpose memory allocator for operating system kernels. This allocator provides page allocation for basic kernel structures, aligned block allocation for PCI buffers, and dynamic, easy to use free routines.

# Requirements

In order for *anmem* to work to its fullest capacity, you must use a kernel virtual memory mapping such that the system's physical memory is mapped (with no gaps) to linear virtual memory. Although *anmem* deals with a virtual memory mapping, it uses the information you pass it at configuration time to provide you with aligned *physical* addresses. This is to satisfy the PCI bus memory address requirements.

# The Algorithm

When you initialize *anmem*, you provide a list of physical memory regions. You also provide the `maxControllable` size. *anmem* uses these arguments to find a chunk of memory of size `maxControllable` which is aligned on a boundary of `maxControllable`. It then sets up a buddy allocator in this chunk of memory so that any `n` page chunk of memory (where `n` is a power of 2) is aligned to `n` pages. This is just what PCI requires.

Take an example: when specify 128MB as `maxControllable`, *anmem* tries to find a 128MB chunk of memory aligned on a 128MB boundary. If no such region exists below 4GB (the "maximum" boundary for alignable PCI memory), then the configuration process begins to look for 64MB regions, and then 32MB regions, etc., until it has found a way to allocate aligned blocks. In a normal, modern 64-bit workstation, a 128MB aligned block of memory is usually present, so no splitting is necessary.

# Subtrees

Subtrees were added like the following example for `analloc`

    git remote add analloc https://github.com/unixpickle/analloc.git
    git fetch analloc
    git checkout -b analloc analloc/master
    git checkout master
    git read-tree --prefix=libs/analloc -u analloc

Update subtrees as follows:

    git checkout analloc
    git pull
    git checkout master
    git merge --squash -s subtree --no-commit analloc
