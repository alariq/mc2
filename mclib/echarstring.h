#ifndef ECHARSTRING_H
#define ECHARSTRING_H

//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// Notes				essentially EString, except that it uses chars (not wchars) even when UNICODE is defined
//				
//***************************************************************

//***************************************************************

/* just for this file, we want UNICODE, K_UNICODE and MBCS undefined */
#ifdef UNICODE
#define _UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#define _PREVIOUS_UNICODE___ECHARSTRING_H	UNICODE
#undef UNICODE
#endif /* def UNICODE */

#ifdef K_UNICODE
#define _K_UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#define _PREVIOUS_K_UNICODE___ECHARSTRING_H		UNICODE
#undef K_UNICODE
#endif /* def K_UNICODE */

#ifdef MBCS
#define _MBCS_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#define _PREVIOUS_MBCS___ECHARSTRING_H		MBCS
#undef MBCS
#endif /* def MBCS */

//***************************************************************



#ifdef UNICODE
#define ECSChar wchar_t
#else
#define ECSChar char
#endif

#include<gameos.hpp>

class ECharString 
{

public:

	// ENUMERATIONS & CONSTANTS
	static const int INVALID_INDEX;

	// CONSTRUCTORS/DESTRUCTORS
	/*explicit*/ ECharString( const ECSChar* newString );
	ECharString( const ECharString& );
	/*explicit*/ ECharString( const ECSChar );
	ECharString();
	~ECharString();

	// MANIPULATORS

	void		Replace( int Start_Index, const ECharString& String );
	void		Replace( int Start_Index, const ECSChar* );

	bool		Remove( int Start_Index, int End_Index );
	bool Remove( ECharString& Sub_String );

	// puts this string in the middle of another string
	inline void	Insert( int Start_Index, const ECharString& String );
	void	Insert( int Start_Index, const ECSChar* String ); 

	void		Swap( ECharString& );
	inline void	Empty();
	void		MakeUpper();
	void 		MakeLower();
	void 		Reverse();

	// works like sprintf with the destination buffer being
	// the this pointer
	void	Format( const ECSChar*, ... );

	inline const ECharString& operator=( const ECSChar* );
	inline const ECharString& operator=( const ECharString& );
	inline const ECharString& operator=( ECSChar );

	inline const ECharString& operator+=( const ECSChar* );
	inline const ECharString& operator+=( const ECharString& );
	inline const ECharString& operator+=( ECSChar );

	friend ECharString operator+( const ECharString&, const ECharString& );

	friend ECharString operator+( const ECSChar*, const ECharString& );
	friend ECharString operator+( const ECharString& , const ECSChar* );

	friend ECharString operator+( const ECharString&,  const  ECSChar );
	friend ECharString operator+( const ECSChar,  const  ECharString& );

 	// ACCESSORS

	// these are case sensitive, use Compare
	// if you don't want case senstitivity
	inline bool operator==( const ECharString& ) const;
	inline bool operator==( const ECSChar* ) const;
	inline friend bool operator==( const ECSChar*, const ECharString& );

	// these functions return -1 if "this" is less than the passed in string
	int Compare( const ECharString&, bool Case_Sensitive = false ) const;
	int Compare( const ECSChar*, bool Case_Sensitive = false ) const;

	bool operator!=( const ECharString& ) const;
	bool operator!=( const ECSChar*) const;
	friend bool operator!=( const ECSChar*, const ECharString& );

	inline ECSChar& operator[]( int Index );
	inline const ECSChar& operator[](int Index) const;

	inline bool operator<( const ECharString& ) const; 
	inline bool operator<( const ECSChar* ) const;
	inline friend bool operator<( const ECSChar*, const ECharString& );

	inline bool operator<=( const ECharString& ) const;
	inline bool operator<=( const ECSChar*) const;
	inline friend bool operator<=( const ECSChar*, const ECharString&);

	inline bool operator>( const ECharString& ) const;
	inline bool operator>(const ECSChar*) const;
	inline friend bool operator>( const ECSChar*, const ECharString&);

	inline bool operator>=( const ECharString& ) const;
	inline bool operator>=( const ECSChar*) const;
	inline friend bool operator>=( const ECSChar*, const ECharString&);
	
	int Size() const; 	// number of bytes
	int Length() const;	// number of characters

	// search functions
	int Find( ECSChar, int Start_Index = ECharString::INVALID_INDEX) const;
	int Find( const ECharString&, int Start_Index = ECharString::INVALID_INDEX) const;
	int Find( const ECSChar*, int Start_Index = ECharString::INVALID_INDEX) const;

	int ReverseFind ( ECSChar, int End_Index = ECharString::INVALID_INDEX) const;

	
	// we are going to treat this object as a TCHAR array, so we 
	// don't have to worry about #of chars versus #of bytes
	ECharString SubString( int Start_Index, int End_Index ) const;
	inline ECharString Left( int Num_Chars) const;	 
	inline ECharString Right( int Num_Chars) const;	

	inline bool 		IsEmpty() const;

	wchar_t*	CreateUNICODE() const;
	char* 	CreateMBCS() const; 

	inline	const ECSChar* Data() const;

	// ALL UNICODE SPECIFIC AND

#ifndef UNICODE

	int Find( unsigned short, int Start_Index = -1 ) const;

#else // K_UNICODE

	// we'll convert string literals for you 
	
	ECharString( const char* );
			
	friend ECharString operator+( const ECharString&, char* );
	friend ECharString operator+( char*, const ECharString& );
	
	friend ECharString operator+( char, const ECharString& );
	friend ECharString operator+( const ECharString&, char );
	
	const ECharString& operator+=( const char* );
	const ECharString& operator+=( char );

	const ECharString& operator=( char );

	bool operator==( const char* );
	friend bool operator==( const char*, const ECharString& );

	bool operator!=( const char*) const;
	friend bool operator!=( const char*, const ECharString& );

	bool operator<( const char* ) const;
	friend bool operator<( const char*, const ECharString& );
	
	bool operator>( const char* ) const;
	friend bool operator>( const char*, const ECharString& );

	bool operator<=( const char*) const;
	friend bool operator<=( const char*, const ECharString&);

	bool operator>=( const char*) const;
	friend bool operator>=( const char*, const ECharString&);

	void	Format( const char*, ... );

	int Find( char, int Start_Index = ECharString::INVALID_INDEX) const;


#endif // Unicode 

private:


	// helper functions

	// Allocates a specific amount
	void	Alloc( int Min_Amount );
	
	// Reallocates if you want to make a change to a shared buffer
	inline	void	ChecEBuffer();
	void	ChecEBufferDoRealloc();

	// sets the buffer, reallocs if necessary
	void	Assign( const ECSChar* p_Str );


	static  inline  wchar_t*	ToUnicode( wchar_t* Buffer, const unsigned char* p_Str, int Num_Chars  );
	static	inline	int	StrSize( const ECSChar* p_Str );
	
	struct EBuffer
	{
		int m_Ref_Count;		// reference count
		int m_Data_Length;		// Length of String
		int m_Alloc_Length;	// Length of the Buffer
	
		inline ECSChar* Data();
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
inline void	ECharString::ChecEBuffer()
{
	if ( m_pBuffer->m_Ref_Count > 0 )
	{
		m_pBuffer->m_Ref_Count --;
		m_pBuffer = EBuffer::s_p_Empty_Buffer;
	}
}

/////////////////////////////////////////////////////////////////
inline void ECharString::EBuffer::Release()
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
inline void	ECharString::Empty()
{
	m_pBuffer->Release();
	m_pBuffer =  EBuffer::s_p_Empty_Buffer;
}


/////////////////////////////////////////////////////////////////
inline void ECharString::Insert( int Start_Index, const ECharString& String )
{
	Insert( Start_Index, String.m_pBuffer->Data() );
}

/////////////////////////////////////////////////////////////////
inline const ECharString& ECharString::operator=( const ECSChar* p_String )
{
	ChecEBuffer();
	Assign( p_String );
	return *this;
}

/////////////////////////////////////////////////////////////////
inline const ECharString& ECharString::operator=( const ECharString& Src )
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
inline const ECharString& ECharString::operator=( ECSChar Char )
{
	ChecEBuffer();

	ECSChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;
	Assign( Tmp );

	return *this;
}

/////////////////////////////////////////////////////////////////
inline const ECharString& ECharString::operator+=( const ECSChar* p_String )
{
	Insert( m_pBuffer->m_Data_Length, p_String );
	return *this;
}

/////////////////////////////////////////////////////////////////
inline const ECharString& ECharString::operator+=( const ECharString& String )
{
	Insert( m_pBuffer->m_Data_Length, String );
	return *this;
}

/////////////////////////////////////////////////////////////////
inline const ECharString& ECharString::operator+=( ECSChar Char )
{
	ECSChar Tmp[2];
	Tmp[0] = Char;
	Tmp[1] = 0;
	Insert(  m_pBuffer->m_Data_Length, Tmp );

	return *this;	
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator!=( const ECharString& Str_To_Compare ) const
{
	return !( operator==(Str_To_Compare) ); 
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator!=( const ECSChar* p_Str_To_Compare ) const
{
	return !( operator==( p_Str_To_Compare ) );
}

/////////////////////////////////////////////////////////////////
inline ECharString ECharString::Left( int Num_Chars) const
{
	// Bill changed to Num_Chars - 1, this was always returning one character too many
	return SubString( 0, Num_Chars - 1 ); 
}

/////////////////////////////////////////////////////////////////
inline ECharString ECharString::Right( int Num_Chars) const
{
	return SubString( m_pBuffer->m_Data_Length - Num_Chars, 
		m_pBuffer->m_Data_Length - 1 );
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::IsEmpty() const
{
	return (m_pBuffer->m_Data_Length <= 0);
}

/////////////////////////////////////////////////////////////////
inline	const ECSChar* ECharString::Data() const
{
	return ( m_pBuffer->Data() );
}

/////////////////////////////////////////////////////////////////
inline ECSChar* ECharString::EBuffer::Data()
{
	if ( !m_Alloc_Length )
	{
		return 0;
	}
	return (ECSChar*)(this + 1);
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator==( const ECharString& Str_To_Compare) const
{
	return ( 0 == Compare( Str_To_Compare, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator==( const ECSChar* p_String ) const
{

	return ( 0 == Compare( p_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator==( const ECSChar* p_String, const ECharString& Str )
{
	return ( 0 == Str.Compare( p_String ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator!=( const ECSChar* p_String, const ECharString& Str )
{
	return !(Str == p_String );
}
/////////////////////////////////////////////////////////////////
inline ECSChar& ECharString::operator[]( int Index )
{
	ChecEBufferDoRealloc();

	gosASSERT( Index < m_pBuffer->m_Data_Length );

	return *(m_pBuffer->Data() + Index);
}

/////////////////////////////////////////////////////////////////
inline const ECSChar& ECharString::operator[](int Index) const
{
	gosASSERT( Index < m_pBuffer->m_Data_Length );
	
	return *(m_pBuffer->Data() + Index);
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator<( const ECharString& Greater_String ) const
{
	return ( 0 > Compare( Greater_String, true));
	
}

/////////////////////////////////////////////////////////////////
inline bool operator<( const ECSChar* p_Lesser_String, 
						const ECharString& Greater_String )
{
	return ( 0 < Greater_String.Compare( p_Lesser_String, true ) ); 
}
	
/////////////////////////////////////////////////////////////////
inline bool ECharString::operator<( const ECSChar* p_Greater_String ) const
{
	return ( 0 > Compare( p_Greater_String, true ) ); 
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator<=( const ECharString& Greater_String ) const
{
	return ( 1 > Compare( Greater_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator<=( const ECSChar* p_Lesser_String, 
						const ECharString& Greater_String )
{
	return ( 0 < Greater_String.Compare( p_Lesser_String, true ) ); 
					 
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator<=( const ECSChar* p_Greater_String ) const
{
	return ( 1 > Compare( p_Greater_String, true ) ); 
}


/////////////////////////////////////////////////////////////////
inline bool ECharString::operator>( const ECharString& Lesser_String ) const
{
	return ( 0 < Compare( Lesser_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator>( const ECSChar* p_Greater_String, 
					    const ECharString& Lesser_String )
{
	return ( 0 > Lesser_String.Compare( p_Greater_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator>( const ECSChar* p_Lesser_String ) const
{
	return ( 0 < Compare( p_Lesser_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator>=( const ECharString& Lesser_String ) const
{
	return ( -1 < Compare( Lesser_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool operator>=( const ECSChar* p_Greater_String, 
					    const ECharString& Lesser_String )
{
	return ( 1 > Lesser_String.Compare( p_Greater_String, true ) );
}

/////////////////////////////////////////////////////////////////
inline bool ECharString::operator>=( const ECSChar* p_Lesser_String ) const
{
	return ( -1 < Compare( p_Lesser_String, true ) );
}



//***************************************************************

/* restore UNICODE, K_UNICODE and MBCS to their previous state */
#ifdef _UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#undef _UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#define UNICODE		_PREVIOUS_UNICODE___ECHARSTRING_H
#undef _PREVIOUS_UNICODE___ECHARSTRING_H
#endif /* def _UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H */

#ifdef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#undef _K_UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#define K_UNICODE	_PREVIOUS_K_UNICODE___ECHARSTRING_H
#undef _PREVIOUS_K_UNICODE___ECHARSTRING_H
#endif /* def _K_UNICODE_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H */

#ifdef _MBCS_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#undef _MBCS_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H
#define MBCS	_PREVIOUS_MBCS___ECHARSTRING_H
#undef _PREVIOUS_MBCS___ECHARSTRING_H
#endif /* def _MBCS_WAS_PREVIOUSLY_DEFINED___ECHARSTRING_H */

//***************************************************************


#endif //ECharString_H_

