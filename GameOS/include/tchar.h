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

#else 

#include<ctype.h>

//typedef char	TCHAR;
//typedef char	_TCHAR;

#define _tcsinc     _strinc
#define _istdigit   isdigit

const char *_strinc( const char *current);

#endif // !_UNICODE

#endif	/* ! _TCHAR_H */
