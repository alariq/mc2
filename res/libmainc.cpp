#include "libmain.h"

DLL_EXPORT void CDECL initStringResources()
{
    initStringResources__();
}

DLL_EXPORT void CDECL freeStringResources()
{
    freeStringResources__();
}

DLL_EXPORT const char* CDECL getStringById(unsigned int id)
{
    return getStringById__(id);
}


