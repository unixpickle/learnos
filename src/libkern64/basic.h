/**
 * Output to a processor pin
 */
void outb64(unsigned int port, unsigned char byte);

/**
 * Infinite hlt loop.
 */
void hang64();

/**
 * Get the CPUID information
 */
unsigned int cpuid(unsigned int level, unsigned int * ebx, unsigned int * ecx, unsigned int * edx);

/**
 * Read a model-specific register.
 * The result is technically EDX:EAX
 */
unsigned long readMSR(unsigned int selector);

/**
 * Write a model specific register.
 */
void writeMSR(unsigned int selector, unsigned long value);

