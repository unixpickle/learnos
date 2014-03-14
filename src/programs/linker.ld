SECTIONS {

  . = 0x10500400000;

  .text BLOCK(4K) : ALIGN(4K) {
    *(.entrypoint)
    *(.text)
  }
  .rodata BLOCK(4K) : ALIGN(4K) {
    *(.rodata)
  }
  .data BLOCK(4K) : ALIGN(4K) {
    *(.data)
  }
  .bss BLOCK(4K) : ALIGN(4K) {
    *(COMMON)
    *(.bss)
  }

}
