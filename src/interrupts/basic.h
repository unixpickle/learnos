// do not call either of these; they return with `iret`, not `ret`!
void handle_unknown_exception();
void handle_div_zero();
void handle_debugger();
void handle_nmi();
void handle_breakpoint();
void handle_overflow();
void handle_bounds();
void handle_invalid_opcode();
void handle_coprocessor_not_available();
void handle_double_fault();
void handle_coprocessor_segment_overrun();
void handle_invalid_tss();
void handle_segmentation_fault();
void handle_stack_fault();
void handle_general_protection_fault();
void handle_page_fault();
void handle_math_fault();
void handle_alignment_check();
void handle_machine_check();
void handle_simd_exception();

void handle_keyboard_interrupt();

/*
 * Run lidt
 */
void load_idtr(void * idtr);
