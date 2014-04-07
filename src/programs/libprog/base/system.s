bits 64

section .text

global sys_print
sys_print:
  mov rsi, rdi
  xor rdi, rdi
  syscall
  ret

global sys_get_time
sys_get_time:
  mov rdi, 1
  syscall
  ret

global sys_sleep
sys_sleep:
  mov rsi, rdi
  mov rdi, 2
  syscall
  ret

global sys_exit
sys_exit:
  mov rdi, 3
  syscall

global sys_thread_exit
sys_thread_exit:
  mov rdi, 4
  syscall

global sys_wants_interrupts
sys_wants_interrupts:
  mov rdi, 5
  syscall
  ret

global sys_get_interrupts
sys_get_interrupts:
  mov rdi, 6
  syscall
  ret

global sys_open
sys_open:
  mov rdi, 7
  syscall
  ret

global sys_connect
sys_connect:
  mov rdx, rsi ; PID
  mov rsi, rdi ; fd
  mov rdi, 8
  syscall
  ret

global sys_close
sys_close:
  mov rsi, rdi
  mov rdi, 9
  syscall
  ret

global sys_write
sys_write:
  mov r8, rdx ; 3rd argument since rcx is reserved for syscall
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 10
  syscall
  ret

global sys_read
sys_read:
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 11
  syscall
  ret

global sys_poll
sys_poll:
  mov rdi, 12
  syscall
  ret

global sys_remote_pid
sys_remote_pid:
  mov rsi, rdi
  mov rdi, 13
  syscall
  ret

global sys_remote_uid
sys_remote_uid:
  mov rsi, rdi
  mov rdi, 14
  syscall
  ret

global sys_in
sys_in:
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 15
  syscall
  ret

global sys_out
sys_out:
  mov r8, rdx
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 0x10
  syscall
  ret

global sys_color
sys_color:
  mov rsi, rdi
  mov rdi, 0x11
  syscall
  ret

global sys_fork
sys_fork:
  mov rsi, rdi
  mov rdi, 0x12
  syscall
  ret

global sys_mem_usage
sys_mem_usage:
  mov rdi, 0x13
  syscall
  ret

global sys_kill
sys_kill:
  mov rsi, rdi
  mov rdi, 0x14
  syscall
  ret

global sys_alloc_page
sys_alloc_page:
  mov rdi, 0x15
  syscall
  ret

global sys_alloc_aligned
sys_alloc_aligned:
  mov rsi, rdi
  mov rdi, 0x16
  syscall
  ret

global sys_free_page
sys_free_page:
  mov rsi, rdi
  mov rdi, 0x17
  syscall
  ret

global sys_free_aligned
sys_free_aligned:
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 0x18
  syscall
  ret

global sys_vmmap
sys_vmmap:
  mov rcx, rdx
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 0x19
  syscall
  ret

global sys_vmunmap
sys_vmunmap:
  mov rdx, rsi
  mov rsi, rdi
  mov rdi, 0x1a
  syscall
  ret

global sys_invlpg
sys_invlpg:
  mov rsi, rdi
  mov rdi, 0x1b
  syscall
  ret

global sys_launch_thread
sys_launch_thread:
  mov rcx, rsi
  mov rdx, rdi
  mov rsi, .launchStub
  mov rdi, 0x1c
  syscall
  ret
.launchStub:
  mov rax, rdx
  mov rdx, rsi
  call rax
  jmp sys_thread_exit

global sys_thread_id
sys_thread_id:
  mov rdi, 0x1d
  syscall
  ret

global sys_unsleep
sys_unsleep:
  mov rsi, rdi
  mov rdi, 0x1e
  syscall
  ret

global sys_self_uid
sys_self_uid:
  mov rdi, 0x1f
  syscall
  ret

global sys_self_pid
sys_self_pid:
  mov rdi, 0x20
  syscall
  ret

global sys_vmread
sys_vmread:
  mov rsi, rdi
  mov rdi, 0x21
  syscall
  ret

global sys_become_pager
sys_become_pager:
  mov rdi, 0x22
  syscall
  ret

global sys_get_fault
sys_get_fault:
  mov rsi, rdi
  mov rdi, 0x23
  syscall
  ret

global sys_self_vmmap
sys_self_vmmap:
  mov rdx, rsi
  mov rsi, rdx
  mov rdi, 0x24
  syscall
  ret

global sys_self_vmunmap
sys_self_vmunmap:
  mov rsi, rdi
  mov rdi, 0x25
  syscall
  ret

global sys_self_invlpg
sys_self_invlpg:
  mov rdi, 0x26
  syscall
  ret
