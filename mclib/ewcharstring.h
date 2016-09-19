#ifndef EWCHARSTRING_H
#define EWCHARSTRING_H

//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// Notes				essentially EString, except that it uses wchars (not chars) even when UNICODE is not defined
//				
//***************************************************************

//***************************************************************

/* just for this file, we want UNICODE and K_UNICODE defined and MBCS undefined */
#ifdef UNICODE
#define _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#else
#define UNICODE
#endif /* def UNICODE */

#ifdef K_UNICODE
#define _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#else
#define K_UNICODE
#endif /* def K_UNICODE */

#ifdef MBCS
#define _MBCS_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#define _PREVIOUS_MBCS___EWCHARSTRING_H		MBCS
#undef MBCS
#endif /* def MBCS */

//***************************************************************



#ifdef UNICODE
#define EWCSChar wchar_t
#else
#define EWCSChar char
#endif

#include<gameos.hpp>

class EWCharString 
{

public:

	// ENUMERATIONS & CONSTANTS
	static const int INVALID_INDEX;

	// CONSTRUCTORS/DESTRUCTORS
	/*explicit*/ EWCharString( const EWCSChar* newString );
	EWCharString( const EWCharString& );
	/*explicit*/ EWCharString( const EWCSChar );
	EWCharString();
	~EWCharString();

	// MANIPULATORS

	void		Replace( int Start_Index, const EWCharString& String );
	void		Replace( int Start_Index, const EWCSChar* );

	bool		Remove( int Start_Index, int End_Index );
	bool Remove( EWCharString& Sub_String );

	// puts this string in the middle of another string
	inline void	Insert( int Start_Index, const EWCharString& String );
	void	Insert( int Start_Index, const EWCSChar* String ); 

	void		Swap( EWCharString& );
	inline void	Empty();
	void		MakeUpper();
	void 		MakeLower();
	void 		Reverse();

	// works like sprintf with the destination buffer being
	// the this pointer
	void	Format( const EWCSChar*, ... );

	inline const EWCharString& operator=( const EWCSChar* );
	inline const EWCharString& operator=( const EWCharString& );
	inline const EWCharString& operator=( EWCSChar );

	inline const EWCharString& operator+=( const EWCSChar* );
	inline const EWCharString& operator+=( const EWCharString& );
	inline const EWCharString& operator+=( EWCSChar );

	friend EWCharString operator+( const EWCharString&, const EWCharString& );

	friend EWCharString operator+( const EWCSChar*, const EWCharString& );
	friend EWCharString operator+( const EWCharString& , const EWCSChar* );

	friend EWCharString operator+( const EWCharString&,  const  EWCSChar );
	friend EWCharString operator+( const EWCSChar,  const  EWCharString& );

 	// ACCESSORS

	// these are case sensitive, use Compare
	// if you don't want case senstitivity
	inline bool operator==( const EWCharString& ) const;
	inline bool operator==( const EWCSChar* ) const;
	inline friend bool operator==( const EWCSChar*, const EWCharString& );

	// these functions return -1 if "this" is less than the passed in string
	int Compare( const EWCharString&, bool Case_Sensitive = false ) const;
	int Compare( const EWCSChar*, bool Case_Sensitive = false ) const;

	bool operator!=( const EWCharString& ) const;
	bool operator!=( const EWCSChar*) const;
	friend bool operator!=( const EWCSChar*, const EWCharString& );

	inline EWCSChar& operator[]( int Index );
	inline const EWCSChar& operator[](int Index) const;

	inline bool operator<( const EWCharString& ) const; 
	inline bool operator<( const EWCSChar* ) const;
	inline friend bool operator<( const EWCSChar*, const EWCharString& );

	inline bool operator<=( const EWCharString& ) const;
	inline bool operator<=( const EWCSChar*) const;
	inline friend bool operator<=( const EWCSChar*, const EWCharString&);

	inline bool operator>( const EWCharString& ) const;
	inline bool operator>(const EWCSChar*) const;
	inline friend bool operator>( const EWCSChar*, const EWCharString&);

	inline bool operator>=( const EWCharString& ) const;
	inline bool operator>=( const EWCSChar*) const;
	inline friend bool operator>=( const EWCSChar*, const EWCharString&);
	
	int Size() const; 	// number of bytes
	int Length() const;	// number of characters

	// search functions
	int Find( EWCSChar, int Start_Index = EWCharString::INVALID_INDEX) const;
	int Find( const EWCharString&, int Start_Index = EWCharString::INVALID_INDEX) const;
	int Find( const EWCSChar*, int Start_Index = EWCharString::INVALID_INDEX) const;

	int ReverseFind ( EWCSChar, int End_Index = EWCharString::INVALID_INDEX) const;

	
	// we are going to treat this object as a TCHAR array, so we 
	// don't have to worry about #of chars versus #of bytes
	EWCharString SubString( int Start_Index, int End_Index ) const;
	inline EWCharString Left( int Num_Chars) const;	 
	inline EWCharString Right( int Num_Chars) const;	

	inline bool 		IsEmpty() const;

	wchar_t*	CreateUNICODE() const;
	char* 	CreateMBCS() const; 

	inline	const EWCSChar* Data() const;

	// ALL UNICODE SPECIFIC AND

#ifndef UNICODE

	int Find( unsigned short, int Start_Index = -1 ) const;

#else // K_UNICODE

	// we'll convert string literals for you 
	
	EWCharString( const char* );
			
	friend EWCharString operator+( const EWCharString&, char* );
	friend EWCharString operator+( char*, const EWCharString& );
	
	friend EWCharString operator+( char, const EWCharString& );
	friend EWCharString operator+( const EWCharString&, char );
	
	const EWCharString& operator+=( const char* );
	const EWCharString& operator+=( char );

	const EWCharString& operator=( char );

	bool operator==( const char* );
	friend bool operator==( const char*, const EWCharString& );

	bool operator!=( const char*) const;
	friend bool operator!=( const char*, const EWCharString& );

	bool operator<( const char* ) const;
	friend bool operator<( const char*, const EWCharString& );
	
	bool operator>( const char* ) const;
	friend bool operator>( const char*, const EWCharString& );

	bool operator<=( const char*) const;
	friend bool operator<=( const char*, const EWCharString&);

	bool operator>=( const char*) const;
	friend bool operator>=( const char*, const EWCharString&);

	void	Format( const char*, ... );

	int Find( char, int Start_Index = EWCharString::INVALID_INDEX) const;


#endif // Unicode 

private:


	// helper functions

	// Allocates a specific amount
	void	Alloc( int Min_Amount );
	
	// Reallocates if you want to make a change to a shared buffer
	inline	void	ChecEBuffer();
	void	ChecEBufferDoRealloc();

	// sets the buffer, reallocs if necessary
	void	Assign( const EWCSChar* p_Str );


	static  inline  wchar_t*	ToUnicode( wchar_t* Buffer, const unsigned char* p_Str, int Num_Chars  );
	static	inline	int	StrSize( const EWCSChar* p_Str );
	
	struct EBuffer
	{
		int m_Ref_Count;		// reference count
		int m_Data_Length;		// Length of String
		int m_Alloc_Length;	// Length of the Buffer
	
		inline EWCSChar* Data();
		inline void Release();

		static EBuffer	s_Empty_Buffer;
		static EBuffer* s_p_Empty_Buffer;
	};

	EBuffer*		m_pBuffer;
	static const int s_Alloc_Allign;
	static const int s_Force_Ansi;
	static const int s_Force_Unicode;

};

//***************************************************************

// inlines



/////////////////////////////////////////////////////////////////
inline void	EWCharString::ChecEBuffer()
{
	if ( m_pBuffer->m_Ref_Count > 0 )
	{
		m_pBuffer->m_Ref_Count --;
		m_pBuffer = EBuffer::s_p_Empty_Buffer;
	}
}

/////////////////////////////////////////////////////////////////
inline void EWCharString::EBuffer::Release()
{
	if ( this != s_p_Empty_Buffer )
	{
		m_Ref_Count --;
		if ( m_Ref_Count < 0 )
		{
			delete [] (char*)this;
		}
	}
}

/////////////////////////////////////////////////////////////////
inline void	EWCharString::Empty()
{
	m_pBuffer->Release();
	m_pBuffer =  EBuffer::s_p_Empty_Buffer;
}


/////////////////////////////////////////////////////////////////
inline void EWCharString::Insert( int Start_Index, const EWCharString& String )
{
	Insert( Start_Index, String.m_pBuffer->Data() );
}

/////////////////////////////////////////////////////////////////
inline const EWCharString& EWCharString::operator=( const EWCSChar* p_String )
{
	ChecEBuffer();
	Assign( p_String );
	return *this;
}

/////////////////////////////////////////////////////////////////
inline const EWCharString& EWCharString::operator=( const EWCharString& Src )
{
	if ( &Src != this )
	{
		m_pBuffer->Release();
		m_pBuffer = Src.m_pBuffer;
		Src.m_pBuffer->m_Ref_Count ++;
	}

	return *this; 
}

/////////////////////////////////////////////////////////////////
inline const EWCharString& EWCharString::operator=( EWCSChar Char )
{
	ChecEBuffer();

	EWCSChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;
	Assign( Tmp );

	return *this;
}

/////////////////////////////////////////////////////////////////
inline const EWCharString& EWCharString::operator+=( const EWCSChar* p_String )
{
	Insert( m_pBuffer->m_Data_Length, p_String );
	return *this;
}

/////////////////////////////////////////////////////////////////
inline const EWCharString& EWCharString::operator+=( const EWCharString& String )
{
	Insert( m_pBuffer->m_Data_Length, String );
	return *this;
}

/////////////////////////////////////////////////////////////////
inline const EWCharString& EWCharString::operator+=( EWCSChar Char )
{
	EWCSChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;
	Insert(  m_pBuffer->m_Data_Length, Tmp );

	return *this;	
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator!=( const EWCharString& Str_To_Compare ) const
{
	return !( operator==(Str_To_Compare) ); 
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator!=( const EWCSChar* p_Str_To_Compare ) const
{
	return !( operator==( p_Str_To_Compare ) );
}

/////////////////////////////////////////////////////////////////
inline EWCharString EWCharString::Left( int Num_Chars) const
{
	// Bill changed to Num_Chars - 1, this was always returning one character too many
	return SubString( 0, Num_Chars - 1 ); 
}

/////////////////////////////////////////////////////////////////
inline EWCharString EWCharString::Right( int Num_Chars) const
{
	return SubString( m_pBuffer->m_Data_Length - Num_Chars, 
		m_pBuffer->m_Data_Length - 1 );
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::IsEmpty() const
{
	return (m_pBuffer->m_Data_Length <= 0);
}

/////////////////////////////////////////////////////////////////
inline	const EWCSChar* EWCharString::Data() const
{
	return ( m_pBuffer->Data() );
}

/////////////////////////////////////////////////////////////////
inline EWCSChar* EWCharString::EBuffer::Data()
{
	if ( !m_Alloc_Length )
	{
		return 0;
	}
	return (EWCSChar*)(this + 1);
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator==( const EWCharString& Str_To_Compare) const
{
	return ( 0 == Compare( Str_To_Compare, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator==( const EWCSChar* p_String ) const
{

	return ( 0 == Compare( p_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator==( const EWCSChar* p_String, const EWCharString& Str )
{
	return ( 0 == Str.Compare( p_String ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator!=( const EWCSChar* p_String, const EWCharString& Str )
{
	return !(Str == p_String );
}
/////////////////////////////////////////////////////////////////
inline EWCSChar& EWCharString::operator[]( int Index )
{
	ChecEBufferDoRealloc();

	gosASSERT( Index < m_pBuffer->m_Data_Length );

	return *(m_pBuffer->Data() + Index);
}

/////////////////////////////////////////////////////////////////
inline const EWCSChar& EWCharString::operator[](int Index) const
{
	gosASSERT( Index < m_pBuffer->m_Data_Length );
	
	return *(m_pBuffer->Data() + Index);
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator<( const EWCharString& Greater_String ) const
{
	return ( 0 > Compare( Greater_String, true));
	
}

/////////////////////////////////////////////////////////////////
inline bool operator<( const EWCSChar* p_Lesser_String, 
						const EWCharString& Greater_String )
{
	return ( 0 < Greater_String.Compare( p_Lesser_String, true ) ); 
}
	
/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator<( const EWCSChar* p_Greater_String ) const
{
	return ( 0 > Compare( p_Greater_String, true ) ); 
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator<=( const EWCharString& Greater_String ) const
{
	return ( 1 > Compare( Greater_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator<=( const EWCSChar* p_Lesser_String, 
						const EWCharString& Greater_String )
{
	return ( 0 < Greater_String.Compare( p_Lesser_String, true ) ); 
					 
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator<=( const EWCSChar* p_Greater_String ) const
{
	return ( 1 > Compare( p_Greater_String, true ) ); 
}


/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator>( const EWCharString& Lesser_String ) const
{
	return ( 0 < Compare( Lesser_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator>( const EWCSChar* p_Greater_String, 
					    const EWCharString& Lesser_String )
{
	return ( 0 > Lesser_String.Compare( p_Greater_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator>( const EWCSChar* p_Lesser_String ) const
{
	return ( 0 < Compare( p_Lesser_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator>=( const EWCharString& Lesser_String ) const
{
	return ( -1 < Compare( Lesser_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator>=( const EWCSChar* p_Greater_String, 
					    const EWCharString& Lesser_String )
{
	return ( 1 > Lesser_String.Compare( p_Greater_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool EWCharString::operator>=( const EWCSChar* p_Lesser_String ) const
{
	return ( -1 < Compare( p_Lesser_String, true ) );
}



//***************************************************************

/* restore UNICODE, K_UNICODE and MBCS to their previous state */
#ifndef _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#undef UNICODE
#else
#undef _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#endif /* ndef _UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H */

#ifndef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#undef K_UNICODE
#else
#undef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#endif /* ndef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H */

#ifdef _MBCS_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#undef _MBCS_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H
#define MBCS	_PREVIOUS_MBCS___EWCHARSTRING_H
#undef _PREVIOUS_MBCS___EWCHARSTRING_H
#endif /* def _MBCS_WAS_PREVIOUSLY_DEFINED___EWCHARSTRING_H */

//***************************************************************


#endif //EWCharString_H_

