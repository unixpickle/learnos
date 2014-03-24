void handle_interrupt_exception();
void handle_interrupt_exception_code();
void handle_interrupt_irq();
void handle_interrupt_ipi();

void handle_unknown_int();

void handle_dummy_lower();
void handle_dummy_upper();

/*
 * Run lidt
 */
void load_idtr(void * idtr);

