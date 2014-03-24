/**
 * Equivalent to this:
 *********************
 * push rcx
 * ; save the page table
 * mov r11, cr3
 * mov r10, rsp
 * mov rax, 0xAABBCCDDEEFF0011
 * mov rsp, rax
 * mov rax, 0xAAAABBBBCCCCDDDD
 * mov cr3, rax
 * ; make the call
 * push r10
 * push r11
 * mov rax, 0x7FFFFFFFFFFFFFFF
 * call rax
 * ; restore the state
 * pop r11
 * pop r10
 * mov rsp, r10
 * mov cr3, r11
 * pop rcx
 * sysret
 ***************************/
typedef struct {
  char code1[0xa]; // 51 41 0F 20 DB 49 89 E2 48 B8
  uint64_t stack;
  char code2[0x5]; // 48 89 C4 48 B8
  uint64_t newPML4;
  char code3[0x9]; // 0F 22 D8 41 52 41 53 48 B8
  uint64_t routine;
  char code4[0x10]; // FF D0 41 5B 41 5A 4C 89 D4 41 0F 22 DB 59 0F 07
} __attribute__((packed)) syscall_code;

typedef struct {
  uint64_t rsp;
  uint64_t rbp;
  uint64_t cr3;
  uint64_t rip;
  uint64_t flags;
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;

  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;

  syscall_code callCode;
} __attribute__((packed)) anscheduler_state;

