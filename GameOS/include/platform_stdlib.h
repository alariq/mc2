#ifndef _STDLIB_WIN_H
#define _STDLIB_WIN_H

#ifdef PLATFORM_WINDOWS
	#include <stdlib.h>
#else

#include<stdio.h>
#include<stdlib.h> // itoa

#ifdef __cplusplus
extern "C" {
#endif

void _splitpath (const char*, char*, char*, char*, char*);
void _itoa(int value, char* str, int radix);

#define _alloca alloca

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_WINDOWS

#endif /* Not _STDLIB_WIN_H */
