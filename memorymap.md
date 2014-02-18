# Kernel Fields

These fields are set by the kernel at bootup time. Some of this information comes from multiboot information, 

0x100000 - the location where the kernel is loaded to
0x200000 - (begin OS specific flags and variables)
           cursor offset, x and y as shorts
0x200004 - a pointer to the multiboot startup structure (in first 4GB phys.)
0x200008 - the first physical address used by kernel page tables (always 0x300000)
0x200010 - the byte *after* the last physical address used by kernel page tables
0x300000 - the beginning of kernel identity page tables (PML4)

