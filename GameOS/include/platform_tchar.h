#ifndef	_TCHAR_H
#define _TCHAR_H

#ifdef	_UNICODE
#include<wchar.h>
#include<wctype.h>

//typedef	wchar_t	TCHAR;
//typedef wchar_t _TCHAR;

#define _tcsinc _wcsinc
#define _istdigit iswdigit

wchar_t* _wcsinc( const wchar_t *current);

/*
 * __TEXT is a private macro whose specific use is to force the expansion of a
 * macro passed as an argument to the macros _T or _TEXT.  DO NOT use this
 * macro within your programs.  It's name and function could change without
 * notice.
 */
#ifndef __TEXT
#define	__TEXT(q)	L##q
#endif

#else // _UNICODE

#include<ctype.h>

//typedef char	TCHAR;
//typedef char	_TCHAR;

#define _tcsinc     _strinc
#define _istdigit   isdigit

const char *_strinc( const char *current);

/*
 * __TEXT is a private macro whose specific use is to force the expansion of a
 * macro passed as an argument to the macros _T or _TEXT.  DO NOT use this
 * macro within your programs.  It's name and function could change without
 * notice.
 */
#ifndef __TEXT
#define	__TEXT(q)	q
#endif

#endif // !_UNICODE

#ifndef _TEXT
/*
 * UNICODE a constant string when _UNICODE is defined else returns the string
 * unmodified.  Also defined in w32api/winnt.h.
 */
#define _TEXT(x)	__TEXT(x)
#endif

#ifndef _T
#define	_T(x)		__TEXT(x)
#endif

#endif	/* ! _TCHAR_H */
