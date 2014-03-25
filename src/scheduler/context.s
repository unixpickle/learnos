extern thread_resume_kernel_stack, thread_vm_kernel_stack
extern anscheduler_cpu_get_thread
extern kernpage_calculate_virtual

PML4_START equ 0x300000

global anscheduler_save_return_state
anscheduler_save_return_state:
  add rdi, 0x40 ; point to anscheduler_state_t structure

  ; rsp
  lea rax, [rsp + 8]
  mov [rdi], rax
  ; rbp
  mov [rdi + 8], rbp
  ; cr3
  mov rax, cr3
  mov [rdi + 0x10], rax
  ; rip
  pop rax
  mov [rdi + 0x18], rax
  ; rbx
  mov [rdi + 0x30], rbx
  ; r12 - r15
  mov [rdi + 0x78], r12
  mov [rdi + 0x80], r13
  mov [rdi + 0x88], r14
  mov [rdi + 0x90], r15

  mov rdi, rsi
  call rdx ; should never return

; void thread_run_state(thread_t * thread);
global thread_run_state
thread_run_state:
  mov rbx, rdi
  call thread_resume_kernel_stack
  mov rsp, rax
  mov rdi, rbx
  add rdi, 0x40

  ; state for iretq

  xor rax, rax
  push rax ; push SS

  mov rax, [rdi]
  push rax ; push RSP

  mov rax, [rdi + 0x20]
  push rax ; RFLAGS

  ; push CS with correct priviledge level
  mov rax, 0x8
  mov rcx, [rdi + 0x10]
  cmp rcx, PML4_START
  je .pushCS
  mov rax, 0x1b ; PL = 3
  mov word [rsp + 0x10], 0x23 ; the SS
.pushCS:
  push rax

  ; push rip
  mov rax, [rdi + 0x18]
  push rax

  ; push the appropriate registers to the stack for the final restoration
  mov rax, [rdi + 0x8] ; rbp
  push rax

  ; push all general purpose registers to the stack, all 0xe of them
  sub rsp, 0x70
  mov rax, rdi
  lea rsi, [rax + 0x28]
  mov rdi, rsp
  mov rcx, 0xe
  rep movsq
  mov rdi, rax

  ; check if we need to calculate a translated stack
  mov rbx, [rdi + 0x10] ; the cr3 field
  cmp rbx, PML4_START
  je .restoreRegs

  ; rdi is already a task, make rsi a thread_t* and rdx the stack to translate
  sub rdi, 0x40 ; go back to the beginning of our thread
  mov rsi, rsp
  call thread_vm_kernel_stack
  mov cr3, rbx ; rbx was preserved
  mov rsp, rax

.restoreRegs:
  pop rax
  pop rbx
  pop rcx
  pop rdx
  pop rsi
  pop rdi
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
  pop rbp

  iretq

; void thread_switch_to_kernpage();
global thread_switch_to_kernpage
thread_switch_to_kernpage:
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

;void thread_save_state(uint64_t fieldCount);
global thread_save_state
thread_save_state:
  ; immediately save the state of the registers
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

  call thread_switch_to_kernpage
  call anscheduler_cpu_get_thread
  test rax, rax
  jz .end

  lea rdi, [rax + 40]
  ; store each of our variables in the thread_t structure
  mov rax, [rsp + 0xa0] ; 0x80 + 0x8 (ret addr) + 0x18 (rip, cs, flags)
  mov [rdi], rax ; rsp
  mov rax, [rsp]
  mov [rdi + 0x8], rax ; rbp
  mov rax, [rsp + 8]
  mov [rdi + 0x10], rax ; cr3
  mov rax, [rsp + 0x88]
  mov [rdi + 0x18], rax ; rip
  mov rax, [rsp + 0x98]
  mov [rdi + 0x20], rax ; flags
  
  ; store rax, rbx, rcx, rdx, rsi, rdi, r8, ..., r15
  lea rsi, [rsp + 0x10]
  lea rdi, [rdi + 0x28]
  mov rcx, 0xe
  rep movsq

.end:
  add rsp, 0x80
  ret

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
