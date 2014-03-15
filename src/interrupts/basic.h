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
void handle_spurious();

void handle_dummy_lower();
void handle_dummy_upper();

void handle_irq0();
void handle_irq1();
void handle_irq2();
void handle_irq3();
void handle_irq4();
void handle_irq5();
void handle_irq6();
void handle_irq7();
void handle_irq8();
void handle_irq9();
void handle_irq10();
void handle_irq11();
void handle_irq12();
void handle_irq13();
void handle_irq14();
void handle_irq15();

/*
 * Run lidt
 */
void load_idtr(void * idtr);

