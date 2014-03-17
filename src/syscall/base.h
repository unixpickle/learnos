/**
 * When a system call occurs, this method is called. It uses the rax register
 * to determine the system call, and then other registers as arguments.
 */
void syscall_interrupt();

