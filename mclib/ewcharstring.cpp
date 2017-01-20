#define EWCharString_CPP

//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//***************************************************************

/* just for this file, we want UNICODE and K_UNICODE defined and MBCS undefined */
#ifdef UNICODE
#define _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#else
#define UNICODE
#endif /* def UNICODE */

#ifdef K_UNICODE
#define _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#else
#define K_UNICODE
#endif /* def K_UNICODE */

#ifdef MBCS
#define _MBCS_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#define _PREVIOUS_MBCS___EWCharString_CPP		MBCS
#undef MBCS
#endif /* def MBCS */

//***************************************************************



#include"platform_windows.h"
#include<stdio.h>
#include"ewcharstring.h"

#include<gameos.hpp>

#ifdef UNICODE
#include<malloc.h>
#include"platform_tchar.h"
#endif


#ifdef LINUX_BUILD
#include<ctype.h> // isspace
#include<wchar.h> // wcslen
#include<stdarg.h> //va_start
#include "mathfunc.h" // min, max
#endif

#pragma warning( disable:4505 )

#pragma warning( push )
#pragma warning( disable:4100 )

#define INT_SIZE_LENGTH 20
extern int __ismbcodepage;
#define _ISNOTMBCP  (__ismbcodepage == 0)


static wchar_t * __cdecl wideRev (
        wchar_t * string
        )
{
        wchar_t *start = string;
        wchar_t *left = string;
        wchar_t ch;

        while (*string++)                 /* find end of string */
                ;
        string -= 2;

        while (left < string)
        {
                ch = *left;
                *left++ = *string;
                *string-- = ch;
        }

        return(start);
}

static wchar_t * __cdecl wideStrStr (
        const wchar_t * wcs1,
        const wchar_t * wcs2
        )
{
        wchar_t *cp = (wchar_t *) wcs1;
        wchar_t *s1, *s2;

        while (*cp)
        {
                s1 = cp;
                s2 = (wchar_t *) wcs2;

                while ( *s1 && *s2 && !(*s1-*s2) )
                        s1++, s2++;

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}

static int __cdecl wideToInt(
        const wchar_t *nptr
        )
{
        char astring[20];

        WideCharToMultiByte (CP_ACP, 0, nptr, -1,
                            astring, INT_SIZE_LENGTH, NULL, NULL);

        return ((int)atol(astring));
}

static long __cdecl atolong(
        const char *nptr
        )
{
        int c;              /* current char */
        long total;         /* current total */
        int sign;           /* if '-', then negative, otherwise positive */

        /* skip whitespace */
        while ( isspace((int)(unsigned char)*nptr) )
            ++nptr;

        c = (int)(unsigned char)*nptr++;
        sign = c;           /* save sign indication */
        if (c == '-' || c == '+')
            c = (int)(unsigned char)*nptr++;    /* skip sign */

        total = 0;

        while (isdigit(c)) {
            total = 10 * total + (c - '0');     /* accumulate digit */
            c = (int)(unsigned char)*nptr++;    /* get next char */
        }

        if (sign == '-')
            return -total;
        else
            return total;   /* return result, negated if necessary */
}


static char * __cdecl StrStr (
        const char * str1,
        const char * str2
        )
{
        char *cp = (char *) str1;
        char *s1, *s2;

        if ( !*str2 )
            return((char *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (char *) str2;

                while ( *s1 && *s2 && !(*s1-*s2) )
                        s1++, s2++;

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);

}

static char * __cdecl StrRev (
        char * string
        )
{
        char *start = string;
        char *left = string;
        char ch;

        while (*string++)                 /* find end of string */
                ;
        string -= 2;

        while (left < string)
        {
                ch = *left;
                *left++ = *string;
                *string-- = ch;
        }

        return(start);
}


//***************************************************************

#ifdef UNICODE
	#define KToUpper	CharUpperW
	#define KToLower	CharLowerW
	#define	KReverse	wideRev
	#define KStrCmp		lstrcmpiW
	#define klstrlen    lstrlen
	#define KStrStr		wideStrStr
	inline	const EWCSChar* KSInc(const EWCSChar* p) { return p+1; }
	inline size_t EWCSCharLen( const EWCSChar* p ) {return 2;}
	#define KToI		wideToInt
#else 
	#define KToI		atolong
	#define KStrCmp		lstrcmpi
	#define klstrlen	lstrlen
	#define KToUpper	CharUpperA
	#define KToLower	CharLowerA

#ifdef MBCS
	#define	KReverse	mbrev
	#define KStrStr		_mbsstr	
	#define KSInc		_mbsinc
	#define EWCSCharLen	_mbclen

#else // single byte
	#define KReverse		StrRev
	#define KStrStr			StrStr
	__inline char * __cdecl KSInc(const char * _pc) { return (char *)(_pc+1); }
	inline 	int EWCSCharLen(const char* x){ return 1; }	
#endif
#endif



const int EWCharString::INVALID_INDEX = -1;
const int EWCharString::s_Alloc_Allign = 4;
const int EWCharString::s_Force_Ansi =  0x10000;
const int EWCharString::s_Force_Unicode = 0x20000;


// keep around an empty buffer which all of our empty objects use
EWCharString::EBuffer	EWCharString::EBuffer::s_Empty_Buffer = { 0, 0, 0 };
EWCharString::EBuffer*   EWCharString::EBuffer::s_p_Empty_Buffer = &EWCharString::EBuffer::s_Empty_Buffer;

// this must be a macro because the debug versions won't expand
// inlines
#define A2W(p_String) (\
	((const char*)p_String == NULL) ? NULL : (\
	EWCharString::ToUnicode( (LPWSTR)alloca((lstrlenA(p_String)+1)*2),\
	(const unsigned char*)p_String, (lstrlenA(p_String)+1)*2)))

// if this doesn't want to link properly, this will have to become
// a macro.
/////////////////////////////////////////////////////////////////
inline	wchar_t*	EWCharString::ToUnicode( wchar_t* p_Buffer, 
								   const unsigned char* p_Str, 
								   int Num_Chars )
{
	gosASSERT( p_Buffer );
	gosASSERT( p_Str );

	p_Buffer[0] = 0;

	MultiByteToWideChar( CP_ACP, 0, (const char*)p_Str, -1, p_Buffer, Num_Chars );

	return p_Buffer;	
}

/////////////////////////////////////////////////////////////////
inline int EWCharString::StrSize( const EWCSChar* p_Str )
{
	return ( p_Str == NULL  ? 0 : 
#ifdef UNICODE 
	wcslen( p_Str )
#else 
	lstrlen( (const char*)p_Str )
#endif 
		);
}

/////////////////////////////////////////////////////////////////
void	EWCharString::ChecEBufferDoRealloc()
{
	if ( m_pBuffer->m_Ref_Count > 0 )
	{
		m_pBuffer->m_Ref_Count --;

		int Cur_Length = m_pBuffer->m_Data_Length;
		EWCSChar* p_Data = m_pBuffer->Data();

		Alloc( Cur_Length );

		memcpy( m_pBuffer + 1, p_Data, Cur_Length * sizeof( EWCSChar ) );
		m_pBuffer->m_Data_Length = Cur_Length;

	}

}




//***************************************************************

//==============================================================
// Constructors/Destructors
//==============================================================

///////////////////////////////////////////////////////////////
EWCharString::EWCharString( const EWCSChar* p_String ) 
: m_pBuffer(  EBuffer::s_p_Empty_Buffer )
{
	if ( p_String != NULL )
	{
		Assign( p_String );
	}
}

///////////////////////////////////////////////////////////////
EWCharString::EWCharString( const EWCharString& Src_String )
: m_pBuffer( Src_String.m_pBuffer )
{ 
	m_pBuffer->m_Ref_Count++;
}

///////////////////////////////////////////////////////////////
EWCharString::EWCharString() : m_pBuffer(  EBuffer::s_p_Empty_Buffer )
{
}

///////////////////////////////////////////////////////////////
EWCharString::EWCharString( const EWCSChar Char )
{
	Alloc( 2 );
	*(m_pBuffer->Data()) = Char;
	*(m_pBuffer->Data() + 1) = 0;
	m_pBuffer->m_Data_Length = 1;
}

///////////////////////////////////////////////////////////////
EWCharString::~EWCharString()
{
	m_pBuffer->Release();
}

///////////////////////////////////////////////////////////////
void EWCharString::Assign( const EWCSChar* p_String )
{
	
	// handle NULL case
	if ( !p_String )
	{
		m_pBuffer->Release();
		m_pBuffer = EBuffer::s_p_Empty_Buffer;
	}
	
	int Len = StrSize( p_String ) + 1;
		
	// buffer big enough, we can recycle
	if ( m_pBuffer->m_Alloc_Length >= Len )
	{
		memcpy( m_pBuffer + 1, p_String, Len * sizeof( EWCSChar ) );
		m_pBuffer->m_Data_Length = Len - 1;
	}
	else if ( p_String ) // need to allocate new buffer
	{
		m_pBuffer->Release();
		Alloc( Len );
		memcpy( m_pBuffer->Data(), p_String, Len * sizeof( EWCSChar ) );
		m_pBuffer->m_Data_Length = Len - 1;
	}
}

///////////////////////////////////////////////////////////////
void EWCharString::Alloc( int Min_Amount )
{
	// we're rouding up to the nearest multiple of 4 for now
	Min_Amount = (Min_Amount/s_Alloc_Allign + 1) * s_Alloc_Allign;

	m_pBuffer = (EWCharString::EBuffer*)new BYTE[sizeof(EBuffer) + 
		(Min_Amount)*sizeof(EWCSChar)];

	memset( m_pBuffer, 0, sizeof(EBuffer) + (Min_Amount)*sizeof(EWCSChar) );
	
	m_pBuffer->m_Alloc_Length = Min_Amount;
}

///////////////////////////////////////////////////////////////
void EWCharString::Replace( int Start_Index, const EWCSChar* p_String )
{
	// keep the buffer
	EBuffer* p_Tmp = m_pBuffer;
	int Cur_Len = m_pBuffer->m_Data_Length;

	// unshare any shared buffers
	ChecEBuffer(); 

	gosASSERT( Start_Index <= Cur_Len );
	
	// check the current buffer, not the current
	// length
	if ( Start_Index <= Cur_Len )
	{
			
		int Length = StrSize( p_String );

		int Alloc_Length = Start_Index + Length + 1;
		if (  Alloc_Length <= m_pBuffer->m_Alloc_Length )
		{
			memcpy( m_pBuffer->Data() + Start_Index, 
					p_String, Length * sizeof( EWCSChar ) );

			// Add on the NULL if necessary
			if ( Start_Index + Length > Cur_Len	)
			{
				*(m_pBuffer->Data() + Start_Index + Length + 1 ) = '\0';
				m_pBuffer->m_Data_Length = Length + Start_Index;
			}				
		}
		else // need to reallocate here
		{
			EBuffer* p_Old_Buffer = m_pBuffer;

			m_pBuffer =  EBuffer::s_p_Empty_Buffer;

			if ( Cur_Len > Alloc_Length )
			{
				Alloc_Length = Cur_Len;
			}
			
			Alloc( Alloc_Length );
			
			memcpy( m_pBuffer->Data(),
					p_Tmp->Data(), 
					(Cur_Len + 1)*sizeof(EWCSChar) );

			memcpy( m_pBuffer->Data() + Start_Index, 
					p_String, Length*sizeof(EWCSChar) );

			m_pBuffer->m_Data_Length = Alloc_Length;

			p_Old_Buffer->Release();		

		}

		// check to see if the p_String is null
		if ( p_String == NULL )
		{
			*(m_pBuffer->Data() + Start_Index) = 0;
		}

		

	}	
}

///////////////////////////////////////////////////////////////
void EWCharString::Replace( int Start_Index, const EWCharString& String )
{
	Replace( Start_Index, String.m_pBuffer->Data() );
}


///////////////////////////////////////////////////////////////
void EWCharString::Insert( int Start_Index, const EWCSChar* p_String )
{
	if ( Start_Index != INVALID_INDEX && Start_Index <= StrSize( Data() ) )
	{
		EBuffer* p_Tmp = m_pBuffer;

		EBuffer* p_Old_Buffer = NULL;

		ChecEBuffer();

		int Length = StrSize( p_String ); 

		// add on 2 for the 'nulls'
		if ( Length + p_Tmp->m_Data_Length + 1 > m_pBuffer->m_Alloc_Length )
		{
			p_Old_Buffer = m_pBuffer;
			Alloc( Length + p_Tmp->m_Data_Length + 1 );
		}
		
		// use memmove in case we are recycling a buffer.
		memmove( m_pBuffer->Data(), p_Tmp->Data(), Start_Index * sizeof ( EWCSChar ) );
		memmove( m_pBuffer->Data() + (Start_Index + Length), p_Tmp->Data() + (Start_Index),
					(p_Tmp->m_Data_Length - Start_Index) * sizeof (EWCSChar) );
		// write the string last in case we are writing over an old buffer
		memcpy( m_pBuffer->Data() + Start_Index, p_String, Length * sizeof( EWCSChar ) );
	
		
		m_pBuffer->m_Data_Length = Length + p_Tmp->m_Data_Length;

		// Bill added - in some cases, removing some characters then inserting fewer
		//              was leaving garbage at the end of the string.  
		//              i.e., the trailing NULL was not being moved.
		(*(m_pBuffer->Data() + m_pBuffer->m_Data_Length)) = 0;

		// free the old buffer -- can't do this earlier, because we
		// need to copy out of it
		if ( p_Old_Buffer )
		{
			p_Old_Buffer->Release();
		}
	}
}

///////////////////////////////////////////////////////////////
bool EWCharString::Remove( int Start_Index, int End_Index )
{
	// Bill changed - this function could not handle removing a single character 
	// - also this didn't remove the character pointed to by End_Index
	if( Start_Index <= End_Index && End_Index < m_pBuffer->m_Data_Length)
	{
		ChecEBufferDoRealloc();

		memcpy( m_pBuffer->Data() + Start_Index, m_pBuffer->Data() + End_Index + 1,
			(StrSize(m_pBuffer->Data()) - End_Index + 1)*sizeof(EWCSChar) );

		m_pBuffer->m_Data_Length -= (End_Index - Start_Index + 1);

		// Bill added - in some cases, removing some characters then inserting fewer
		//              was leaving garbage at the end of the string.  
		//              i.e., the trailing NULL was not being moved.
		(*(m_pBuffer->Data() + m_pBuffer->m_Data_Length)) = 0;

		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////
bool EWCharString::Remove( EWCharString& Sub_String )
{
	int Index = Find( Sub_String );

	if ( Index != -1 )
	{
		return Remove( Index, Index + Sub_String.StrSize( Sub_String.Data() ) - 1 );
	}

	return false;
}


///////////////////////////////////////////////////////////////
void EWCharString::Swap( EWCharString& Src )
{
	EBuffer* p_Tmp = Src.m_pBuffer;
	Src.m_pBuffer = m_pBuffer;
	m_pBuffer = p_Tmp;
}

///////////////////////////////////////////////////////////////
void EWCharString::MakeUpper()
{
	ChecEBufferDoRealloc();
	KToUpper( m_pBuffer->Data() );
}

///////////////////////////////////////////////////////////////
void EWCharString::MakeLower()
{
	ChecEBufferDoRealloc();
	KToLower( m_pBuffer->Data() );
}

///////////////////////////////////////////////////////////////
void EWCharString::Reverse()
{
	ChecEBufferDoRealloc();
	
	KReverse( m_pBuffer->Data() );
}

// works like sprintf
///////////////////////////////////////////////////////////////
void EWCharString::Format( const EWCSChar* p_Str, ... )
{
	va_list Arg_List;
	
	va_start(Arg_List, p_Str);

    // sebi !NB
	//va_list Arg_List_Save = Arg_List;
	va_list Arg_List_Save;
	va_copy(Arg_List_Save, Arg_List);

	// make a guess at the maximum length of the resulting string
	int Max_Len = 0;
	for (const EWCSChar* p_Tmp = p_Str; *p_Tmp != '\0'; p_Tmp = KSInc(p_Tmp))
	{
		// handle '%' character, but watch out for '%%'
		if (*p_Tmp != '%' || *(p_Tmp = KSInc(p_Tmp)) == '%')
		{
			Max_Len += EWCSCharLen(p_Tmp);
			continue;
		}

		int Item_Len = 0;

		// handle '%' character with format
		int Width = 0;
		for (; *p_Tmp != '\0'; p_Tmp = KSInc(p_Tmp))
		{
			// check for valid flags
			if (*p_Tmp == '#')
				Max_Len += 2;   // for '0x'
			else if (*p_Tmp == '*')
				Width = va_arg(Arg_List, int);
			else if (*p_Tmp == '-' || *p_Tmp == '+' || *p_Tmp == '0' ||
				*p_Tmp == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if ( Width == 0)
		{
			// width indicated by
			Width = KToI(p_Tmp);
			unsigned short buffer;
			for (; *p_Tmp != '\0'; )
			{
				GetStringTypeExW(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, p_Tmp, 1, &buffer);
				if (buffer == C1_DIGIT || buffer == C1_XDIGIT)
					p_Tmp = KSInc(p_Tmp);
				else
					break;
			}
		}
		
		gosASSERT( Width >= 0);

		int Precision = 0;	
		if (*p_Tmp == '.')
		{
			// skip past '.' separator (width.precision)
			p_Tmp = KSInc(p_Tmp);

			// get precision and skip it
			if (*p_Tmp == '*')
			{
				Precision = va_arg(Arg_List, int);
				p_Tmp = KSInc(p_Tmp);
			}
			else
			{
				Precision = KToI(p_Tmp);
				for (; *p_Tmp != '\0'; )
				{
					unsigned short buffer;
					GetStringTypeExW(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, p_Tmp, 1, &buffer);
					//if (buffer == C1_DIGIT || buffer == C1_XDIGIT)
					if (buffer & C1_DIGIT)	//mh
						p_Tmp = KSInc(p_Tmp);
					else
						break;
				}

			}
			gosASSERT( Precision >= 0);
		}

		// should be on type modifier or specifier
		int	Modifier = 0;
		switch (*p_Tmp)
		{
		// modifiers that affect size
		case 'h':
			Modifier = s_Force_Ansi;
			p_Tmp = KSInc(p_Tmp);
			break;
		case 'l':
			Modifier = s_Force_Unicode;
			p_Tmp = KSInc(p_Tmp);
			break;

		// modifiers that do not affect size
		case 'F':
		case 'N':
		case 'L':
			p_Tmp = KSInc(p_Tmp);
			break;
		}

		// now should be on specifier
		switch (*p_Tmp | Modifier)
		{
		// single characters
		case 'c':
		case 'C':
			Item_Len = 2;
            // sebi !NB
			//va_arg(Arg_List, EWCSChar);
			va_arg(Arg_List, int);
			break;
		case 'c'|s_Force_Ansi:
		case 'C'|s_Force_Ansi:
			Item_Len = 2;
            // sebi !NB
            //va_arg(Arg_List, char);
			va_arg(Arg_List, int);
			break;
		case 'c'|s_Force_Unicode:
		case 'C'|s_Force_Unicode:
			Item_Len = 2;
            // sebi !NB
			//va_arg(Arg_List, short);
			va_arg(Arg_List, int);
			break;

		// strings
		case 's':
		{
			const EWCSChar* p_Next_Arg = va_arg(Arg_List, const EWCSChar*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6;  // "(null)"
			else
			{
			   Item_Len = StrSize(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
			break;
		}

		case 'S':
		{
#ifndef K_UNICODE
			const unsigned short* p_Next_Arg = va_arg(Arg_List, const unsigned short*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6;  // "(null)"
			else
			{
			   Item_Len = wcslen(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
#else
			const char* p_Next_Arg = va_arg(Arg_List, const char*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6; // "(null)"
			else
			{
			   Item_Len = lstrlenA(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
#endif
			break;
		}

		case 's'|s_Force_Ansi:
		case 'S'|s_Force_Ansi:
		{
			const char* p_Next_Arg = va_arg(Arg_List, const char*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6; // "(null)"
			else
			{
			   Item_Len = lstrlenA(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
			break;
		}

#ifndef _MAC
		case 's'|s_Force_Unicode:
		case 'S'|s_Force_Unicode:
		{
			LPWSTR pstrNextArg = va_arg(Arg_List, LPWSTR);
			if (pstrNextArg == NULL)
			   Item_Len = 6; // "(null)"
			else
			{
			   Item_Len = wcslen(pstrNextArg);
			   Item_Len = max(1, Item_Len);
			}
			break;
		}
#endif
		}

		// adjust nItemLen for strings
		if (Item_Len != 0)
		{
			Item_Len = max(Item_Len, Width);
			if (Precision != 0)
			{
				Item_Len = min(Item_Len, Precision);
			}
		}
		else
		{
			switch (*p_Tmp)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				va_arg(Arg_List, int);
				Item_Len = 32;
				Item_Len = max(Item_Len, Width + Precision);
				break;

			case 'e':
			case 'f':
			case 'g':
			case 'G':
				va_arg(Arg_List, double);
				Item_Len = 128;
				Item_Len = max(Item_Len, Width + Precision);
				break;

			case 'p':
				va_arg(Arg_List, void*);
				Item_Len = 32;
				Item_Len = max(Item_Len, Width + Precision);
				break;

			// no output
			case 'n':
				va_arg(Arg_List, int*);
				break;
			
			case 'I': // assume INT64 skip next two chars
				p_Tmp = KSInc(p_Tmp);
				p_Tmp = KSInc(p_Tmp);
				p_Tmp = KSInc(p_Tmp);
				Item_Len = 64;
				va_arg(Arg_List, __int64);
				break;


			default:
				gosASSERT(false);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		Max_Len += Item_Len;
	}

	// make sure noone else is using this buffer
	ChecEBuffer();
	
	if ( Max_Len > m_pBuffer->m_Alloc_Length )
	{
		m_pBuffer->Release();
		Alloc( Max_Len + 1 );
	}

#ifdef UNICODE
	//vswprintf(m_pBuffer->Data(), len, p_Str, Arg_List_Save);
    // sebi !NB
    size_t len = StrSize( m_pBuffer->Data());
	vswprintf(m_pBuffer->Data(), len, p_Str, Arg_List_Save);
#else
	vsprintf(m_pBuffer->Data(), p_Str, Arg_List_Save);
#endif
    //sebi
    va_end(Arg_List_Save);


	m_pBuffer->m_Data_Length = StrSize( m_pBuffer->Data() );

	// clean up
	va_end( Arg_List );
	
}

/////////////////////////////////////////////////////////////////
EWCharString operator+(	const EWCharString& Begin_String, 
					const EWCharString& End_String )
{
	return Begin_String + End_String.m_pBuffer->Data();
}

/////////////////////////////////////////////////////////////////
EWCharString operator+( const EWCSChar* p_Begin_String, 
				   const EWCharString& End_String )
{
	EWCharString Ret_String;

	int Length = EWCharString::StrSize( p_Begin_String );
	
	Ret_String.Alloc( End_String.m_pBuffer->m_Data_Length + Length + 1 );

	memcpy( Ret_String.m_pBuffer->Data(), p_Begin_String, Length );

	memcpy( Ret_String.m_pBuffer->Data() + Length, 
			End_String.m_pBuffer->Data(),
			End_String.m_pBuffer->m_Data_Length );

	Ret_String.m_pBuffer->m_Data_Length = Length + End_String.m_pBuffer->m_Data_Length;

	return Ret_String;

}

/////////////////////////////////////////////////////////////////
EWCharString operator+( const EWCharString& Begin_String, const EWCSChar* p_End_String )
{
	EWCharString Ret_String;

	int Length = EWCharString::StrSize( p_End_String );
	
	Ret_String.Alloc( Begin_String.m_pBuffer->m_Data_Length + Length + 1 );

	memcpy( Ret_String.m_pBuffer->Data(), 
			Begin_String.m_pBuffer->Data(), 
			Begin_String.m_pBuffer->m_Data_Length);

	memcpy( Ret_String.m_pBuffer->Data() + Begin_String.m_pBuffer->m_Data_Length, 
			p_End_String, Length );

	Ret_String.m_pBuffer->m_Data_Length = Length + Begin_String.m_pBuffer->m_Data_Length;

	return Ret_String;

}

/////////////////////////////////////////////////////////////////
EWCharString operator+( const EWCharString& Begin_String,  EWCSChar Char )
{
	EWCSChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;

	return Begin_String + Char;
}

/////////////////////////////////////////////////////////////////
EWCharString operator+( const EWCSChar Char,  const  EWCharString& Begin_String )
{
	EWCSChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;

	return Tmp + Begin_String;
}


/////////////////////////////////////////////////////////////////
int EWCharString::Compare( const EWCharString& Str_To_Compare, bool Case_Sensitive ) const
{
	return Compare( Str_To_Compare.m_pBuffer->Data(), Case_Sensitive );
}

/////////////////////////////////////////////////////////////////
int EWCharString::Compare( const EWCSChar* p_String, bool Case_Sensitive ) const
{
	
	int Length = StrSize( p_String );

	if ( 0 == StrSize(m_pBuffer->Data()) )
	{
		if ( Length )
			return -1;
		return 0;
	}

	if ( !Length )
	{
		return 1;
	}
	
	//if ( m_pBuffer->m_Data_Length != Length )
	//{
	//	return ( m_pBuffer->m_Data_Length < Length ? -1 : 1 );
	//}
	if ( Case_Sensitive )
	{
		return ( memcmp( m_pBuffer->Data(), p_String, Length ) );
	}	
	return (KStrCmp( m_pBuffer->Data(), p_String ) );
	
}
 	
/////////////////////////////////////////////////////////////////
int EWCharString::Size() const 	// number of bytes
{
	return m_pBuffer->m_Data_Length * sizeof(EWCSChar);
}


/////////////////////////////////////////////////////////////////
int EWCharString::Length() const	// number of characters
{

#ifdef UNICODE
	return wcslen( m_pBuffer->Data() );
#else
	return lstrlen( (char*)m_pBuffer->Data() );
#endif
}

/////////////////////////////////////////////////////////////////
int EWCharString::Find( EWCSChar Char, int Start_Index ) const
{
	if ( Start_Index == -1 )
	{
		Start_Index = 0;
	}
	
	EWCSChar* p_Tmp = m_pBuffer->Data(); 

	// Bill added this line - this function ingored the start index
	p_Tmp += Start_Index;

	while( *p_Tmp )
	{
		if ( *p_Tmp == Char )
		{
			return p_Tmp - m_pBuffer->Data();
		}
		
		p_Tmp++;
		
	}

	return INVALID_INDEX;

}

/////////////////////////////////////////////////////////////////
int EWCharString::Find( const EWCharString& Str_To_Find, int Start_Index ) const
{
	if ( -1 == Start_Index )
	{
		Start_Index = 0;
	}
	
	EWCSChar* p_Tmp = KStrStr( m_pBuffer->Data() + Start_Index, 
							Str_To_Find.m_pBuffer->Data() );

	return ( p_Tmp ? p_Tmp - m_pBuffer->Data() : INVALID_INDEX );
}

/////////////////////////////////////////////////////////////////
int EWCharString::Find( const EWCSChar* p_Str_To_Find, int Start_Index ) const
{
	if ( -1 == Start_Index )
	{
		Start_Index = 0;
	}

	
	EWCSChar* p_Tmp = KStrStr( m_pBuffer->Data() + Start_Index, 
							p_Str_To_Find );

	return ( p_Tmp ? p_Tmp - m_pBuffer->Data() : INVALID_INDEX );

}

/////////////////////////////////////////////////////////////////
int EWCharString::ReverseFind ( EWCSChar Char, int End_Index ) const
{
	if ( -1 == End_Index )
	{
		End_Index = m_pBuffer->m_Data_Length;
	}
	
	EWCSChar* p_Tmp = m_pBuffer->Data() + End_Index; 

	while( End_Index >= 0 )
	{
		if ( *p_Tmp == Char )
		{
			return End_Index;
		}
		
		p_Tmp --;
		End_Index --;
	
	}

	return INVALID_INDEX;
}

/////////////////////////////////////////////////////////////////
#ifndef UNICODE
int EWCharString::Find( unsigned short Char, int Start_Index ) const
{
	unsigned short Tmp[2];
	*Tmp = Char;
	Tmp[2] = 0;

	return Find( (EWCSChar*)Tmp, Start_Index );
}
#endif // !K_UNICODE
	
/////////////////////////////////////////////////////////////////
EWCharString EWCharString::SubString( int Start_Index, int End_Index ) const
{
	EWCharString Ret_String;

	// Bill changed so that it can return a single character
	// (Start == End)
	gosASSERT( Start_Index <= End_Index && 
		End_Index < m_pBuffer->m_Data_Length );

	Ret_String.Alloc( End_Index - Start_Index + 2);

	EWCSChar* p_Dest = Ret_String.m_pBuffer->Data();

	EWCSChar* p_Src = m_pBuffer->Data() + Start_Index;

	memcpy( p_Dest, 
			p_Src,
			(End_Index - Start_Index + 1) * sizeof(EWCSChar) );

	*(p_Dest + End_Index - Start_Index + 1) = 0;

	Ret_String.m_pBuffer->m_Data_Length = End_Index - Start_Index + 1;

	return Ret_String;
}



/////////////////////////////////////////////////////////////////
wchar_t* EWCharString::CreateUNICODE() const
{
#ifdef UNICODE
	wchar_t* p_Ret_String = new wchar_t[m_pBuffer->m_Data_Length + 1];
	memcpy( p_Ret_String, m_pBuffer->Data(), 2*(m_pBuffer->m_Data_Length + 1) );
	return p_Ret_String;
#else
	 wchar_t* p_Ret_String = new wchar_t[lstrlen((char*)m_pBuffer->Data()) + 1];
	 ToUnicode( p_Ret_String, (unsigned char*)m_pBuffer->Data(), m_pBuffer->m_Data_Length + 1 );
	 return p_Ret_String;
#endif
}
/////////////////////////////////////////////////////////////////
char* 	EWCharString::CreateMBCS() const
{

	char* p_Ret_String = new char[m_pBuffer->m_Data_Length + 1];

#ifdef K_UNICODE
	wcstombs( p_Ret_String, m_pBuffer->Data(), m_pBuffer->m_Data_Length + 1  );

#else
	 memcpy( p_Ret_String, m_pBuffer->Data(), m_pBuffer->m_Data_Length + 1 );
#endif

	return p_Ret_String;

}



 	



///****************************************************************
#ifdef K_UNICODE

/////////////////////////////////////////////////////////////////
EWCharString::EWCharString(const  char* p_String ) 
: m_pBuffer( EBuffer::s_p_Empty_Buffer ) 
{
	if ( p_String )
	{
		Assign( A2W( p_String ) );
	}
}
		
/////////////////////////////////////////////////////////////////
const EWCharString& EWCharString::operator+=( const char* p_String )
{
	if ( p_String )
	{
		*this += A2W( p_String );
	}

	return *this;
}

/////////////////////////////////////////////////////////////////
const EWCharString& EWCharString::operator+=( char Char )
{
	
	char Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;
	*this += A2W( Tmp );

	return *this;	
}


/////////////////////////////////////////////////////////////////
EWCharString operator+( const EWCharString& Begin_String, char* p_End_String )
{
	EWCharString RetString( Begin_String );
	
	if ( p_End_String )
	{
		RetString += A2W( p_End_String );
	}

	return RetString;
}

/////////////////////////////////////////////////////////////////
EWCharString operator+( char* p_Begin_String, const EWCharString& End_String )
{
	EWCharString RetString( A2W( p_Begin_String) +  End_String );

	return RetString;
}

/////////////////////////////////////////////////////////////////
EWCharString operator+( char Char, const EWCharString& End_String )
{
	char Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;

	EWCharString	RetString( A2W( Tmp ) + End_String );
	
	return RetString;
}

/////////////////////////////////////////////////////////////////
bool EWCharString::operator<( const char* p_String ) const
{
	return operator<(A2W( p_String ) );
}

/////////////////////////////////////////////////////////////////
bool operator<( const char* p_String, const EWCharString& String )
{
	return operator<(A2W(p_String), String ); 
}
	
/////////////////////////////////////////////////////////////////
bool EWCharString::operator>( const char* p_String ) const
{
	return EWCharString::operator>(A2W( p_String) );
}

/////////////////////////////////////////////////////////////////
bool operator>( const char* p_String, const EWCharString& String )
{
	return operator>( A2W(p_String), String );
}
/////////////////////////////////////////////////////////////////
bool EWCharString::operator<=( const char* p_String) const
{
	return EWCharString::operator<=( A2W(p_String) );
}

/////////////////////////////////////////////////////////////////
bool operator<=( const char* p_String, const EWCharString& String)
{
	return ( A2W( p_String ) <= String );
}

/////////////////////////////////////////////////////////////////
bool EWCharString::operator>=( const char* p_String) const
{
	return EWCharString::operator>=( A2W(p_String) );
}
/////////////////////////////////////////////////////////////////
bool operator>=( const char* p_String, const EWCharString& String)
{
	return ( A2W( p_String ) >= String );
}

/////////////////////////////////////////////////////////////////
void EWCharString::Format( const char* p_Str, ... )
{
	// NOTE, we are assuming that the end user has
	// made ALL STRING parameters UNICODE
	va_list Arg_List;
	
	va_start(Arg_List, p_Str);

    //sebi !NB
	//va_list Arg_List_Save = Arg_List;
	va_list Arg_List_Save;
	va_copy(Arg_List_Save, Arg_List);

	// make a guess at the maximum length of the resulting string
	int Max_Len = 0;
    // sebi !NB check this!!! it is incorrect to just step one byte a t a time
    // most probably tcsinc is not crrectly implemented
	for (const char* p_Tmp = p_Str; *p_Tmp != '\0'; p_Tmp++)
	{
		// handle '%' character, but watch out for '%%'
		if (*p_Tmp != '%' || *(p_Tmp = _tcsinc(p_Tmp)) == '%')
		{
			Max_Len += lstrlenA(p_Tmp);
			continue;
		}

		int Item_Len = 0;

		// handle '%' character with format
		int Width = 0;
		for (; *p_Tmp != '\0'; p_Tmp = _tcsinc(p_Tmp))
		{
			// check for valid flags
			if (*p_Tmp == '#')
				Max_Len += 2;   // for '0x'
			else if (*p_Tmp == '*')
				Width = va_arg(Arg_List, int);
			else if (*p_Tmp == '-' || *p_Tmp == '+' || *p_Tmp == '0' ||
				*p_Tmp == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if ( Width == 0)
		{
			// width indicated by
			Width = atoi(p_Tmp);
			for (; *p_Tmp != '\0' && _istdigit(*p_Tmp); p_Tmp = _tcsinc(p_Tmp))
				;
		}
		
		gosASSERT( Width >= 0);

		int Precision = 0;
		if (*p_Tmp == '.')
		{
			// skip past '.' separator (width.precision)
			p_Tmp = _tcsinc(p_Tmp);

			// get precision and skip it
			if (*p_Tmp == '*')
			{
				Precision = va_arg(Arg_List, int);
				p_Tmp = _tcsinc(p_Tmp);
			}
			else
			{
				Precision = atoi(p_Tmp);
				for (; *p_Tmp != '\0' && _istdigit(*p_Tmp); p_Tmp = _tcsinc(p_Tmp))
					;
			}
			gosASSERT( Precision >= 0);
		}

		// should be on type modifier or specifier
		int	Modifier = 0;
		switch (*p_Tmp)
		{
		// modifiers that affect size
		case 'h':
			Modifier = s_Force_Ansi;
			p_Tmp = _tcsinc(p_Tmp);
			break;
		case 'l':
			Modifier = s_Force_Unicode;
			p_Tmp = _tcsinc(p_Tmp);
			break;

		// modifiers that do not affect size
		case 'F':
		case 'N':
		case 'L':
			p_Tmp = _tcsinc(p_Tmp);
			break;
		}

		// now should be on specifier
		switch (*p_Tmp | Modifier)
		{
		// single characters
		case 'c':
		case 'C':
			Item_Len = 2;
			// here we are asuming regular char
            // sebi !NB
			//va_arg(Arg_List, char);
			va_arg(Arg_List, int);
			break;
		case 'c'|s_Force_Ansi:
		case 'C'|s_Force_Ansi:
			Item_Len = 2;
            // sebi !NB
			//va_arg(Arg_List, char);
			va_arg(Arg_List, int);
			break;
		case 'c'|s_Force_Unicode:
		case 'C'|s_Force_Unicode:
			Item_Len = 2;
            // sebi !NB
			//va_arg(Arg_List, short);
			va_arg(Arg_List, int);
			break;

		// strings
		case 's':
		{
			const char* p_Next_Arg = va_arg(Arg_List, const char*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6;  // "(null)"
			else
			{
			   Item_Len = lstrlenA(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
			break;
		}

		case 'S':
		{
#ifndef K_UNICODE
			const unsigned short* p_Next_Arg = va_arg(Arg_List, const unsigned short*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6;  // "(null)"
			else
			{
			   Item_Len = wcslen(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
#else
			const char* p_Next_Arg = va_arg(Arg_List, const char*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6; // "(null)"
			else
			{
			   Item_Len = lstrlenA(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
#endif
			break;
		}

		case 's'|s_Force_Ansi:
		case 'S'|s_Force_Ansi:
		{
			const char* p_Next_Arg = va_arg(Arg_List, const char*);
			if (p_Next_Arg == NULL)
			   Item_Len = 6; // "(null)"
			else
			{
			   Item_Len = lstrlenA(p_Next_Arg);
			   Item_Len = max(1, Item_Len);
			}
			break;
		}

#ifndef _MAC
		case 's'|s_Force_Unicode:
		case 'S'|s_Force_Unicode:
		{
			LPWSTR pstrNextArg = va_arg(Arg_List, LPWSTR);
			if (pstrNextArg == NULL)
			   Item_Len = 6; // "(null)"
			else
			{
			   Item_Len = wcslen(pstrNextArg);
			   Item_Len = max(1, Item_Len);
			}
			break;
		}
#endif
		}

		// adjust nItemLen for strings
		if (Item_Len != 0)
		{
			Item_Len = max(Item_Len, Width);
			if (Precision != 0)
			{
				Item_Len = min(Item_Len, Precision);
			}
		}
		else
		{
			switch (*p_Tmp)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				va_arg(Arg_List, int);
				Item_Len = 32;
				Item_Len = max(Item_Len, Width + Precision);
				break;

			case 'e':
			case 'f':
			case 'g':
			case 'G':
				va_arg(Arg_List, double);
				Item_Len = 128;
				Item_Len = max(Item_Len, Width + Precision);
				break;

			case 'p':
				va_arg(Arg_List, void*);
				Item_Len = 32;
				Item_Len = max(Item_Len, Width + Precision);
				break;

			// no output
			case 'n':
				va_arg(Arg_List, int*);
				break;

			case 'I': // assume INT64 skip next two chars
				p_Tmp = _tcsinc(p_Tmp);
				p_Tmp = _tcsinc(p_Tmp);
				p_Tmp = _tcsinc(p_Tmp);
				Item_Len = 64;
				va_arg(Arg_List, __int64);
				break;

			default:
				gosASSERT(false);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		Max_Len += Item_Len;
	}

	// make sure noone else is using this buffer
	ChecEBuffer();
	
	if ( Max_Len > m_pBuffer->m_Alloc_Length )
	{
		m_pBuffer->Release();
		Alloc( Max_Len + 1 );
	}

	char* pTmp = new char[ Max_Len + 1];

	vsprintf(pTmp, p_Str, Arg_List_Save);
    // sebi
    va_end(Arg_List_Save);

	memcpy( m_pBuffer->Data(), A2W(pTmp), (Max_Len + 1)*sizeof(EWCSChar) );

	m_pBuffer->m_Data_Length = StrSize( m_pBuffer->Data() );

	// clean up
	va_end( Arg_List );
	

	
}

/////////////////////////////////////////////////////////////////
bool EWCharString::operator==( const char* p_String )
{
	return operator==( A2W( p_String ) );
}

/////////////////////////////////////////////////////////////////
bool operator==( const char* p_String, const EWCharString& String)
{
	return operator==( A2W( p_String ), String );
}

/////////////////////////////////////////////////////////////////
bool EWCharString::operator!=( const char* p_String) const
{
	return operator!=( A2W(p_String) );
}

/////////////////////////////////////////////////////////////////
bool operator!=( const char* p_String, const EWCharString& String )
{
	return operator!=( A2W(p_String), String ); 
}

/////////////////////////////////////////////////////////////////
int EWCharString::Find( char Char, int Start_Index ) const
{
	EWCSChar Tmp; 

	mbtowc( &Tmp, &Char, 1 );
	
	return Find( Tmp, Start_Index );
}

/////////////////////////////////////////////////////////////////
const EWCharString& EWCharString::operator=( char Char )
{
	EWCSChar Tmp; 

	mbtowc( &Tmp, &Char, 1 );

	return operator=( Tmp );
}



#endif // Unicode




//***************************************************************

/* restore UNICODE, K_UNICODE and MBCS to their previous state */
#ifndef _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#undef UNICODE
#else
#undef _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#endif /* ndef _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP */

#ifndef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#undef K_UNICODE
#else
#undef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#endif /* ndef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP */

#ifdef _MBCS_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#undef _MBCS_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP
#define MBCS	_PREVIOUS_MBCS___EWCharString_CPP
#undef _PREVIOUS_MBCS___EWCharString_CPP
#endif /* def _MBCS_WAS_PREVIOUSLY_DEFINED___EWCharString_CPP */

//***************************************************************

//****************************************************************

#pragma warning( pop )
