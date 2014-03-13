START equ 0x10500400000

global keyboard_driver_start
keyboard_driver_start:
  jmp .readNextChar
.keyboard_loop:
  int 0x23
.readNextChar:
  ; read the scan code
  mov rdi, 1
  mov rsi, 0x64
  int 0x24
  test al, 1
  jnz .readScan
  jmp .keyboard_loop
.readScan:
  mov rdi, 1
  mov rsi, 0x60
  int 0x24
  ; al is scan code
  mov rdi, (.msg_event - keyboard_driver_start + START)
  int 0x21
  jmp .readNextChar

.msg_event:
  db 'Key ', 0

global keyboard_driver_end
keyboard_driver_end:

