/**
 * Lower word = x, upper word = y of cursor.
 */
#define CURSOR_INFO ((volatile short *)0x200000)

/**
 * Contains a pointer to the Multiboot information data structure
 */
#define MBOOT_INFO (*((volatile uint32_t **)0x200004))

/**
 * The highest physical page that is mapped.
 */
#define LAST_PAGE (*((volatile unsigned long long *)0x20000c))

/**
 * The highest virtual page that is mapped.
 */
#define LAST_VPAGE (*((volatile unsigned long long *)0x200014))

/**
 * A 1-byte counter for the map which follows.
 */
#define PHYSICAL_MAP_COUNT (*((volatile unsigned char *)0x20001c))

/**
 * A 1-byte flag; if 1, then the kernpage_ functions may be used.
 */
#define KERNPAGE_ENABLED (*((volatile unsigned char *)0x20001d))

/**
 * An array of structures like such:
 * struct map {
 *   uint64_t basePage;
 *   uint64_t pagesLong;
 * }
 * This can have up to 256 entries, so the size is 4096 bytes.
 */
#define PHYSICAL_MAP_ADDR 0x20001e

/**
 * The (virtual kernpage) address used to control the APIC.
 */
#define APIC_PTR (*((volatile void **)0x201016))

/**
 * Set CR3=PML4_START to enable kernel paging.
 */
#define PML4_START 0x300000

