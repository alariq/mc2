#include "libmain.h"

DLL_EXPORT void __cdecl initStringResources()
{
    initStringResources__();
}

DLL_EXPORT void __cdecl freeStringResources()
{
    freeStringResources__();
}

DLL_EXPORT const char* __cdecl getStringById(unsigned int id)
{
    return getStringById__(id);
}


