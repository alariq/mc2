//===========================================================================//
// File:	iterator.hh                                                      //
// Contents: Interface specifications for the abstract iterator              //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Iterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	typedef int CollectionSize;
    // sebi
	typedef int IteratorPosition;
#ifndef LINUX_BUILD
	class _declspec(novtable) Iterator
#else
	class Iterator
#endif
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Public interface to abstract class
		//
		// NOTE: All unsafe, untyped public methods are named XXXItem.  The
		// safe, typed public methods are named XXX and are declared in the
		// template sub-classes.
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//

		//
		//--------------------------------------------------------------------
		// Destructor and testing
		//--------------------------------------------------------------------
		//
		virtual
			~Iterator()
				{}
		virtual Iterator*
			MakeClone() = 0;
		void
			TestInstance()
				{}

		//
		//--------------------------------------------------------------------
		// First						- Moves to next item
		// Last						- Moves to last item
		//--------------------------------------------------------------------
		//
		virtual void
			First();
		virtual void
			Last();

		//
		//--------------------------------------------------------------------
		// Next			 			- Moves to next item
		// Previous			 		- Moves to previous item
		//--------------------------------------------------------------------
		//
		virtual void
			Next();
		virtual void
			Previous();

		//
		//--------------------------------------------------------------------
		// ReadAndNextItem		- Returns current item and moves to next item
		// ReadAndPreviousItem	- Returns current item and moves to prev item
		// GetCurrentItem			- Returns current item
		//--------------------------------------------------------------------
		//
		void*
			ReadAndNextItem()
				{return ReadAndNextImplementation();}
		void*
			ReadAndPreviousItem()
				{return ReadAndPreviousImplementation();}
		void*
			GetCurrentItem()
				{return GetCurrentImplementation();}

		//
		//--------------------------------------------------------------------
		// GetSize					- Returns number of items
		// GetNthItem				- Returns nth item
		//--------------------------------------------------------------------
		//
		virtual CollectionSize
			GetSize();
		void*
			GetNthItem(CollectionSize index)
				{return GetNthImplementation(index);}

		//
		//--------------------------------------------------------------------
		// BeginIterator			- Moves to first item and returns iterator
		// EndIterator				- Moves to last item and returns iterator
		//--------------------------------------------------------------------
		//
		virtual Iterator&
			BeginIterator()
				{return BeginImplementation();}
		virtual Iterator&
			EndIterator()
				{return EndImplementation();}

		//
		//--------------------------------------------------------------------
		// ForwardIterator	  	- Moves to next item and returns iterator
		// BackwardIterator	  	- Moves to previous item and returns iterator
		//--------------------------------------------------------------------
		//
		virtual Iterator&
			ForwardIterator()
				{return ForwardImplementation();}
		virtual Iterator&
			BackwardIterator()
				{return BackwardImplementation();}

	protected:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Protected interface to abstract class
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		Iterator()
			{}

		//
		//--------------------------------------------------------------------
		// Untyped implementations
		//--------------------------------------------------------------------
		//
		virtual void*
			ReadAndNextImplementation();
		virtual void*
			ReadAndPreviousImplementation();
		virtual void*
			GetCurrentImplementation();
		virtual void*
			GetNthImplementation(CollectionSize index);

		virtual Iterator&
			BeginImplementation()
				{First(); return *this;}
		virtual Iterator&
			EndImplementation()
				{Last(); return *this;}
		virtual Iterator&
			ForwardImplementation()
				{Next(); return *this;}
		virtual Iterator&
			BackwardImplementation()
				{Previous(); return *this;}
	};

}
