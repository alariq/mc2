#ifndef PLATFORM_WINDOWS

#include "platform_tchar.h"

// http://www.gnu.org/software/libc/manual/html_node/Converting-a-Character.html#Converting-a-Character

#if defined(_UNICODE)
char *_strinc( const char *current) {
    return current + 1;
}

#else

const wchar_t* _wcsinc( const wchar_t *current) {
    return current + 1;
}

#endif 

#endif // PLATFORM_WINDOWS
