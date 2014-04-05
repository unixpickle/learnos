# TODO

 * Replace all appropriate `uint64_t` with `page_t`
 * Disable PIT after LAPIC timers are initialized
 * Implement a `timed` timer notification service with two threads
   * test `timed` for leaks -- multithreading hasn't been tested

# Building

You must be on a Linux system which has objcopy, ld, gcc, NASM, and make. If your system meets these requirements, simply run the following commands in the root project directory

    make deps
    make

This will generate learnos.bin, a multiboot-compliant flat kernel binary. Additionally, if you have `grub-mkrescue` installed, you may run

    make image

to generate a bootable iso file. This will also allow you to run `bochs` in this directory (since the bochsrc file points to `learnos.iso`).

# Subtrees

The `anpages` subtree was setup as follows:

    git remote add anpages https://github.com/unixpickle/anpages.git
    git fetch anpages
    git checkout -b anpages anpages/master
    git checkout master
    git read-tree --prefix=libs/anpages/ -u anpages

To pull upstream changes from `anpages`, use:

    git checkout anpages
    git pull
    git checkout master
    git merge --squash -s subtree --no-commit anpages

The same proceedure applies for the `anlock` subtree.
