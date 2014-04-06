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
 * The base address of all the global variables.
 */
#define VARBASE 0x100000

/**
 * Lower word = x, upper word = y of cursor.
 */
#define CURSOR_INFO ((volatile short *)VARBASE)

/**
 * Contains a pointer to the Multiboot information data structure
 */
#define MBOOT_INFO (*((uint32_t **)(VARBASE + 4)))

/**
 * The highest physical page that is mapped. When the kernpage subsystem has
 * been initialized, this is used to store the number of pages allocated.
 */
#define LAST_PAGE (*((volatile unsigned long long *)(VARBASE + 0xc)))

/**
 * The highest virtual page that is mapped.
 */
#define LAST_VPAGE (*((volatile unsigned long long *)(VARBASE + 0x14)))

/**
 * A 1-byte counter for the map which follows. No longer used.
 */
#define PHYSICAL_MAP_COUNT (*((volatile unsigned char *)(VARBASE + 0x1c)))

/**
 * A 1-byte flag; if 1, then the kernpage_ functions may be used.
 * Not used for anything.
 */
#define KERNPAGE_ENABLED (*((volatile unsigned char *)(VARBASE + 0x1d)))

/**
 * An array of structures like such:
 * struct map {
 *   uint64_t basePage;
 *   uint64_t pagesLong;
 * }
 * This can have up to 0x100 entries, so the size is 0x1000 bytes.
 */
#define PHYSICAL_MAP_ADDR (VARBASE + 0x1e)

/**
 * Free space starts at 0x20101e and ends at 0x20102e.
 * I am using 0x201020 to store the timestamp because it's aligned.
 */

#define SYS_TIMESTAMP (VARBASE + 0x1020)

/**
 * The address which contains the "IDTR" to load with lidt
 */
#define IDTR_PTR (VARBASE + 0x102e)

/**
 * A counter which the PIT will increment 100 times a second
 * during boot in order to keep the system time.
 */
#define PIT_TICK_COUNT (*((volatile uint64_t *)(VARBASE + 0x1038)))

/**
 * The address of the IDT to be used by the whole system.
 * 16*256 = 4096 bytes, so one page for this table is reserved.
 */
#define IDT_PTR (VARBASE + 0x2000)

/**
 * One 0x1000 byte page containing the MADT table.
 * No longer in use.
 */
#define ACPI_MADT_PTR (VARBASE + 0x3000)

/**
 * No longer in use.
 */
#define ANPAGES_STRUCT (VARBASE + 0x5000)

/**
 * No longer in use.
 */
#define ANPAGES_LOCK (VARBASE + 0x5020)

/**
 * No longer in use. This should be used for other things.
 */
#define CPU_INFO_FIRST (*((volatile uint64_t *)(VARBASE + 0x5028)))

/**
 * No longer in use. This should be used for other things.
 */
#define CPU_LIST_LOCK (VARBASE + 0x5030)

/**
 * The 0x40 byte structure containing the task list.
 */
#define TASK_LIST_PTR (VARBASE + 0x5038)

/**
 * The GDT which is used for processors after SMP is started up. This is
 * needed to have a TSS for each CPU core.
 */
#define DYNAMIC_GDT (VARBASE + 0x10000)

/**
 * 0x67f98 bytes, round to 0x70 pages
 */
#define TSS_ENTRIES (VARBASE + 0x20000)

/**
 * A list of anmem_section_t structures. They are currently 0x350 bytes, so
 * this list can hold 0x4D of them for a total of about 0x1000 bytes
 * (really 0xff1 bytes).
 */
#define ANMEM_SECTIONS (VARBASE + 0x90000)

/**
 * Set CR3=PML4_START to enable kernel paging.
 */
#define PML4_START 0x300000

