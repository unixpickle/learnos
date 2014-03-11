#include "pic.h"
#include <stdio.h>
#include <libkern_base.h>
#include "acpi.h"

void pic_disable() {
  if (!acpi_has_pic()) return;
  print("disabling PIC...\n");
  outb(0x21, 0xff);
  outb(0xa1, 0xff);
}

