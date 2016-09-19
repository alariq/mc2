#define EString_CPP

//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
#include<windows.h>
#include"estring.h"

#include<gameos.hpp>

#include<stdio.h>

#ifdef UNICODE
#define K_UNICODE
#include<malloc.h>
#include<tchar.h>
#endif

#ifdef LINUX_BUILD
#include<ctype.h> // isspace
#include<stdarg.h> // va_start
#include<wchar.h> // wcslen
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
	#define KToUpper	CharUpper
	#define KToLower	CharLower
	#define	KReverse	wideRev
	#define KStrCmp		lstrcmpi
	#define klstrlen		lstrlen
	#define KStrStr		wideStrStr
	inline	const EChar* KSInc(const EChar* p) { return p+1; }
	inline size_t ECharLen( const EChar* p ) {return 2;}
	#define KToI		wideToInt
#else 
	#define KToI		atolong
	#define KStrCmp		lstrcmpi
	#define klstrlen	lstrlen
	#define KToUpper	CharUpper
	#define KToLower	CharLower

#ifdef MBCS
	#define	KReverse	mbrev
	#define KStrStr		_mbsstr	
	#define KSInc		_mbsinc
	#define ECharLen	_mbclen

#else // single byte
	#define KReverse		StrRev
	#define KStrStr			StrStr
	__inline char * __cdecl KSInc(const char * _pc) { return (char *)(_pc+1); }
	inline 	int ECharLen(const char* x){ return 1; }	
#endif
#endif



const int EString::INVALID_INDEX = -1;
const int EString::s_Alloc_Allign = 4;
const int EString::s_Force_Ansi =  0x10000;
const int EString::s_Force_Unicode = 0x20000;


// keep around an empty buffer which all of our empty objects use
EString::EBuffer	EString::EBuffer::s_Empty_Buffer = { 0, 0, 0 };
EString::EBuffer*   EString::EBuffer::s_p_Empty_Buffer = &EString::EBuffer::s_Empty_Buffer;

// this must be a macro because the debug versions won't expand
// inlines
#define A2W(p_String) (\
	((const char*)p_String == NULL) ? NULL : (\
	EString::ToUnicode( (LPWSTR)_alloca((lstrlenA(p_String)+1)*2),\
	(const unsigned char*)p_String, (lstrlenA(p_String)+1)*2)))

// if this doesn't want to link properly, this will have to become
// a macro.
/////////////////////////////////////////////////////////////////
inline	wchar_t*	EString::ToUnicode( wchar_t* p_Buffer, 
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
inline int EString::StrSize( const EChar* p_Str )
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
void	EString::ChecEBufferDoRealloc()
{
	if ( m_pBuffer->m_Ref_Count > 0 )
	{
		m_pBuffer->m_Ref_Count --;

		int Cur_Length = m_pBuffer->m_Data_Length;
		EChar* p_Data = m_pBuffer->Data();

		Alloc( Cur_Length );

		memcpy( m_pBuffer + 1, p_Data, Cur_Length * sizeof( EChar ) );
		m_pBuffer->m_Data_Length = Cur_Length;

	}

}




//***************************************************************

//==============================================================
// Constructors/Destructors
//==============================================================

///////////////////////////////////////////////////////////////
EString::EString( const EChar* p_String ) 
: m_pBuffer(  EBuffer::s_p_Empty_Buffer )
{
	if ( p_String != NULL )
	{
		Assign( p_String );
	}
}

///////////////////////////////////////////////////////////////
EString::EString( const EString& Src_String )
: m_pBuffer( Src_String.m_pBuffer )
{ 
	m_pBuffer->m_Ref_Count++;
}

///////////////////////////////////////////////////////////////
EString::EString() : m_pBuffer(  EBuffer::s_p_Empty_Buffer )
{
}

///////////////////////////////////////////////////////////////
EString::EString( const EChar Char )
{
	Alloc( 2 );
	*(m_pBuffer->Data()) = Char;
	*(m_pBuffer->Data() + 1) = 0;
	m_pBuffer->m_Data_Length = 1;
}

///////////////////////////////////////////////////////////////
EString::~EString()
{
	m_pBuffer->Release();
}

///////////////////////////////////////////////////////////////
void EString::Assign( const EChar* p_String )
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
		memcpy( m_pBuffer + 1, p_String, Len * sizeof( EChar ) );
		m_pBuffer->m_Data_Length = Len - 1;
	}
	else if ( p_String ) // need to allocate new buffer
	{
		m_pBuffer->Release();
		Alloc( Len );
		memcpy( m_pBuffer->Data(), p_String, Len * sizeof( EChar ) );
		m_pBuffer->m_Data_Length = Len - 1;
	}
}

///////////////////////////////////////////////////////////////
void EString::Alloc( int Min_Amount )
{
	// we're rouding up to the nearest multiple of 4 for now
	Min_Amount = (Min_Amount/s_Alloc_Allign + 1) * s_Alloc_Allign;

	m_pBuffer = (EString::EBuffer*)malloc(sizeof(EBuffer) + 
		(Min_Amount)*sizeof(EChar));

	memset( m_pBuffer, 0, sizeof(EBuffer) + (Min_Amount)*sizeof(EChar) );
	
	m_pBuffer->m_Alloc_Length = Min_Amount;
}

void EString::SetBufferSize( int size )
{
	EBuffer* p_Old_Buffer = m_pBuffer;
	
	Alloc( size );

	p_Old_Buffer->Release();
}

///////////////////////////////////////////////////////////////
void EString::Replace( int Start_Index, const EChar* p_String )
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
					p_String, Length * sizeof( EChar ) );

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
					(Cur_Len + 1)*sizeof(EChar) );

			memcpy( m_pBuffer->Data() + Start_Index, 
					p_String, Length*sizeof(EChar) );

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
void EString::Replace( int Start_Index, const EString& String )
{
	Replace( Start_Index, String.m_pBuffer->Data() );
}


///////////////////////////////////////////////////////////////
void EString::Insert( int Start_Index, const EChar* p_String )
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
		memmove( m_pBuffer->Data(), p_Tmp->Data(), Start_Index * sizeof ( EChar ) );
		memmove( m_pBuffer->Data() + (Start_Index + Length), p_Tmp->Data() + (Start_Index),
					(p_Tmp->m_Data_Length - Start_Index) * sizeof (EChar) );
		// write the string last in case we are writing over an old buffer
		memcpy( m_pBuffer->Data() + Start_Index, p_String, Length * sizeof( EChar ) );
	
		
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
bool EString::Remove( int Start_Index, int End_Index )
{
	// Bill changed - this function could not handle removing a single character 
	// - also this didn't remove the character pointed to by End_Index
	if( Start_Index <= End_Index && End_Index < m_pBuffer->m_Data_Length)
	{
		ChecEBufferDoRealloc();

		memmove( m_pBuffer->Data() + Start_Index, m_pBuffer->Data() + End_Index + 1,
			(StrSize(m_pBuffer->Data()) - End_Index - 1)*sizeof(EChar) );

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
bool EString::Remove( const EString& Sub_String )
{
	int Index = Find( Sub_String );

	if ( Index != -1 )
	{
		return Remove( Index, Index + Sub_String.StrSize( Sub_String.Data() ) - 1 );
	}

	return false;
}


///////////////////////////////////////////////////////////////
void EString::Swap( EString& Src )
{
	EBuffer* p_Tmp = Src.m_pBuffer;
	Src.m_pBuffer = m_pBuffer;
	m_pBuffer = p_Tmp;
}

///////////////////////////////////////////////////////////////
void EString::MakeUpper()
{
	ChecEBufferDoRealloc();
	KToUpper( m_pBuffer->Data() );
}

///////////////////////////////////////////////////////////////
void EString::MakeLower()
{
	ChecEBufferDoRealloc();
	KToLower( m_pBuffer->Data() );
}

///////////////////////////////////////////////////////////////
void EString::Reverse()
{
	ChecEBufferDoRealloc();
	
	KReverse( m_pBuffer->Data() );
}

// works like sprintf
///////////////////////////////////////////////////////////////
void EString::Format( const EChar* p_Str, ... )
{
	va_list Arg_List;
	
	va_start(Arg_List, p_Str);

    // sebi !NB
	//va_list Arg_List_Save = Arg_List;
	va_list Arg_List_Save;
    va_copy(Arg_List_Save, Arg_List);

	// make a guess at the maximum length of the resulting string
	int Max_Len = 0;
	for (const EChar* p_Tmp = p_Str; *p_Tmp != '\0'; p_Tmp = KSInc(p_Tmp))
	{
		// handle '%' character, but watch out for '%%'
		if (*p_Tmp != '%' || *(p_Tmp = KSInc(p_Tmp)) == '%')
		{
			Max_Len += ECharLen(p_Tmp);
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
				GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, p_Tmp, 1, &buffer);
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
					GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, p_Tmp, 1, &buffer);
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
            // sebi: char is promoted to int
			//va_arg(Arg_List, EChar);
			va_arg(Arg_List, int);
			break;
		case 'c'|s_Force_Ansi:
		case 'C'|s_Force_Ansi:
			Item_Len = 2;
            // sebi: char is promoted to int
			//va_arg(Arg_List, char);
			va_arg(Arg_List, int);
			break;
		case 'c'|s_Force_Unicode:
		case 'C'|s_Force_Unicode:
			Item_Len = 2;
            // sebi: char is promoted to int
			//va_arg(Arg_List, short);
			va_arg(Arg_List, int);
			break;

		// strings
		case 's':
		{
			const EChar* p_Next_Arg = va_arg(Arg_List, const EChar*);
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
            // sebi
			const wchar_t* p_Next_Arg = va_arg(Arg_List, const wchar_t*);
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
	vswprintf(m_pBuffer->Data(), p_Str, Arg_List_Save);
#else
	vsprintf(m_pBuffer->Data(), p_Str, Arg_List_Save);
#endif
    va_end(Arg_List_Save);


	m_pBuffer->m_Data_Length = StrSize( m_pBuffer->Data() );

	// clean up
	va_end( Arg_List );
	
}

/////////////////////////////////////////////////////////////////
EString operator+(	const EString& Begin_String, 
					const EString& End_String )
{
	return Begin_String + (const EChar*)(End_String.m_pBuffer->Data());
}

/////////////////////////////////////////////////////////////////
EString operator+( const EChar* p_Begin_String, 
				   const EString& End_String )
{
	EString Ret_String;

	int Length = EString::StrSize( p_Begin_String );
	
	Ret_String.Alloc( End_String.m_pBuffer->m_Data_Length + Length + 1 );

	memcpy( Ret_String.m_pBuffer->Data(), p_Begin_String, Length );

	memcpy( Ret_String.m_pBuffer->Data() + Length, 
			End_String.m_pBuffer->Data(),
			End_String.m_pBuffer->m_Data_Length );

	Ret_String.m_pBuffer->m_Data_Length = Length + End_String.m_pBuffer->m_Data_Length;

	return Ret_String;

}

/////////////////////////////////////////////////////////////////
EString operator+( const EString& Begin_String, const EChar* p_End_String )
{
	EString Ret_String;

	int Length = EString::StrSize( p_End_String );
	
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
EString operator+( const EString& Begin_String,  EChar Char )
{
	EChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;

	return Begin_String + Char;
}

/////////////////////////////////////////////////////////////////
EString operator+( const EChar Char,  const  EString& Begin_String )
{
	EChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;

	return Tmp + Begin_String;
}


/////////////////////////////////////////////////////////////////
int EString::Compare( const EString& Str_To_Compare, bool Case_Sensitive ) const
{
	return Compare( Str_To_Compare.m_pBuffer->Data(), Case_Sensitive );
}

/////////////////////////////////////////////////////////////////
int EString::Compare( const EChar* p_String, bool Case_Sensitive ) const
{
	
	int Length = StrSize( p_String );

	int myLength = StrSize(m_pBuffer->Data());

	if ( 0 ==  myLength )
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
		if ( 0 == memcmp( m_pBuffer->Data(), p_String, Length ) )
		{
			if ( Length == myLength )
				return 0;

			else if ( myLength > Length )
				return 1;
			else
				return -1;
		}
		
	}	
	return (KStrCmp( m_pBuffer->Data(), p_String ) );
	
}
 	
/////////////////////////////////////////////////////////////////
int EString::Size() const 	// number of bytes
{
	return m_pBuffer->m_Data_Length * sizeof(EChar);
}


/////////////////////////////////////////////////////////////////
int EString::Length() const	// number of characters
{

	if ( !m_pBuffer->Data() )
		return 0;
#ifdef UNICODE
	return wcslen( m_pBuffer->Data() );
#else
	return lstrlen( (char*)m_pBuffer->Data() );
#endif
}

/////////////////////////////////////////////////////////////////
int EString::Find( EChar Char, int Start_Index ) const
{
	if ( Start_Index == -1 )
	{
		Start_Index = 0;
	}
	
	EChar* p_Tmp = m_pBuffer->Data(); 

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
int EString::Find( const EString& Str_To_Find, int Start_Index ) const
{
	if ( -1 == Start_Index )
	{
		Start_Index = 0;
	}
	
	EChar* p_Tmp = KStrStr( m_pBuffer->Data() + Start_Index, 
							Str_To_Find.m_pBuffer->Data() );

	return ( p_Tmp ? p_Tmp - m_pBuffer->Data() : INVALID_INDEX );
}

/////////////////////////////////////////////////////////////////
int EString::Find( const EChar* p_Str_To_Find, int Start_Index ) const
{
	if ( -1 == Start_Index )
	{
		Start_Index = 0;
	}

	
	EChar* p_Tmp = KStrStr( m_pBuffer->Data() + Start_Index, 
							p_Str_To_Find );

	return ( p_Tmp ? p_Tmp - m_pBuffer->Data() : INVALID_INDEX );

}

/////////////////////////////////////////////////////////////////
int EString::ReverseFind ( EChar Char, int End_Index ) const
{
	if ( -1 == End_Index )
	{
		End_Index = m_pBuffer->m_Data_Length;
	}
	
	EChar* p_Tmp = m_pBuffer->Data() + End_Index; 

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
int EString::Find( unsigned short Char, int Start_Index ) const
{
	unsigned short Tmp[2];
	*Tmp = Char;
	Tmp[2] = 0;

	return Find( (EChar*)Tmp, Start_Index );
}
#endif // !K_UNICODE
	
/////////////////////////////////////////////////////////////////
EString EString::SubString( int Start_Index, int End_Index ) const
{
	EString Ret_String;

	// Bill changed so that it can return a single character
	// (Start == End)
	gosASSERT( Start_Index <= End_Index && 
		End_Index < m_pBuffer->m_Data_Length );

	Ret_String.Alloc( End_Index - Start_Index + 2);

	EChar* p_Dest = Ret_String.m_pBuffer->Data();

	EChar* p_Src = m_pBuffer->Data() + Start_Index;

	memcpy( p_Dest, 
			p_Src,
			(End_Index - Start_Index + 1) * sizeof(EChar) );

	*(p_Dest + End_Index - Start_Index + 1) = 0;

	Ret_String.m_pBuffer->m_Data_Length = End_Index - Start_Index + 1;

	return Ret_String;
}



/////////////////////////////////////////////////////////////////
wchar_t* EString::CreateUNICODE() const
{
#ifdef UNICODE
	unsigned short* p_Ret_String = new unsigned short[m_pBuffer->m_Data_Length + 1];
	memcpy( p_Ret_String, m_pBuffer->Data(), 2*(m_pBuffer->m_Data_Length + 1) );
	return p_Ret_String;
#else
	 wchar_t* p_Ret_String = new wchar_t[lstrlen((char*)m_pBuffer->Data()) + 1];
	 ToUnicode( p_Ret_String, (unsigned char*)m_pBuffer->Data(), m_pBuffer->m_Data_Length + 1 );
	 return p_Ret_String;
#endif
}
/////////////////////////////////////////////////////////////////
char* 	EString::CreateMBCS() const
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
EString::EString(const  char* p_String ) 
: m_pBuffer( EBuffer::s_p_Empty_Buffer ) 
{
	if ( p_String )
	{
		Assign( A2W( p_String ) );
	}
}
		
/////////////////////////////////////////////////////////////////
const EString& EString::operator+=( const char* p_String )
{
	if ( p_String )
	{
		*this += A2W( p_String );
	}

	return *this;
}

/////////////////////////////////////////////////////////////////
const EString& EString::operator+=( char Char )
{
	
	char Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;
	*this += A2W( Tmp );

	return *this;	
}


/////////////////////////////////////////////////////////////////
EString operator+( const EString& Begin_String, char* p_End_String )
{
	EString RetString( Begin_String );
	
	if ( p_End_String )
	{
		RetString += A2W( p_End_String );
	}

	return RetString;
}

/////////////////////////////////////////////////////////////////
EString operator+( char* p_Begin_String, const EString& End_String )
{
	EString RetString( A2W( p_Begin_String) +  End_String );

	return RetString;
}

/////////////////////////////////////////////////////////////////
EString operator+( char Char, const EString& End_String )
{
	char Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;

	EString	RetString( A2W( Tmp ) + End_String );
	
	return RetString;
}

/////////////////////////////////////////////////////////////////
bool EString::operator<( const char* p_String ) const
{
	return operator<(A2W( p_String ) );
}

/////////////////////////////////////////////////////////////////
bool operator<( const char* p_String, const EString& String )
{
	return operator<(A2W(p_String), String ); 
}
	
/////////////////////////////////////////////////////////////////
bool EString::operator>( const char* p_String ) const
{
	return EString::operator>(A2W( p_String) );
}

/////////////////////////////////////////////////////////////////
bool operator>( const char* p_String, const EString& String )
{
	return operator>( A2W(p_String), String );
}
/////////////////////////////////////////////////////////////////
bool EString::operator<=( const char* p_String) const
{
	return EString::operator<=( A2W(p_String) );
}

/////////////////////////////////////////////////////////////////
bool operator<=( const char* p_String, const EString& String)
{
	return ( A2W( p_String ) <= String );
}

/////////////////////////////////////////////////////////////////
bool EString::operator>=( const char* p_String) const
{
	return EString::operator>=( A2W(p_String) );
}
/////////////////////////////////////////////////////////////////
bool operator>=( const char* p_String, const EString& String)
{
	return ( A2W( p_String ) >= String );
}

/////////////////////////////////////////////////////////////////
void EString::Format( const char* p_Str, ... )
{
	// NOTE, we are assuming that the end user has
	// made ALL STRING parameters UNICODE
	va_list Arg_List;
	
	va_start(Arg_List, p_Str);

	va_list Arg_List_Save = Arg_List;

	// make a guess at the maximum length of the resulting string
	int Max_Len = 0;
	for (const char* p_Tmp = p_Str; *p_Tmp != '\0'; p_Tmp = _tcsinc(p_Tmp))
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
			va_arg(Arg_List, char);
			break;
		case 'c'|s_Force_Ansi:
		case 'C'|s_Force_Ansi:
			Item_Len = 2;
			va_arg(Arg_List, char);
			break;
		case 'c'|s_Force_Unicode:
		case 'C'|s_Force_Unicode:
			Item_Len = 2;
			va_arg(Arg_List, short);
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

	memcpy( m_pBuffer->Data(), A2W(pTmp), (Max_Len + 1)*sizeof(EChar) );

	m_pBuffer->m_Data_Length = StrSize( m_pBuffer->Data() );

	// clean up
	va_end( Arg_List );
	

	
}

/////////////////////////////////////////////////////////////////
bool EString::operator==( const char* p_String )
{
	return operator==( A2W( p_String ) );
}

/////////////////////////////////////////////////////////////////
bool operator==( const char* p_String, const EString& String)
{
	return operator==( A2W( p_String ), String );
}

/////////////////////////////////////////////////////////////////
bool EString::operator!=( const char* p_String) const
{
	return operator!=( A2W(p_String) );
}

/////////////////////////////////////////////////////////////////
bool operator!=( const char* p_String, const EString& String )
{
	return operator!=( A2W(p_String), String ); 
}

/////////////////////////////////////////////////////////////////
int EString::Find( char Char, int Start_Index ) const
{
	EChar Tmp; 

	mbtowc( &Tmp, &Char, 1 );
	
	return Find( Tmp, Start_Index );
}

/////////////////////////////////////////////////////////////////
const EString& EString::operator=( char Char )
{
	EChar Tmp; 

	mbtowc( &Tmp, &Char, 1 );

	return operator=( Tmp );
}



#endif // Unicode

//****************************************************************

#pragma warning( pop )
