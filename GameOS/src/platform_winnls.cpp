#ifndef PLATFORM_WINDOWS

#include <stdlib.h>
#include <wchar.h>
#include <ctype.h>
#include "platform_windows.h"

#define _In_
#define _Out_opt_ 
#define _In_opt_  


int WideCharToMultiByte(
  _In_      UINT    CodePage,
  _In_      DWORD   dwFlags,
  _In_      LPCWSTR lpWideCharStr,
  _In_      int     cchWideChar,
  _Out_opt_ LPSTR   lpMultiByteStr,
  _In_      int     cbMultiByte,
  _In_opt_  LPCSTR  lpDefaultChar,
  _Out_opt_ LPBOOL  lpUsedDefaultChar
)
{
    return wcstombs(lpMultiByteStr, lpWideCharStr, wcslen(lpWideCharStr));
}

int MultiByteToWideChar(
        _In_      UINT   CodePage,
        _In_      DWORD  dwFlags,
        _In_      LPCSTR lpMultiByteStr,
        _In_      int    cbMultiByte,
        _Out_opt_ LPWSTR lpWideCharStr,
        _In_      int    cchWideChar
        )
{
    return mbstowcs(lpWideCharStr, lpMultiByteStr, cchWideChar);
}

BOOL GetStringTypeEx(
        LCID    Locale,
        DWORD   dwInfoType,
        LPCTSTR lpSrcStr,
        int     cchSrc,
        LPWORD  lpCharType
        ) {

    if(dwInfoType == CT_CTYPE1)
    {
        for(int i=0;i < cchSrc; ++i)
        {
            if(isdigit(lpSrcStr[i])) {
                lpCharType[i] = C1_DIGIT;
                if(isxdigit(lpSrcStr[i])) {
                    lpCharType[i] |= C1_XDIGIT;
                }
            } else if(isblank(lpSrcStr[i])) {
                if(isspace(lpSrcStr[i])) {
                    lpCharType[i] |= C1_SPACE;
                }
            } else if(isalpha(lpSrcStr[i])) {
                lpCharType[i] = C1_ALPHA;
                if(islower(lpSrcStr[i])) {
                    lpCharType[i] |= C1_LOWER;
                } else if(isupper(lpSrcStr[i])) {
                    lpCharType[i] |= C1_UPPER;
                }
            }

            if(ispunct(lpSrcStr[i])) {
                lpCharType[i] |= C1_PUNCT;
            }
            if(iscntrl(lpSrcStr[i])) {
                lpCharType[i] |= C1_CNTRL;
            }
        }            
        return true;
    } else {
        return false;
    }
}

BOOL IsDBCSLeadByte(BYTE TestChar) {
    // sebi: NB!
    return FALSE;
}

#endif // PLATFORM_WINDOWS
