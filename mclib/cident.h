//---------------------------------------------------------------------------
//
// cident.h - This file contains the class declarations for misc file stuff
//
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CIDENT_H
#define CIDENT_H
//---------------------------------------------------------------------------
// Include files

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef DIDENT_H
#include"dident.h"
#endif

#include<string.h>
#include "string_win.h"

//---------------------------------------------------------------------------
// Externs

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	class IDString
//---------------------------------------------------------------------------
class IDString
{
	protected:

		char id[ID_SIZE];					// 8 characters and a terminator...

	public:

		void init (void)
		{
			memset(id,0,ID_SIZE);
		}

		void init (const char *new_id)
		{
			strncpy(id,new_id,ID_SIZE-1);   // pads unused ID with 0s!
			id[ID_SIZE-1] = 0;
		}

		void init (const IDString & new_ids)
		{
			strncpy(id,new_ids.id,ID_SIZE-1);
			id[ID_SIZE-1] = 0;
		}

		IDString (void)
		{
			init();
		}

		IDString (const char * new_id)
		{
			init(new_id);
		}

		IDString (const IDString & new_ids)
		{
			init(new_ids);
		}

		inline operator char * (void)
		{
			return id;
		}

		bool operator == (const char *other_id)
		{
			if( other_id[0]!=id[0] )
				return FALSE;
			if( other_id[0]==0 )
				return TRUE;

			if( other_id[1]!=id[1] )
				return FALSE;
			if( other_id[1]==0 )
				return TRUE;

			if( other_id[2]!=id[2] )
				return FALSE;
			if( other_id[2]==0 )
				return TRUE;

			if( other_id[3]!=id[3] )
				return FALSE;
			if( other_id[3]==0 )
				return TRUE;

			if( other_id[4]!=id[4] )
				return FALSE;
			if( other_id[4]==0 )
				return TRUE;

			if( other_id[5]!=id[5] )
				return FALSE;
			if( other_id[5]==0 )
				return TRUE;

			if( other_id[6]!=id[6] )
				return FALSE;
			if( other_id[6]==0 )
				return TRUE;

			return (other_id[7]==id[7]);
//			return (strncmp(id,other_id,ID_SIZE-1) == 0);
		}

		bool operator == (const IDString &other_ids)
		{
			if( other_ids.id[0]!=id[0] )
				return FALSE;
			if( other_ids.id[0]==0 )
				return TRUE;

			if( other_ids.id[1]!=id[1] )
				return FALSE;
			if( other_ids.id[1]==0 )
				return TRUE;

			if( other_ids.id[2]!=id[2] )
				return FALSE;
			if( other_ids.id[2]==0 )
				return TRUE;

			if( other_ids.id[3]!=id[3] )
				return FALSE;
			if( other_ids.id[3]==0 )
				return TRUE;

			if( other_ids.id[4]!=id[4] )
				return FALSE;
			if( other_ids.id[4]==0 )
				return TRUE;

			if( other_ids.id[5]!=id[5] )
				return FALSE;
			if( other_ids.id[5]==0 )
				return TRUE;

			if( other_ids.id[6]!=id[6] )
				return FALSE;
			if( other_ids.id[6]==0 )
				return TRUE;

			return (other_ids.id[7]==id[7]);
//			return (strncmp(id,other_ids.id,ID_SIZE-1) == 0);
		}

		bool operator != (const char *other_id)
		{
			return !(*this == other_id);
		}

		bool operator != (const IDString &other_ids)
		{
			return !(*this == other_ids);
		}

		void operator = (const char *new_id)
		{
			init(new_id);
		}

		void operator = (const IDString & new_ids)
		{
			init(new_ids);
		}

		bool isNull (void)
		{
	      return (id[0] == 0);
		}

		void standardize (void)
		{
			strupr(id);
		}
};

//--------------------------------------------------------------------------
class FullPathFileName
{
	//Data items...
	//-------------

	private:

	protected:

		char *fullName;

	public:

	//Member functions...
	//-------------------

	private:

	protected:

	public:

		void init (void)
		{
			fullName = NULL;
		}

		void init (const char *dir_path, const char *name, const char *ext);

		FullPathFileName (void)
		{
			init();
		}

		FullPathFileName (const char *dir_path, const char *name, const char *ext)
		{
			init();
			init(dir_path, name, ext);
		}

		inline operator const char * (void) const 
		{
			return fullName;
		}

		void destroy (void);

		void changeExt (const char *from, const char *to);

		~FullPathFileName (void);
};

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
// $Log$
//
//---------------------------------------------------------------------------
