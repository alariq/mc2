#ifndef _WINNT_H
#define _WINNT_H

#ifdef LINUX_BUILD

#ifdef __cplusplus
extern "C" {
#endif

#include "platform_winerror.h"

#ifndef RC_INVOKED
#include <string.h>

/* FIXME: add more architectures. Is there a way to specify this in GCC? */
#ifdef _X86_
#define UNALIGNED
#else
#define UNALIGNED
#endif

#ifndef DECLSPEC_ALIGN
#define DECLSPEC_ALIGN(x) __attribute__((aligned(x)))
#endif

#ifndef FORCEINLINE
#define FORCEINLINE __inline  __attribute__((always_inline))
#endif

#ifndef C_ASSERT
#define C_ASSERT(expr) typedef char __C_ASSERT__[(expr)?1:-1]
#endif

#ifndef VOID
#define VOID void
#endif

typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef char CCHAR, *PCCHAR;
typedef unsigned char UCHAR,*PUCHAR;
typedef unsigned short USHORT,*PUSHORT;
typedef unsigned long ULONG,*PULONG;
typedef char *PSZ;

typedef void *PVOID,*LPVOID;

typedef void *HANDLE;

/* FIXME for __WIN64 */
#ifndef  __ptr64
#define __ptr64
#endif
typedef void* __ptr64 PVOID64;
/*
#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
#ifndef _WCHAR_T_
#define _WCHAR_T_
#undef __need_wchar_t
#ifndef __cplusplus
typedef unsigned short wchar_t;
#endif
#endif
#endif
*/

typedef wchar_t WCHAR;
typedef WCHAR *PWCHAR,*LPWCH,*PWCH,*NWPSTR,*LPWSTR,*PWSTR;
typedef CONST WCHAR *LPCWCH,*PCWCH,*LPCWSTR,*PCWSTR;
typedef CHAR *PCHAR,*LPCH,*PCH,*NPSTR,*LPSTR,*PSTR;
typedef CONST CHAR *LPCCH,*PCSTR,*LPCSTR;

#ifndef _TCHAR_DEFINED
#define _TCHAR_DEFINED

/* Cannot use __AW(). */
#ifdef UNICODE
/*
 * NOTE: This tests UNICODE, which is different from the _UNICODE define
 *       used to differentiate standard C runtime calls.
 */
typedef WCHAR TCHAR;
typedef WCHAR _TCHAR;
/*
 * __TEXT is a private macro whose specific use is to force the expansion of a
 * macro passed as an argument to the macro TEXT.  DO NOT use this
 * macro within your programs.  It's name and function could change without
 * notice.
 */
#define __TEXT(q) L##q
#else
typedef CHAR TCHAR;
typedef CHAR _TCHAR;
#define __TEXT(q) q
#endif

#endif

typedef TCHAR TBYTE,*PTCH,*PTBYTE;
typedef TCHAR *LPTCH,*PTSTR,*LPTSTR,*LP,*PTCHAR;
typedef const TCHAR *LPCTSTR;

/*
 * UNICODE a constant string when UNICODE is defined, else returns the string
 * unmodified.
 * The corresponding macros  _TEXT() and _T() for mapping _UNICODE strings
 * passed to C runtime functions are defined in mingw/tchar.h
 */
#define TEXT(q) __TEXT(q)    
#define _TEXT(q) __TEXT(q) // sebi
typedef SHORT *PSHORT;
typedef LONG *PLONG;

#ifdef STRICT
typedef void *HANDLE;
#define DECLARE_HANDLE(n) typedef struct n##__{int i;}*n
#else

typedef PVOID HANDLE;
#define DECLARE_HANDLE(n) typedef HANDLE n
#endif

typedef HANDLE *PHANDLE,*LPHANDLE;
typedef DWORD LCID;
typedef PDWORD PLCID;
typedef WORD LANGID;

#define _HAVE_INT64
#define _INTEGRAL_MAX_BITS 64
#undef __int64
#define __int64 long long

typedef __int64 _int64; //sebi

typedef __int64 LONGLONG;
typedef unsigned __int64 DWORDLONG;

typedef LONGLONG *PLONGLONG;
typedef DWORDLONG *PDWORDLONG;
typedef DWORDLONG ULONGLONG,*PULONGLONG;
typedef LONGLONG USN;

#define Int32x32To64(a,b) ((LONGLONG)(a)*(LONGLONG)(b))
#define UInt32x32To64(a,b) ((DWORDLONG)(a)*(DWORDLONG)(b))
#define Int64ShllMod32(a,b) ((DWORDLONG)(a)<<(b))
#define Int64ShraMod32(a,b) ((LONGLONG)(a)>>(b))
#define Int64ShrlMod32(a,b) ((DWORDLONG)(a)>>(b))

#define ANSI_NULL '\0'
#define UNICODE_NULL L'\0'
typedef BYTE BOOLEAN,*PBOOLEAN;
#endif
typedef BYTE FCHAR;
typedef WORD FSHORT;
typedef DWORD FLONG;

#define NTAPI __stdcall
#include "platform_basetsd.h"

#define FILE_ATTRIBUTE_READONLY			0x00000001
#define FILE_ATTRIBUTE_HIDDEN			0x00000002
#define FILE_ATTRIBUTE_SYSTEM			0x00000004
#define FILE_ATTRIBUTE_DIRECTORY		0x00000010
#define FILE_ATTRIBUTE_ARCHIVE			0x00000020
#define FILE_ATTRIBUTE_DEVICE			0x00000040
#define FILE_ATTRIBUTE_NORMAL			0x00000080
#define FILE_ATTRIBUTE_TEMPORARY		0x00000100
#define FILE_ATTRIBUTE_SPARSE_FILE		0x00000200
#define FILE_ATTRIBUTE_REPARSE_POINT		0x00000400
#define FILE_ATTRIBUTE_COMPRESSED		0x00000800
#define FILE_ATTRIBUTE_OFFLINE			0x00001000
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED	0x00002000
#define FILE_ATTRIBUTE_ENCRYPTED		0x00004000
#define FILE_ATTRIBUTE_VIRTUAL			0x00010000
#define FILE_ATTRIBUTE_VALID_FLAGS		0x00017fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS		0x000031a7

#define PAGE_NOACCESS	0x0001
#define PAGE_READONLY	0x0002
#define PAGE_READWRITE	0x0004
#define PAGE_WRITECOPY	0x0008
#define PAGE_EXECUTE	0x0010
#define PAGE_EXECUTE_READ	0x0020
#define PAGE_EXECUTE_READWRITE	0x0040
#define PAGE_EXECUTE_WRITECOPY	0x0080
#define PAGE_GUARD		0x0100
#define PAGE_NOCACHE		0x0200
#define PAGE_WRITECOMBINE 0x0400
#define MEM_COMMIT           0x1000
#define MEM_RESERVE          0x2000
#define MEM_DECOMMIT         0x4000
#define MEM_RELEASE          0x8000
#define MEM_FREE            0x10000
#define MEM_PRIVATE         0x20000
#define MEM_MAPPED          0x40000
#define MEM_RESET           0x80000
#define MEM_TOP_DOWN       0x100000
#define MEM_WRITE_WATCH	   0x200000 /* 98/Me */
#define MEM_PHYSICAL	   0x400000
#define MEM_4MB_PAGES    0x80000000

typedef struct _SECURITY_ATTRIBUTES {
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES,*PSECURITY_ATTRIBUTES,*LPSECURITY_ATTRIBUTES;


#ifdef __cplusplus
}
#endif


#endif // LINUX_BUILD

#endif
