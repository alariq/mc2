//===========================================================================//
// File:     namelist.hpp                                                    //
// Title:    Declaration of NameList classes.                                //
// Purpose:  Maintains an unsorted list of strings with (void *) to          //
//           anything the client needs to associate with the string.         //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"scalar.hpp"

namespace Stuff {class NameList;}

namespace MemoryStreamIO
{
	Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream *stream,
			Stuff::NameList *names
		);
	Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream *stream,
			const Stuff::NameList *names
		);
}

namespace Stuff {

	class ObjectNameList;
	class ObjectNameList__Entry;
	class NameList;
	class AlphaNameList;

	//=======================================================================
	// Notice:
	//		In using ObjectNameList class, think of AddEntry sort of like a New
	// in the sense that you MUST store the (char *) it returns in the name
	// field of the class you are bonding ObjectNameList to so that when you
	// DeleteEntry you can pass that exact same pointer back.  DeleteEntry
	// does not do a find on the name you pass it.  It expects to receive the
	// pointer that AddEntry gave to you.
	//    This restriction does not exist for NameList and AlphaNameList.
	//-----------------------------------------------------------------------
	// Another Notice:
	//		NameList::Entry objects are never _constructed_ and therefore
	// Check_Pointer(this) is used instead of Check_Object(this).  Also Register_
	// Pointer and Unregister_Pointer are used instead of Register_Object
	// and Unregister_Object.
	//=======================================================================

	//##########################################################################
	//##############    ObjectNameList    ######################################
	//##########################################################################

	class ObjectNameList
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		typedef ObjectNameList__Entry    Entry;

	protected:
		Entry
			*firstEntry,
			*lastEntry;

	public:
		ObjectNameList();
		virtual
			~ObjectNameList();

		virtual const char*
			AddEntry(
				const char *name,
				void *data
			);
		void*
			FindObject(const char *name);
		void
			DeleteEntry(const char *name);	// ** DANGEROUS!! see notice above **
		int
			GetEntryCount() const;	// (implementation assumes infrequent use)
		bool
			IsEmpty() const
				{ Check_Object(this); return firstEntry == NULL && lastEntry == NULL; }
		Entry*
			GetFirstEntry()
				{ Check_Object(this); return firstEntry; }
		const Entry*
			GetFirstEntry() const
				{ Check_Object(this); return firstEntry; }
		Entry*
			GetLastEntry()
				{ Check_Object(this); return lastEntry; }
		const Entry*
			GetLastEntry() const
				{ Check_Object(this); return lastEntry; }
		int
			BuildSubList(
				const ObjectNameList &source_list,
				const char *prefix
			);

		void
			TestInstance() const
				{}
		static bool
			TestClass();
	};

	//##########################################################################
	//##############    ObjectNameList::Entry    ###############################
	//##########################################################################

	class ObjectNameList__Entry
	{
		friend class ObjectNameList;
		friend class NameList;
		friend class AlphaNameList;

	private:
		ObjectNameList::Entry
			*nextEntry;

	public:
		void
			*dataReference;

	protected:
		void
			SetName(const char *name);
	
	public:
		const char*
			GetName() const
				{
					Check_Object(this);
					return &(
						Cast_Pointer(const char*, this)[
							sizeof(ObjectNameList::Entry)
						]
					);
				}
		bool
			IsName(const char *name) const;
		void*
			GetObject()
				{ Check_Object(this); return dataReference; }
		void*
			GetData()
				{ Check_Object(this); return dataReference; }
		const void*
			GetData() const
				{ Check_Object(this); return dataReference; }
		char*
			GetChar()
				{Check_Object(this); return Cast_Pointer(char *,dataReference);}
		const char*
			GetChar() const
				{
					Check_Object(this);
					return Cast_Pointer(const char *,dataReference);
				}
		int
			GetAtoi() const
				{
					Check_Object(this); Check_Pointer(dataReference);
					return atoi(Cast_Pointer(const char *, dataReference));
				}
		long
			GetAtol() const
				{
					Check_Object(this); Check_Pointer(dataReference);
					return atol(Cast_Pointer(const char *, dataReference));
				}
		Scalar
			GetAtof() const
				{
					Check_Object(this); Check_Pointer(dataReference);
					return AtoF(Cast_Pointer(const char *, dataReference));
				}
		ObjectNameList::Entry*
			GetNextEntry()
				{ Check_Object(this); return nextEntry; }
		const ObjectNameList::Entry*
			GetNextEntry() const
				{ Check_Object(this); return nextEntry; }
		void
			TestInstance() const
				{}
	};

	//##########################################################################
	//##############    NameList    ############################################
	//##########################################################################

	class NameList:
		public ObjectNameList
	{
	public:
		NameList();
		~NameList();

		void*
			FindData(const char *name)
				{ return FindObject(name); }
		const char*
			FindName(void *data);
		Entry*
			FindEntry(const char *name);
		int
			FindEntryIndex(const char *name);
		Entry*
			FindEntry(void *data);
		void
			DeleteEntry(const char *name);	// this one is searches for name
		static bool
			TestClass();

		friend MemoryStream&
			MemoryStreamIO::Read(
				MemoryStream *stream,
				NameList *names
			);
		friend MemoryStream&
			MemoryStreamIO::Write(
				MemoryStream *stream,
				const NameList *names
			);
	};

	//##########################################################################
	//##############    AlphaNameList    #######################################
	//##########################################################################

	class AlphaNameList:
		public NameList
	{
	public:
		AlphaNameList();
		~AlphaNameList();

		const char*
			AddEntry(
				const char *name,
				void *data
			);
		static bool
			TestClass();
	};

}
