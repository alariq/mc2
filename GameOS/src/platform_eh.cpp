#ifndef PLATFORM_WINDOWS

#include<exception>
#include<stdio.h>
#include"platform_eh.h"

static exception_handler_ptr_t original_handler = NULL;

static void my_handler_proxy(void)
{
    fprintf(stderr, "sebi: program was terminated\n");
    original_handler();
    original_handler = NULL;
}


void set_terminate(exception_handler_ptr_t eh)
{
    std::set_terminate(my_handler_proxy);
    original_handler = eh;
}

#endif // PLATFORM_WINDOWS