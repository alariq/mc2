#ifndef STRING_UTILS_H
#define STRING_UTILS_H

void StringFree(char* s);
void StringFree(wchar_t* s);
char* StringDup(const char* s);
wchar_t* StringDup(const wchar_t* s);
char* StringDupSafe(const char* s);
wchar_t* StringDupSafe(const wchar_t* s);

// this function assume that szPath and szLowerPath are valid pointers
int StringToLower(const char* const szPath, char* szLowerPath, int nLen);

int StringToLowerSafe(const char* const szPath, char* szLowerPath, int nLen);
/*
char* WStringToString(const wchar_t* wstring, char* string, int stringMaxLen);
char* WStringToString(const wchar_t* wstring);
wchar_t* StringToWString(const char* string, wchar_t* wstring, int wstringMaxLen);
wchar_t* StringToWString(const char* string);
BOOL CopyToStaticString(char *szStaticString, int32 iStaticStringLen, const char *szSource);
*/

const char* StringGetExtention(const char* pstr);

// aka C99 snprintf
int StringFormat(char* buf, size_t buf_size, const char* format, ...);

#endif // STRING_UTILS_H
