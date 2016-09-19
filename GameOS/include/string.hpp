#pragma once
//===========================================================================//
// File:	 String.hpp														 //
// Contents: String Routines												 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//



//
// Fixed length strings are used for error messages. No GOS memory is used and
// and functions that would cause the string to overrun are simply truncated.
//
// Can be initialized, appended to and passed as a char*
//
class FixedLengthString
{
	private:
//
// Data
//
	int MaximumLength;
	int CurrentSize;
	char* Text;
//
// Visible stuff
//
	public:
//
// Constructor
//
	inline FixedLengthString( int Length )
	{
		gosASSERT( Length!=0 );
		Text=(char*)malloc(Length);
		MaximumLength=Length;
		CurrentSize=0;
		*Text=0;
	}
//
// Destructor
//
	inline ~FixedLengthString()
	{
		free(Text);
	}
//
// FixedLengthString can be referenced as a char*
//
	inline operator char*() const
	{
		return Text;
	}
//
// Individual chars can be referenced
//
	inline char operator [] ( int Offset) const
	{
		if( Offset<=CurrentSize )
			return *(Text+Offset);
		else
			return 0;
	}
//
// Get Length
//
	inline int Length()
	{
		return CurrentSize;
	}
//
// Reset to null
//
	inline void Reset()
	{
		CurrentSize=0;
		*Text=0;
	}
//
// Strings can be initialized by other FixedLengthStrings
//
	inline FixedLengthString& operator = (const FixedLengthString&)
	{
		return *this;
	}
//
// Strings can be initialized by char* strings
//
	inline FixedLengthString& operator = (const char* Source)
	{
		if( Source )
		{
			int Length=strlen(Source)+1;

			if( Length>MaximumLength )
				Length=MaximumLength;

			memcpy( Text, Source, Length );

			CurrentSize=Length-1;

			*(Text+CurrentSize)=0;
		}
		return *this;
	}
//
// Strings can be appended with char* strings
//
	inline FixedLengthString& operator += (char* Source)
	{
		int Length=strlen(Source)+1;

		if( CurrentSize+Length>MaximumLength )
			Length=MaximumLength-CurrentSize;

		memcpy( Text+CurrentSize, Source, Length );

		CurrentSize+=Length-1;

		*(Text+CurrentSize)=0;

		return *this;
	}
//
// Strings can be appended with char* strings
//
	inline FixedLengthString& operator << (char* Source)
	{
		if( Source )
		{
			int Length=strlen(Source)+1;

			if( CurrentSize+Length>MaximumLength )
				Length=MaximumLength-CurrentSize;

			memcpy( Text+CurrentSize, Source, Length );

			CurrentSize+=Length-1;

			*(Text+CurrentSize)=0;
		}
		return *this;
	}
//
// Strings can be appended with integers
//
	inline FixedLengthString& operator << (int Value)
	{
		char Source[30];
		
		_itoa(Value, Source, 10);

		int Length=strlen(Source)+1;

		if( CurrentSize+Length>MaximumLength )
			Length=MaximumLength-CurrentSize;

		memcpy( Text+CurrentSize, Source, Length );

		CurrentSize+=Length-1;

		*(Text+CurrentSize)=0;

		return *this;
	}
//
// Strings can be appended with short integers
//
	inline FixedLengthString& operator << (short int Value)
	{
		char Source[30];
		
		_itoa(Value, Source, 10);

		int Length=strlen(Source)+1;

		if( CurrentSize+Length>MaximumLength )
			Length=MaximumLength-CurrentSize;

		memcpy( Text+CurrentSize, Source, Length );

		CurrentSize+=Length-1;

		*(Text+CurrentSize)=0;

		return *this;
	}
};





