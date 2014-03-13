/**
 * The pointer+limit structure to the 64-bit GDT structure.
 * This is for processor initialization
 */
#define GDT64_PTR 0x5ff6

/**
 * The address where new processors will be initialized to.
 * This address must be page-aligned.
 */
#define PROC_INIT_PTR 0x6000

/**
 * Lower word = x, upper word = y of cursor.
 */
#define CURSOR_INFO ((volatile short *)0x200000)

/**
 * Contains a pointer to the Multiboot information data structure
 */
#define MBOOT_INFO (*((uint32_t **)0x200004))

/**
 * The highest physical page that is mapped. When the kernpage subsystem has
 * been initialized, this is used to store the number of pages allocated.
 */
#define LAST_PAGE (*((volatile unsigned long long *)0x20000c))

/**
 * The highest virtual page that is mapped.
 */
#define LAST_VPAGE (*((volatile unsigned long long *)0x200014))

/**
 * A 1-byte counter for the map which follows. No longer used.
 */
#define PHYSICAL_MAP_COUNT (*((volatile unsigned char *)0x20001c))

/**
 * A 1-byte flag; if 1, then the kernpage_ functions may be used.
 * Not used for anything.
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
 * No longer used.
 */
#define LAPIC_PTR (*((volatile void **)0x20101e))

/**
 * The (virtual kernpage) address which 0xFEC... is mapped to
 * No longer used.
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
#define PIT_TICK_COUNT (*((volatile uint64_t *)0x201038))

/**
 * The address of the IDT to be used by the whole system.
 * 16*256 = 4096 bytes, so one page for this table is reserved.
 */
#define IDT_PTR 0x202000

/**
 * One 0x1000 byte page containing the MADT table
 */
#define ACPI_MADT_PTR 0x203000

/**
 * No longer in use.
 */
#define ANPAGES_STRUCT 0x205000

/**
 * No longer in use.
 */
#define ANPAGES_LOCK 0x205020

/**
 * No longer in use. This should be used for other things.
 */
#define CPU_INFO_FIRST (*((volatile uint64_t *)0x205028))

/**
 * No longer in use. This should be used for other things.
 */
#define CPU_LIST_LOCK 0x205030

/**
 * The 0x40 byte structure containing the task list.
 */
#define TASK_LIST_PTR 0x205038

/**
 * The GDT which is used for processors after SMP is started up. This is
 * needed to have a TSS for each CPU core.
 */
#define DYNAMIC_GDT 0x210000

/**
 * 0x67f98 bytes, round to 0x70 pages
 */
#define TSS_ENTRIES 0x220000

/**
 * Set CR3=PML4_START to enable kernel paging.
 */
#define PML4_START 0x300000

/**
 * Page addresses in process address space.
 */

/**
 * Kernel stacks take up 1 page each, and with our limit of 0x100000 threads,
 * this is a total of 0x100000 pages long.
 */
#define PROC_KERN_STACKS 0x400L

/**
 * User stacks are each 0x100 pages long, and there's a hard limit of 0x100000
 * of them, just like kernel stacks.
 */
#define PROC_USER_STACKS 0x100400L

/**
 * Each socket is 4 pages long, and once again we have a 0x100000 limit.
 */
#define PROC_SOCKET_BUFFS 0x10100400L

/**
 * The code buffer is of a hard limit of 0x100000 pages long (4 GiB).
 */
#define PROC_CODE_BUFF 0x10500400L

/**
 * The heap can be as long as it could possibly want to be.
 * Note that this address, in terms of pages, is an entire 1TB
 * up in memory already. Our hard limit with 4-level paging is 256TB.
 */
#define PROC_HEAP_BUFF 0x10600400L

