SECTIONS {

  . = 2M;

  .text BLOCK(16) : ALIGN(16) {
    *(.multiboot)
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
