bits 64

PML4_START equ 0x300000

extern kernpage_calculate_virtual
extern thread_resume_kernel_stack
extern scheduler_handle_timer
extern anlock_unlock
extern anlock_lock
extern cpu_current, cpu_dedicated_stack
extern thread_translate_kernel_stack

global task_run_with_stack
task_run_with_stack:
  mov rsp, rdi
  mov rbp, rdi
  mov rdi, rsi
  jmp rdx

global task_save_state
task_save_state:
  ; immediately save the state of the 8 general purpose registers
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
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
  call cpu_current

  ; get the current thread running on this CPU
  mov rsi, rax
  mov rdi, [rsi + 0x18] ; thread_t structure
  cmp rdi, 0
  je .end ; there is no current thread_t running on the CPU

  ; store each of our variables in the thread_t structure
  mov rax, [rsp + 0x60] ; 0x40 + 0x8 (ret addr) + 0x18 (rip, cs, flags)
  mov [rdi + 0x10], rax ; rsp
  mov rax, [rsp]
  mov [rdi + 0x18], rax ; rbp
  mov rax, [rsp + 8]
  mov [rdi + 0x20], rax ; cr3
  mov rax, [rsp + 0x48]
  mov [rdi + 0x28], rax ; rip
  mov rax, [rsp + 0x58]
  mov [rdi + 0x30], rax ; flags
  
  ; store rax, rbx, rcx, rdx, rsi, rdi, r8, ..., r15
  lea rsi, [rsp + 0x10]
  lea rax, [rdi + 0x38]
  mov rdi, rax
  mov rcx, 0xe
  rep movsq

.end:
  ; return after relieving our stack of its registers!
  add rsp, 0x80
  ret
  
global task_switch_to_kernpage
task_switch_to_kernpage:
  mov rcx, cr3
  cmp rcx, PML4_START
  je .return

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
.return:
  ret

; leapfrog off a thread's stack and setup the context
global task_switch
task_switch:
  ; immediately, we jump into the thread's kernel stack
  push rdi
  push rsi

  call thread_resume_kernel_stack
  pop rsi
  pop rdi
  mov rsp, rax
  mov rbp, rax

  ; unset the NT flag (should never be set anyways)
  pushfq
  pop rax
  mov rcx, (0xffffffffffffffff ^ (1 << 14))
  and rax, rcx
  push rax
  popfq

  add rsi, 0x10 ; start of state_t structure

  ; start pushing state for IRETQ

  mov rax, 0x0
  push rax ; push SS
  mov rax, [rsi]
  push rax ; push RSP

  mov rax, [rsi + 0x20] ; flags
  push rax

  ; push CS with correct priviledge level
  mov rax, 0x8
  mov rcx, [rsi + 0x10]
  cmp rcx, PML4_START
  je .pushCS
  mov rax, 0x1b ; PL = 3
  mov word [rsp + 0x10], 0x23 ; the SS
.pushCS:
  push rax

  ; push rip
  mov rax, [rsi + 0x18]
  push rax

  ; push the appropriate registers to the stack for the next thingy
  mov rax, [rsi + 0x8] ; rbp
  push rax

  ; push all general purpose registers to the stack, all 0xe of them
  sub rsp, 0x70
  mov rdi, rsp
  mov rax, rsi
  lea rsi, [rax + 0x28]
  mov rcx, 0xe
  rep movsq

  mov rsi, rax

  ; check if we need to calculate a translated stack
  mov rbx, [rsi + 0x10] ; the cr3 field
  cmp rbx, PML4_START
  je .avoidRecalc

  ; rdi is already a task, make rsi a thread_t* and rdx the stack to translate
  sub rsi, 0x10 ; go back to the beginning of our thread
  mov rdx, rsp
  call thread_translate_kernel_stack
  mov cr3, rbx ; rbx was preserved
  mov rsp, rax

.avoidRecalc:
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  pop rbp

  iretq ; will reset the interrupts flag in the FLAGS register


global task_switch_interrupt
task_switch_interrupt:
  call task_save_state
  call cpu_dedicated_stack
  mov rsp, rax
  call scheduler_handle_timer ; hangs if no tasks can be run, never returns

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

