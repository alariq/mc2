#include "gameos.hpp"
#include "strres.h"

#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_loadso.h>

#ifdef PLATFORM_WINDOWS
static void* DL_Open(const char* name) {
	return SDL_LoadObject(name);
}
static void DL_Close(void* handle) {
	SDL_UnloadObject(handle);
}
static void* DL_LoadFunction(void* handle, const char* name) {
	return SDL_LoadFunction(handle, name);
}
// mimic dlerror
static const char* DL_GetError() {
	const char* err = SDL_GetError();
	SDL_ClearError();
	return strlen(err)==0 ? NULL : err;
}
#else
#include <dlfcn.h>

static void* DL_Open(const char* name) {
    return dlopen(name, RTLD_LAZY);
}
static void DL_Close(void* handle) {
	dlclose(handle);
}
static void* DL_LoadFunction(void* handle, const char* name) {
	return dlsym(handle, name);
}
static const char* DL_GetError() {
	 const char* err = dlerror();
	 return err;
}
#endif

HSTRRES __stdcall gos_OpenResourceDLL(char const* FileName, const char** strings, int num)
{
    const char *error;
    void *module;
    get_string_by_id_fptr fptr;

	DL_GetError();

    /* Load dynamically loaded library */
    module = DL_Open(FileName);
    gosASSERT(module);
    if (!module) {
        fprintf(stderr, "Couldn't open resourse dll: %s\n", DL_GetError());
        return NULL;
    }

	DL_GetError();    /* Clear any existing error */

    /* Get symbol */
    fptr = (get_string_by_id_fptr)DL_LoadFunction(module, "getStringById");
    if ((error = DL_GetError())) {
        fprintf(stderr, "Couldn't find hello: %s\n", error);
        return NULL;
    }

    init_string_resources_fptr init_fptr;
    init_fptr = (init_string_resources_fptr)DL_LoadFunction(module, "initStringResources");
    if ((error = DL_GetError())) {
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
    DL_GetError();

    const char *error;
    free_fptr = (free_string_resources_fptr)DL_LoadFunction(pstrres->module, "freeStringResources");
    if ((error = DL_GetError())) {
        fprintf(stderr, "Couldn't find hello: %s\n", error);
    } else {
        free_fptr();
    }

    DL_Close(pstrres->module);
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
