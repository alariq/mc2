#ifndef LIBMAIN_H
#define LIBMAIN_H

#ifdef PLATFORM_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
#define CDECL __cdecl
#else
#define DLL_EXPORT
#define CDECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT const char* CDECL getStringById(unsigned int id);
DLL_EXPORT void CDECL initStringResources();
DLL_EXPORT void CDECL freeStringResources();

#ifdef __cplusplus
}
#endif

const char* getStringById__(unsigned int id);
void initStringResources__();
void freeStringResources__();

#endif // LIBMAIN_H
