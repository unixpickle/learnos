#include "pic.h"
#include <stdio.h>
#include <libkern_base.h>
#include "acpi.h"

void pic_disable() {
  if (!acpi_has_pic()) return;
  print64("disabling PIC...\n");
  outb64(0x21, 0xff);
  outb64(0xa1, 0xff);
}

