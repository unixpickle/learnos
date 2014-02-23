#include <shared/types.h>

/**
 * Output to a processor pin
 */
void outb64(unsigned int port, unsigned char byte);

/**
 * Infinite hlt loop.
 */
void hang64();

/**
 * hlt - wait for next interrupt
 */
void halt64();

/**
 * Get the CPUID information
 */
unsigned int cpuid(unsigned int level, unsigned int * ebx, unsigned int * ecx, unsigned int * edx);

/**
 * Read a model-specific register.
 * The result is technically EDX:EAX
 */
unsigned long long msr_read(unsigned int selector);

/**
 * Write a model specific register.
 */
void msr_write(unsigned int selector, unsigned long long value);

/**
 * Calls invlpg on a virtual page.
 */
void invalidate_page(uint64_t virtualIndex);

/**
 * Calls sti.
 */
void enable_interrupts();

/**
 * Calls cli.
 */
void disable_interrupts();

