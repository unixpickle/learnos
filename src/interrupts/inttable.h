/**
 * This file should only be included from idt.c
 */

#ifndef __INTTABLE_H__
#define __INTTABLE_H__

#define INT_TYPE_EXC 0
#define INT_TYPE_EXC_CODE 1
#define INT_TYPE_IRQ 2
#define INT_TYPE_IPI 3

/**
 * Represents the intel machine code:
 * 
 * pushq %rax
 * pushq %rax
 * movabsq $argument, %rax
 * movq %rax, 0x8(%rsp)
 * movabsq $function, %rax
 * xchgq %rax, (%rsp)
 * ret
 */
typedef struct {
  char code1[4];
  uint64_t argument;
  char code2[7];
  uint64_t function;
  char code3[5];
} __attribute__((packed)) int_handler_t;

typedef struct {
  uint16_t low_offset;
  uint16_t code_segment;
  uint8_t reserved1;
  uint8_t flags;
  uint16_t mid_offset;
  uint32_t high_offset;
  uint32_t reserved2;
} __attribute__((packed)) idt_entry_t;

typedef struct {
  uint64_t vector;
  uint64_t handler;
  uint8_t flags;
} idt_entry_marker_t;

#define INT_HANDLER_INIT(arg, func) {"\x50\x50\x48\xB8", arg, "\x48\x89\x44\x24\x08\x48\xB8", func, "\x48\x87\x04\x24\xC3"}
#define IDT_ENTRY_INIT(exc, flags) {exc & 0xffff, 8, 0, flags, (exc >> 16) & 0xffff, exc >> 32, 0}

static int_handler_t gIDTHandlers[] = {
  INT_HANDLER_INIT(0, INT_TYPE_EXC),
  INT_HANDLER_INIT(1, INT_TYPE_EXC),
  INT_HANDLER_INIT(2, INT_TYPE_EXC),
  INT_HANDLER_INIT(3, INT_TYPE_EXC),
  INT_HANDLER_INIT(4, INT_TYPE_EXC),
  INT_HANDLER_INIT(5, INT_TYPE_EXC),
  INT_HANDLER_INIT(6, INT_TYPE_EXC),
  INT_HANDLER_INIT(7, INT_TYPE_EXC),
  INT_HANDLER_INIT(8, INT_TYPE_EXC_CODE),
  INT_HANDLER_INIT(9, INT_TYPE_EXC),
  INT_HANDLER_INIT(10, INT_TYPE_EXC_CODE),
  INT_HANDLER_INIT(11, INT_TYPE_EXC_CODE),
  INT_HANDLER_INIT(12, INT_TYPE_EXC_CODE),
  INT_HANDLER_INIT(13, INT_TYPE_EXC_CODE),
  INT_HANDLER_INIT(14, INT_TYPE_EXC_CODE),
  INT_HANDLER_INIT(0x10, INT_TYPE_EXC),
  INT_HANDLER_INIT(0x11, INT_TYPE_EXC_CODE),
  INT_HANDLER_INIT(0x12, INT_TYPE_EXC),
  INT_HANDLER_INIT(0x13, INT_TYPE_EXC),
  INT_HANDLER_INIT(0x14, INT_TYPE_EXC),
  INT_HANDLER_INIT(0x1e, INT_TYPE_EXC),
  INT_HANDLER_INIT(0x20, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x21, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x22, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x23, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x24, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x25, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x26, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x27, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x28, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x29, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x2a, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x2b, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x2c, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x2d, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x2e, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0x2f, INT_TYPE_IRQ),
  INT_HANDLER_INIT(0xff, INT_TYPE_IRQ), // spurrious
  INT_HANDLER_INIT(0x30, INT_TYPE_IRQ), // LAPIC timer interrupt
  INT_HANDLER_INIT(0x31, INT_TYPE_IPI), // inval page
};

#endif

