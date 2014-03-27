SECTIONS {

  . = 1M;

  .text BLOCK(4K) : ALIGN(4K) {
    *(.multiboot)
    *(.text)
  }
  .rodata BLOCK(4K) : ALIGN(4K) {
    *(.rodata)
  }
  .data BLOCK(4K) : ALIGN(4K) {
    *(.data)
    *(COMMON)
    *(.bss)
  }

}
