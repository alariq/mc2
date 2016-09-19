#ifndef STRING_WIN_H
#define STRING_WIN_H

#ifdef LINUX_BUILD

#include<ctype.h>
#include<strings.h>

char *_strlwr(
           char * str
        );

char *_strupr(
           char * str
        );

char *strlwr(
           char * str
        );

char *strupr(
           char * str
        );

int _stricmp(
        const char *string1,
        const char *string2 
        );

int stricmp(
        const char *string1,
        const char *string2 
        );

int _strnicmp(
        const char *string1,
        const char *string2,
        size_t count 
        );

int strnicmp(
        const char *string1,
        const char *string2,
        size_t count 
        );

#define _strdup strdup

#endif // LINUX_BUILD

#endif //STRING_WIN_H
