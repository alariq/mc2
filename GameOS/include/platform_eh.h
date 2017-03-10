#ifndef PLATFORM_EH_H
#define PLATFORM_EH_H

#ifndef PLATFORM_WINDOWS

typedef void (*exception_handler_ptr_t)(void);

void set_terminate(exception_handler_ptr_t eh);
#else
#include<exception>
#endif

#endif // PLATFORM_EH_H
