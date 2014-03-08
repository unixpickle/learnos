#ifndef __LIBKERN_BASE_H__
#define __LIBKERN_BASE_H__

#include "stdint.h"

/**
 * Output to a processor pin
 */
void outb(uint32_t port, uint8_t byte);

/**
 * Infinite hlt loop.
 */
void hang();

/**
 * hlt - wait for next interrupt
 */
void halt();

/**
 * Get the CPUID information
 */
uint32_t cpuid(uint32_t level, uint32_t * ebx, uint32_t * ecx, uint32_t * edx);

/**
 * Read a model-specific register.
 * The result is technically EDX:EAX
 */
unsigned long long msr_read(uint32_t selector);

/**
 * Write a model specific register.
 */
void msr_write(uint32_t selector, uint64_t value);

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

/**
 * Print `count` bytes of the stack. Preserves all registers.
 */
void stack_log(uint64_t count);

#endif
