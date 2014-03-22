# TODO

 * Get rid of startup thread and replace with page based allocation
 * Implement a socket system
 * Create PS/2 driver
 * Wake up thread when an interrupt is received
 * Replace all appropriate `uint64_t` with `page_t`
 * Disable PIT after LAPIC timers are initialized

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
