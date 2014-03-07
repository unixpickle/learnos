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
  mov rax, [rsp + 0x5a]
  mov [rdi + 0x20], rax ; rsp
  mov rax, [rsp]
  mov [rdi + 0x28], rax ; rbp
  mov rax, [rsp + 8]
  mov [rdi + 0x30], rax ; cr3
  mov rax, [rsp + 0x48]
  mov [rdi + 0x38], rax ; rip
  mov rax, [rsp + 0x52]
  mov [rdi + 0x40], rax; flags
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

  ; return after releaving our stack of its registers!
.end:
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
  mov rcx, rbp
  and rcx, 0xfff
  add rax, rcx
  mov rsp, rax

  mov rax, PML4_START
  mov cr3, rax
  ret

; leapfrog off a thread's stack and setup the context for an iretq
global task_switch
task_switch:
  ; immediately, we jump into the thread's kernel stack
  push rsi
  push rdi
  call thread_resume_kernel_stack
  pop rdi
  pop rsi
  mov rsp, rax
  mov rbp, rax

  ; unset the NT flag so that iretq has the expected behavior
  pushfq
  pop rax
  mov rcx, (0xffffffffffffffff ^ (1 << 14))
  or rax, rcx
  push rax
  popfq

  add rsi, 0x20 ; start of state_t structure

  ; push state for iretq

  mov rax, [rsi + 0x20] ; flags
  push rax

  ; push CS with correct priviledge level
  mov ax, 0x8
  mov rax, [rsi + 0x10]
  cmp rax, PML4_START
  je .pushCS
  or ax, 0x3 ; PL = 3
.pushCS:
  push ax

  ; push rip
  mov rax, [rsi + 0x18]
  push rax

  ; load the appropriate registers
  mov rax, [rsi + 0x28]
  mov rbx, [rsi + 0x30]
  mov rcx, [rsi + 0x38]
  mov rdx, [rsi + 0x40]
  mov rdi, [rsi + 0x50]

  ; finally, get the rsi value
  push rbx
  mov rbx, [rsi + 0x48]
  mov rsi, rbx
  pop rbx
  iretq ; will reset the interrupts flag in the FLAGS register

global task_switch_interrupt
task_switch_interrupt:
  call task_save_state
  call scheduler_run_next
.loop:
  hlt
  jmp .loop

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
  sub rbp, 0x18
  mov qword [rbp - 0x18], 27 ; shift
  mov rax, cr3
  shr rax, 12
  mov [rbp - 0x10], rax      ; physical table page
  mov [rbp - 0x8], rdi       ; virtual page to lookup

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
  call universal_table_lookup
  mov rcx, rax
  and rcx, 1 ; make sure first bit is set in page entry
  jz .pageNotFound
  shr rax, 12
  mov [rbp - 0x10], rax

  mov rax, [rbp - 0x18]
  sub rax, 9
  mov [rbp - 0x18], rax

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

