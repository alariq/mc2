#include "libmain.h"

extern void initStringResources()
{
    initStringResources__();
}

extern void freeStringResources()
{
    freeStringResources__();
}

extern const char* getStringById(unsigned int id)
{
    return getStringById__(id);
}


