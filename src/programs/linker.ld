SECTIONS {

  . = 0x400000;

  .text BLOCK(16) : ALIGN(16) {
    *(.entrypoint)
    *(.text)
  }
  .rodata BLOCK(16) : ALIGN(16) {
    *(.rodata)
  }
  .data BLOCK(16) : ALIGN(16) {
    *(.data)
    *(COMMON)
    *(.bss)
  }

}
