// do not call either of these; they return with `iret`, not `ret`!
extern void handle_unknown_exception();
extern void handle_keyboard_interrupt();

/*
 * Run lidt
 */
extern void load_idtr(void * idtr);
