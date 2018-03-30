#ifndef PLATFORM_WINDOWS

#include<stdio.h>
#include<wctype.h>
#include<ctype.h> // tolower...

#include"platform_windows.h"
#include"platform_str.h"

#define _Inout_

int GetSystemMetrics(int)
{
    // TODO: implememt
    return 0;
}

SHORT GetAsyncKeyState(int vk_key)
{
    // TODO: implememt
    return 0;
}


/*
LPTSTR WINAPI CharLower(
          _Inout_ LPTSTR lpsz
        )
{
    return strlwr(lpsz);
}
*/

int MessageBoxA(HWND,LPCSTR header, LPCSTR message, UINT )
{
    printf("MSGBOX: %s : %s\n", header, message);
    return 0;
}

int MessageBoxW(HWND,LPCWSTR header, LPCWSTR message, UINT)
{
    printf("MSGBOX: %S : %S\n", header, message);
    return 0;
}

LPSTR CharLowerA(LPSTR s)
{
    char* p = s;
    while ((*p = tolower( *p ))) p++;
    return s;
}

LPSTR CharUpperA(LPSTR s)
{
    char* p = s;
    while ((*p = toupper( *p ))) p++;
    return s;
}

LPWSTR CharLowerW(LPWSTR s)
{
    wchar_t* p = s;
    while ((*p = towlower( *p ))) p++;
    return s;
}
LPWSTR CharUpperW(LPWSTR s)
{
    wchar_t* p = s;
    while ((*p = towupper( *p ))) p++;
    return s;

}

#endif // PLATFORM_WINDOWS
