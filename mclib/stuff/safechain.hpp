//===========================================================================//
// File:	schain.hh                                                        //
// Contents: Interface definition for safe chains                            //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"safesocket.hpp"
#include"memoryblock.hpp"

namespace Stuff {
    
    class SafeChain;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeChainLink ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	enum {
		SafeChainLink_MemoryBlock_Allocation = 100
	};

	class SafeChainLink:
		public Link
	{
	public:
		friend class SafeChain;
		friend class SafeChainIterator;

		static void
			InitializeClass(
				size_t block_count = SafeChainLink_MemoryBlock_Allocation,
				size_t block_delta = SafeChainLink_MemoryBlock_Allocation
			);
		static void
			TerminateClass();

	public:
		~SafeChainLink();
		void
			TestInstance();

	private:
		SafeChainLink(
			SafeChain *chain,
			Plug *plug,
			SafeChainLink *nextSafeChainLink,
			SafeChainLink *prevSafeChainLink
		);

		SafeChainLink *nextSafeChainLink;
		SafeChainLink *prevSafeChainLink;

	private:
		static MemoryBlock
			*AllocatedMemory;

		void*
			operator new(size_t)
				{return AllocatedMemory->New();}
		void
			operator delete(void *where)
				{AllocatedMemory->Delete(where);}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeChain ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SafeChain:
		public SafeSocket
	{
		friend class SafeChainLink;
		friend class SafeChainIterator;

	public:
 		//
		//-----------------------------------------------------------------------
		//-----------------------------------------------------------------------
		// Public interface
		//-----------------------------------------------------------------------
		//-----------------------------------------------------------------------
		//
		explicit SafeChain(Node *node);
		~SafeChain();

		void
			TestInstance();
		static void
			TestClass();
		static void
			ProfileClass();

		//
		//-----------------------------------------------------------------------
		// IsEmpty - Returns true if the socket contains no plugs.
		//-----------------------------------------------------------------------
		//
		bool
			IsEmpty();

	protected:
		//
		//-----------------------------------------------------------------------
		//-----------------------------------------------------------------------
		// Protected interface
		//-----------------------------------------------------------------------
		//-----------------------------------------------------------------------
		//
		void
			AddImplementation(Plug *plug);

	private:
		//
		//-----------------------------------------------------------------------
		// Private utilities
		//-----------------------------------------------------------------------
		//
		SafeChainLink*
			InsertSafeChainLink(
				Plug *plug,
				SafeChainLink *link
			);

		//
		//-----------------------------------------------------------------------
		// Private data
		//-----------------------------------------------------------------------
		//
		SafeChainLink *head;
		SafeChainLink *tail;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeChainOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class SafeChainOf:
		public SafeChain
	{
	public:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Public interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		explicit SafeChainOf(Node *node);
		~SafeChainOf();

		//
		//--------------------------------------------------------------------
		// Socket methods (see Socket for full listing)
		//--------------------------------------------------------------------
		//
		void
			Add(T plug)
				{AddImplementation(Cast_Pointer(Plug*, plug));}
		void
			Remove(T plug)
				{RemovePlug(Cast_Pointer(Plug*, plug));}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeChainOf templates ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T>
		SafeChainOf<T>::SafeChainOf(Node *node):
			SafeChain(node)
	{
	}

	template <class T>
		SafeChainOf<T>::~SafeChainOf()
	{
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeChainIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SafeChainIterator:
		public SafeIterator
	{
	public:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Public interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//

		//
		//--------------------------------------------------------------------
		// Constructors, Destructor and testing
		//--------------------------------------------------------------------
		//
		SafeChainIterator(
			SafeChain *chain,
			bool move_next_on_remove
		);
		SafeChainIterator(const SafeChainIterator &iterator);
		~SafeChainIterator();
				
		void
			TestInstance() const;

		//
		//--------------------------------------------------------------------
		// Iterator methods (see Iterator for full listing)
		//--------------------------------------------------------------------
		//
		void
			First();
		void
			Last();
		void
			Next();
		void
			Previous();
		CollectionSize
			GetSize();
		void
			Remove();
	
	protected:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Protected interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		void*
			ReadAndNextImplementation();
		void*
			ReadAndPreviousImplementation();
		void*
			GetCurrentImplementation();
		void*
			GetNthImplementation(
				CollectionSize index
			);
		void
			InsertImplementation(Plug *plug);

	private:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Private interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		void
			ReceiveMemo(
				IteratorMemo memo,
				void *content
			);

		//
		//--------------------------------------------------------------------
		// Private data
		//--------------------------------------------------------------------
		//
		SafeChainLink *currentLink;
		bool moveNextOnRemove;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SafeChainIteratorOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class SafeChainIteratorOf:
		public SafeChainIterator
	{
	public:
  		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Public interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//

		//
		//--------------------------------------------------------------------
		// Constructors and Destructor
		//--------------------------------------------------------------------
		//
		SafeChainIteratorOf(
			SafeChainOf<T> *chain,
			bool move_next_on_remove=true
		);
		SafeChainIteratorOf(const SafeChainIteratorOf<T> &iterator);
		Iterator*
			MakeClone();

		~SafeChainIteratorOf();

		//
		//--------------------------------------------------------------------
		// Iterator methods (see Iterator for full listing)
		//--------------------------------------------------------------------
		//
		T
			ReadAndNext()
				{return (T)ReadAndNextImplementation();}
		T
			ReadAndPrevious()
				{return (T)ReadAndPreviousImplementation();}
		T
			GetCurrent()
				{return (T)GetCurrentImplementation();}
		T
			GetNth(CollectionSize index)
				{return (T)GetNthImplementation(index);}
		void
			Insert(T plug)
				{InsertImplementation(Cast_Object(Plug*,plug));}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~ SafeChainIteratorOf templates ~~~~~~~~~~~~~~~~~~~~~~~

	template <class T>
		SafeChainIteratorOf<T>::SafeChainIteratorOf(
			SafeChainOf<T> *chain,
			bool move_next_on_remove
		):
			SafeChainIterator(chain, move_next_on_remove)
	{
	}

	template <class T>
		SafeChainIteratorOf<T>::SafeChainIteratorOf(const SafeChainIteratorOf<T> &iterator):
			SafeChainIterator(iterator)
	{
	}

	template <class T> Iterator*
		SafeChainIteratorOf<T>::MakeClone()
	{
		return new SafeChainIteratorOf<T>(*this);
	}

	template <class T>
		SafeChainIteratorOf<T>::~SafeChainIteratorOf()
	{
	}

}
