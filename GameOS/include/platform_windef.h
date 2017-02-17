#ifndef _WINDEF_H
#define _WINDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#define FAR
#define far
#define NEAR
#define near

#ifndef CONST
#define CONST const
#endif

#undef MAX_PATH
#define MAX_PATH 260

#ifndef NULL

#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif

#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define CDECL //_cdecl
#define STDCALL //__stdcall
#define FASTCALL //__fastcall
#define WINAPI //__stdcall
#define WINAPIV //__cdecl
#define APIENTRY //__stdcall
#define CALLBACK //__stdcall
#define APIPRIVATE //__stdcall

#define DECLSPEC_IMPORT __declspec(dllimport)
#define DECLSPEC_EXPORT __declspec(dllexport)
#define DECLSPEC_NORETURN __declspec(noreturn)
#define DECLARE_STDCALL_P( type ) __stdcall type

#define MAKEWORD(a,b)	((WORD)(((BYTE)(((DWORD_PTR)(a))&0xFF))|(((WORD)((BYTE)(((DWORD_PTR)(b))&0xFF)))<<8)))
#define MAKELONG(a,b)	((LONG)(((WORD)(((DWORD_PTR)(a))&0xFFFF))|(((DWORD)((WORD)(((DWORD_PTR)(b))&0xFFFF)))<<16)))
#define LOWORD(l)	((WORD)(((DWORD_PTR)(l))&0xFFFF))
#define HIWORD(l)	((WORD)(((DWORD_PTR)(l)>>16)&0xFFFF))
#define LOBYTE(w)	((BYTE)(((DWORD_PTR)(w))&0xFF))
#define HIBYTE(w)	((BYTE)((((DWORD_PTR)(w))>>8)&0xFF))

// !NB sebi, DWORD on Windows is always 32bit!!!    
//typedef unsigned long DWORD;
typedef unsigned int DWORD;
typedef int WINBOOL,*PWINBOOL,*LPWINBOOL;
/* FIXME: Is there a good solution to this? */
#define BOOL WINBOOL
typedef unsigned char BYTE;
typedef BOOL *PBOOL,*LPBOOL;
typedef unsigned short WORD;
typedef float FLOAT;
typedef FLOAT *PFLOAT;
typedef BYTE *PBYTE,*LPBYTE;
typedef int *PINT,*LPINT;
typedef WORD *PWORD,*LPWORD;
typedef long *LPLONG;
typedef DWORD *PDWORD,*LPDWORD;
typedef CONST void *PCVOID,*LPCVOID;
typedef int INT;
typedef unsigned int UINT,*PUINT,*LPUINT;
typedef double DOUBLE; // sebi, actually in wtypes.h

#ifndef NOMINMAX

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#endif

#include "platform_winnt.h"

typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;

#ifndef _HRESULT_DEFINED
typedef LONG HRESULT;
#define _HRESULT_DEFINED
#endif

typedef WORD ATOM;
typedef HANDLE HHOOK;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HANDLE GLOBALHANDLE;
typedef HANDLE LOCALHANDLE;
typedef void *HGDIOBJ;
DECLARE_HANDLE(HACCEL);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HCOLORSPACE);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HGLRC);
DECLARE_HANDLE(HDESK);
DECLARE_HANDLE(HENHMETAFILE);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HKEY);
/* FIXME: How to handle these. SM_CMONITORS etc in winuser.h also. */
DECLARE_HANDLE(HMONITOR);
#define HMONITOR_DECLARED 1
DECLARE_HANDLE(HTERMINAL);
DECLARE_HANDLE(HWINEVENTHOOK);

typedef HKEY *PHKEY;
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HMETAFILE);
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;
DECLARE_HANDLE(HPALETTE);
DECLARE_HANDLE(HPEN);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HRSRC);
DECLARE_HANDLE(HSTR);
DECLARE_HANDLE(HTASK);
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HWINSTA);
DECLARE_HANDLE(HKL);
typedef int HFILE;
typedef HICON HCURSOR;
typedef DWORD COLORREF;
typedef int (WINAPI *FARPROC)();
typedef int (WINAPI *NEARPROC)();
typedef int (WINAPI *PROC)();
typedef struct tagRECT {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT,*PRECT,*LPRECT;
typedef const RECT *LPCRECT;
typedef struct tagRECTL {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECTL,*PRECTL,*LPRECTL;
typedef const RECTL *LPCRECTL;
typedef struct tagPOINT {
	LONG x;
	LONG y;
} POINT,POINTL,*PPOINT,*LPPOINT,*PPOINTL,*LPPOINTL;
typedef struct tagSIZE {
	LONG cx;
	LONG cy;
} SIZE,SIZEL,*PSIZE,*LPSIZE,*PSIZEL,*LPSIZEL;
typedef struct tagPOINTS {
	SHORT x;
	SHORT y;
} POINTS,*PPOINTS,*LPPOINTS;

#ifdef __cplusplus
}
#endif

#endif
