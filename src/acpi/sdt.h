/**
 * Finds the RSDT or XSDT and returns values from whichever one ought to be
 * used.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <memory/kernpage.h>

/**
 * Finds the RSDT or XSDT and returns true if one of those was found. The
 * following functions require you call this first.
 */
bool acpi_sdt_find();

/**
 * Returns the number of tables contained in the *SDT. This method requires
 * that the virtual memory subsystem has been configured.
 */
uint64_t acpi_sdt_count_tables();

/**
 * Returns a table from the *SDT. The address which is returned is a physical
 * one, so you will have to map it or use one of the other acpi_ functions.
 */
void * acpi_sdt_get_table(uint64_t idx);
