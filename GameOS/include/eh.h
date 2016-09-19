#ifndef EH_H
#define EH_H

typedef void (*exception_handler_ptr_t)(void);

void set_terminate(exception_handler_ptr_t eh);

#endif // EH_H
