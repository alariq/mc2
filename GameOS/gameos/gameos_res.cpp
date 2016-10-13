#include "gameos.hpp"
#include "strres.h"

#include <stdlib.h>
#include <stdio.h>
/* Need dlfcn.h for the routines to
   dynamically load libraries */
#include <dlfcn.h>

HSTRRES __stdcall gos_OpenResourceDLL(char const* FileName, const char** strings, int num)
{
    const char *error;
    void *module;
    get_string_by_id_fptr fptr;

    /* Load dynamically loaded library */
    module = dlopen(FileName, RTLD_LAZY);
    gosASSERT(module);
    if (!module) {
        fprintf(stderr, "Couldn't open resourse dll: %s\n", dlerror());
        return NULL;
    }

    /* Get symbol */
    dlerror();
    fptr = (get_string_by_id_fptr)dlsym(module, "getStringById");
    if ((error = dlerror())) {
        fprintf(stderr, "Couldn't find hello: %s\n", error);
        return NULL;
    }

    init_string_resources_fptr init_fptr;
    dlerror();
    init_fptr = (init_string_resources_fptr)dlsym(module, "initStringResources");
    if ((error = dlerror())) {
        fprintf(stderr, "Couldn't find hello: %s\n", error);
        return NULL;
    }
    init_fptr();

    gos_StringRes* pstrres = new gos_StringRes();
    pstrres->getStringByIdFptr = fptr;
    pstrres->module = module;

    /* deprecated */
    pstrres->strings = NULL;
    pstrres->num_strings = 0;

    return pstrres;
}
void __stdcall gos_CloseResourceDLL(HSTRRES handle)
{
    gos_StringRes* pstrres = (gos_StringRes*)handle;
    gosASSERT(pstrres && pstrres->module);

    free_string_resources_fptr free_fptr;
    dlerror();

    const char *error;
    free_fptr = (free_string_resources_fptr)dlsym(pstrres->module, "freeStringResources");
    if ((error = dlerror())) {
        fprintf(stderr, "Couldn't find hello: %s\n", error);
    } else {
        free_fptr();
    }

    dlclose(pstrres->module);
    delete pstrres;
}

const char* __stdcall gos_GetResourceString(HSTRRES handle, DWORD id)
{
    static const char* dummy_string_res = "missing string res";
    gosASSERT(handle);
    gos_StringRes* pstrres = (gos_StringRes*)(handle);
    const char* str = pstrres->getStringByIdFptr(id);

    if(NULL == str) {
        fprintf(stderr, "Requested string id: %d not found, return dummy string\n", id);
        return dummy_string_res;
    }
    return str;
}
