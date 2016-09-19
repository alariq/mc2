#include"string_win.h"

char* _strlwr(char* s) {
    char* p = s;
    while (*p = tolower( *p )) p++;
    return s;
}
char* strlwr(char* s) {
    return _strlwr(s);
}

char* _strupr(char* s) {
    char* p = s;
    while (*p = toupper( *p )) p++;
    return s;
}
char* strupr(char* s) {
    return _strupr(s);
}

int _stricmp(
        const char *string1,
        const char *string2 
        )
{
    return strcasecmp(string1, string2);
}

int stricmp(
        const char *string1,
        const char *string2 
        )
{
    return strcasecmp(string1, string2);
}

int strnicmp(
        const char *string1,
        const char *string2,
        size_t count 
        ) {
    return strncasecmp(string1, string2, count);
}

int _strnicmp(
        const char *string1,
        const char *string2,
        size_t count 
        ) {
    return strncasecmp(string1, string2, count);
}
