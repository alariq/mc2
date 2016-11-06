#ifndef _WINNLS_H
#define _WINNLS_H

#ifdef __cplusplus
extern "C" {
#endif

#define CP_ACP 0
#define LOCALE_SYSTEM_DEFAULT	0x800

#define CT_CTYPE1 1
#define CT_CTYPE2 2
#define CT_CTYPE3 4

#define C1_UPPER 1
#define C1_LOWER 2
#define C1_DIGIT 4
#define C1_SPACE 8
#define C1_PUNCT 16
#define C1_CNTRL 32
#define C1_BLANK 64
#define C1_XDIGIT 128
#define C1_ALPHA 256
#define C2_LEFTTORIGHT 1
#define C2_RIGHTTOLEFT 2
#define C2_EUROPENUMBER 3
#define C2_EUROPESEPARATOR 4
#define C2_EUROPETERMINATOR 5
#define C2_ARABICNUMBER 6
#define C2_COMMONSEPARATOR 7
#define C2_BLOCKSEPARATOR 8
#define C2_SEGMENTSEPARATOR 9
#define C2_WHITESPACE 10
#define C2_OTHERNEUTRAL 11
#define C2_NOTAPPLICABLE 0
#define C3_NONSPACING 1
#define C3_DIACRITIC 2
#define C3_VOWELMARK 4
#define C3_SYMBOL 8
#define C3_KATAKANA 16
#define C3_HIRAGANA 32
#define C3_HALFWIDTH 64
#define C3_FULLWIDTH 128
#define C3_IDEOGRAPH 256
#define C3_KASHIDA 512
#define C3_LEXICAL 1024
#define C3_ALPHA 32768
#define C3_NOTAPPLICABLE 0

int WideCharToMultiByte(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
int MultiByteToWideChar(UINT,DWORD,LPCSTR,int,LPWSTR,int);

#define GetStringTypeEx GetStringTypeExA
BOOL GetStringTypeExA(LCID,DWORD,LPCSTR,int,LPWORD);
BOOL GetStringTypeExW(LCID,DWORD,LPCWSTR,int,LPWORD);

BOOL IsDBCSLeadByte(BYTE);

#ifdef __cplusplus
}
#endif

#endif
