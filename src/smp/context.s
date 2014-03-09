bits 64

PML4_START equ 0x300000

extern task_critical_stop
extern cpu_get_current
extern kernpage_calculate_virtual
extern thread_resume_kernel_stack
extern scheduler_run_next
extern anlock_unlock
extern anlock_lock
extern ref_retain
extern ref_release
extern lapic_timer_set
extern lapic_send_eoi
extern thread_translate_kernel_stack, cpu_get_dedicated_stack
extern stack_log

global task_run_with_stack
task_run_with_stack:
  mov rsp, rdi
  mov rbp, rdi
  mov rdi, rsi
  jmp rdx

global task_save_state
task_save_state:
  ; immediately save the state of the 8 general purpose registers
  push rdi
  push rsi
  push rdx
  push rcx
  push rbx
  push rax
  mov rax, cr3
  push rax
  push rbp
  ; stack: flags, cs, rip, ret_addr, [registers]

  call task_switch_to_kernpage
  call cpu_get_current
  ; get the current thread running on this CPU
  mov rsi, rax
  lea rdi, [rsi + 0x14]
  push rdi
  push rsi
  call anlock_lock
  pop rsi
  mov rdi, [rsi + 0x24]
  call ref_retain
  pop rdi
  push rax
  call anlock_unlock

  pop rdi ; thread_t structure
  cmp rdi, 0
  je .end ; there is no current thread_t running on the CPU

  ; store each of our variables in the thread_t structure
  mov rax, [rsp + 0x60] ; 0x40 + 0x8 (ret addr) + 0x18 (rip, cs, flags)
  mov [rdi + 0x20], rax ; rsp
  mov rax, [rsp]
  mov [rdi + 0x28], rax ; rbp
  mov rax, [rsp + 8]
  mov [rdi + 0x30], rax ; cr3
  mov rax, [rsp + 0x48]
  mov [rdi + 0x38], rax ; rip
  mov rax, [rsp + 0x58]
  mov [rdi + 0x40], rax ; flags
  mov rax, [rsp + 0x10]
  mov [rdi + 0x48], rax ; rax
  mov rax, [rsp + 0x18]
  mov [rdi + 0x50], rax ; rbx
  mov rax, [rsp + 0x20]
  mov [rdi + 0x58], rax ; rcx
  mov rax, [rsp + 0x28]
  mov [rdi + 0x60], rax ; rdx
  mov rax, [rsp + 0x30]
  mov [rdi + 0x68], rax ; rsi
  mov rax, [rsp + 0x38]
  mov [rdi + 0x70], rax ; rdi
  call ref_release

.end:
  ; return after relieving our stack of its registers!
  add rsp, 0x40
  ret
  
global task_switch_to_kernpage
task_switch_to_kernpage:
  mov rdi, rsp
  shr rdi, 12
  call universal_page_lookup
  mov rdi, rax
  call kernpage_calculate_virtual
  shl rax, 12
  mov rcx, rsp
  and rcx, 0xfff
  or rax, rcx
  mov rsp, rax

  mov rax, PML4_START
  mov cr3, rax
  ret

; leapfrog off a thread's stack and setup the context for an iretq
global task_switch
task_switch:
  ; immediately, we jump into the thread's kernel stack
  push rdi
  push rsi

  mov r8, rdi
  mov rdi, 0x10
  call stack_log
  mov rdi, r8

  call thread_resume_kernel_stack
  pop rsi
  pop rdi
  mov rsp, rax
  mov rbp, rax

  ; unset the NT flag so that iretq has the expected behavior
  pushfq
  pop rax
  mov rcx, (0xffffffffffffffff ^ (1 << 14))
  and rax, rcx
  push rax
  popfq

  add rsi, 0x20 ; start of state_t structure

  ; push state for iretq
  mov ax, 0x0 ; TODO: set PL in the data segment
  push ax ; push SS
  mov rax, [rsi]
  push rax ; push RSP

  mov rax, [rsi + 0x20] ; flags
  push rax

  ; push CS with correct priviledge level
  mov rax, 0x8
  mov rcx, [rsi + 0x10]
  cmp rcx, PML4_START
  je .pushCS
  or rax, 0x3 ; PL = 3
.pushCS:
  push rax

  ; push rip
  mov rax, [rsi + 0x18]
  push rax

  ; push the appropriate registers to the stack for the next thingy
  mov rax, [rsi + 0x8] ; rbp
  push rax
  mov rax, [rsi + 0x28] ; rax
  push rax
  mov rax, [rsi + 0x30] ; rbx
  push rax
  mov rax, [rsi + 0x38] ; rcx
  push rax
  mov rax, [rsi + 0x40] ; rdx
  push rax
  mov rax, [rsi + 0x48] ; rsi
  push rax
  mov rax, [rsi + 0x50] ; rdi
  push rax

  ; check if we need to calculate a translated stack
  mov rbx, [rsi + 0x10] ; the cr3 field
  cmp rbx, PML4_START
  je .avoidRecalc

  ; rdi is already a task, make rsi a thread_t* and rdx the stack to translate
  sub rsi, 0x20 ; go back to the beginning of our thread
  mov rdx, rsp
  call thread_translate_kernel_stack
  mov cr3, rbx ; rbp was preserved
  mov rsp, rax

extern printHex
  mov rdi, rax
  call printHex

.avoidRecalc:
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  pop rbp

  mov r8, rdi
  mov rdi, 0x28
  call stack_log
  mov rdi, r8

  iretq ; will reset the interrupts flag in the FLAGS register


global task_switch_interrupt
task_switch_interrupt:
  call task_save_state
  call cpu_get_dedicated_stack
  mov rsp, rax

  ; well, we should set an interrupt
  call lapic_send_eoi
  mov rdi, 0x20
  mov rsi, 0x10000
  mov rdx, 0x1000 ; super slow
  ; call lapic_timer_set
  call scheduler_run_next ; only returns if #CPU's > #tasks

  ; hang here until another timer interrupt
  sti
.hang:
  hlt
  jmp .hang

; looks up and returns an entry in a page table
; uint64_t virtual_table_lookup(uint64_t index, page_t kernpageAddress);
universal_table_lookup:
  shl rdi, 3 ; byte offset instead of page offset
  shl rsi, 12 ; make the kernpageAddress a real memory address
  mov rcx, cr3
  mov rax, PML4_START
  mov cr3, rax
  mov rax, [rdi+rsi] ; rax is return value
  mov cr3, rcx
  ret

; Uses the PML4 in cr3 to find the physical address for a page
; page_t universal_page_lookup(page_t virtual);
universal_page_lookup:
  push rbp
  mov rbp, rsp

  sub rsp, 0x18
  mov qword [rbp - 0x18], 27 ; shift
  mov rax, cr3
  shr rax, 12
  mov [rbp - 0x10], rax      ; physical table page
  mov [rbp - 0x8], rdi       ; virtual page to lookup
  mov rcx, 4                 ; number of times to run iteration

.copyLoop:
  ; lookup kernpage address of the physical table
  push rcx
  mov rdi, [rbp - 0x10]
  call kernpage_calculate_virtual
  mov rsi, rax
  mov rdi, [rbp - 0x8] ; get argument
  mov rcx, [rbp - 0x18] ; get shift
  shr rdi, cl
  and rdi, 0x1ff
  sub rcx, 9
  mov [rbp - 0x18], rcx
  call universal_table_lookup
  mov rcx, rax
  and rcx, 1 ; make sure first bit is set in page entry
  jz .pageNotFound
  shr rax, 12
  mov [rbp - 0x10], rax

  pop rcx
  loop .copyLoop

  ; return the physical page
  mov rax, [rbp - 0x10]
  leave
  ret

.pageNotFound:
  mov rax, 0
  leave
  ret

