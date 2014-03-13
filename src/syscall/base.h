
/**
 * The system call for user applications to print text to the screen.
 */
void syscall_print();

/**
 * Inaccurate PIT sleep.
 */
void syscall_sleep();

/**
 * Waits until an incoming interrupt occurs and then returns the INT_FLAGS.
 */
void syscall_getint();

/**
 * Read or write from a CPU port.
 */
void syscall_pinio();

