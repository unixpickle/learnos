/**
 * Lower word = x, upper word = y of cursor.
 */
#define CURSOR_INFO ((volatile short *)0x200000)

/**
 * Contains a pointer to the Multiboot information data structure
 */
#define MBOOT_INFO (*((uint32_t **)0x200004))

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
 * The (virtual kernpage) address which 0xFEE... is mapped to
 */
#define LAPIC_PTR (*((volatile void **)0x20101e))

/**
 * The (virtual kernpage) address which 0xFEC... is mapped to
 */
#define IOAPIC_PTR (*((volatile void **)0x201026))

/**
 * The address which contains the "IDTR" to load with lidt
 */
#define IDTR_PTR 0x20102e

/**
 * A counter which the PIT will increment 100 times a second
 * during boot in order to keep the system time.
 */
#define PIT_TICK_COUNT (*((volatile uint64_t *)0x201036))

/**
 * The address of the IDT to be used by the whole system.
 * 16*256 = 4096 bytes, so one page for this table is reserved.
 */
#define IDT_PTR 0x202000

/**
 * One 0x1000 byte page, containing the MADT data structure.
 */
#define ACPI_MADT_PTR 0x203000

/**
 * Set CR3=PML4_START to enable kernel paging.
 */
#define PML4_START 0x300000

