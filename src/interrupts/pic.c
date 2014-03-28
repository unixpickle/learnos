#include "pic.h"
#include "acpi.h"
#include <stdio.h>
#include <libkern_base.h>

#define ICW1_ICW4  0x01    /* ICW4 (not) needed */
#define ICW1_SINGLE  0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL  0x08    /* Level triggered (edge) mode */
#define ICW1_INIT  0x10    /* Initialization - required! */
 
#define ICW4_8086  0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO  0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER  0x0C    /* Buffered mode/master */
#define ICW4_SFNM  0x10    /* Special fully nested (not) */

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1

void io_wait() {
  outb(0x80, 0);
}

void pic_disable() {
  if (!acpi_has_pic()) return;
  print("disabling PIC...\n");

  /* from: http://wiki.osdev.org/8259_PIC */

  uint8_t offset1 = 0xf0;
  uint8_t offset2 = 0xf8;
 
  outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
  io_wait();
  outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
  io_wait();
  outb(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  io_wait();
  outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();
 
  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  outb(0x21, 0xff);
  outb(0xa1, 0xff);
}

