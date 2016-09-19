//===========================================================================//
// File:	chain.hh                                                         //
// Contents: Interface specification of Chains and their iterators           //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"node.hpp"
#include"memoryblock.hpp"

namespace Stuff {

	class Chain;
	class ChainIterator;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ChainLink ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	enum {
		ChainLink_MemoryBlock_Allocation = 1000
	};

	class ChainLink : public Link
	{
	public:
		friend class Chain;
		friend class ChainIterator;

		static void
			InitializeClass(
				size_t block_count = ChainLink_MemoryBlock_Allocation,
				size_t block_delta = ChainLink_MemoryBlock_Allocation
			);
		static void
			TerminateClass();

	public:
		~ChainLink();
		void
			TestInstance()
				{}

	private:
		ChainLink(
			Chain *chain,
			Plug *plug,
			ChainLink *nextChainLink,
			ChainLink *prevChainLink
		);

		ChainLink
			*nextChainLink,
			*prevChainLink;

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

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Chain ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Chain:
		public Socket
	{
		friend class ChainLink;
		friend class ChainIterator;

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
		// Constructor, Destructor and testing
		//--------------------------------------------------------------------
		//
		explicit Chain(Node *node);
		~Chain();

		void
			TestInstance()
				{}
		static bool
			TestClass();
		static bool
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
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Protected interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		void
			AddImplementation(Plug *plug);

	private:
		//
		//--------------------------------------------------------------------
		// Private methods
		//--------------------------------------------------------------------
		//
		ChainLink*
			InsertChainLink(
				Plug *plug,
				ChainLink *current_link
			);

		//
		//--------------------------------------------------------------------
		// Private data
		//--------------------------------------------------------------------
		//
		ChainLink
			*head,
			*tail;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ChainOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class ChainOf:
		public Chain
	{
	public:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Public interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		explicit ChainOf(Node *node);
		~ChainOf();

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

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ ChainOf templates ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T>
		ChainOf<T>::ChainOf(Node *node):
			Chain(node)
	{
	}

	template <class T>
		ChainOf<T>::~ChainOf()
	{
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ ChainIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class ChainIterator:
		public SocketIterator
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
		explicit ChainIterator(Chain *chain):
			SocketIterator(chain)
				{Check_Object(chain); currentLink = chain->head;}

		ChainIterator(const ChainIterator &iterator):
			SocketIterator(iterator.socket)
				{Check_Object(&iterator); currentLink = iterator.currentLink;}

		~ChainIterator()
			{}
		
		void
			TestInstance() const
				{}

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
			GetNthImplementation(CollectionSize index);
		void
			InsertImplementation(Plug*);

		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Protected data
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		ChainLink
			*currentLink;
	};

	inline void*
		ChainIterator::ReadAndNextImplementation()
	{
		if (currentLink != NULL)
		{
			Check_Object(currentLink);
			Plug *plug = currentLink->plug;
			currentLink = currentLink->nextChainLink;
			return plug;
		}
		return NULL;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ ChainIteratorOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class ChainIteratorOf:
		public ChainIterator
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
		ChainIteratorOf(ChainOf<T> *chain);
		ChainIteratorOf(const ChainIteratorOf<T> &iterator);
		Iterator*
			MakeClone();
		~ChainIteratorOf();

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

		ChainIteratorOf<T>&
			Begin()
				{return (ChainIteratorOf<T>&)BeginImplementation();}
		ChainIteratorOf<T>&
			End()
				{return (ChainIteratorOf<T>&)EndImplementation();}
		ChainIteratorOf<T>&
			Forward()
				{return (ChainIteratorOf<T>&)ForwardImplementation();}
		ChainIteratorOf<T>&
			Backward()
				{return (ChainIteratorOf<T>&)BackwardImplementation();}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~ ChainIteratorOf templates ~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T>
		ChainIteratorOf<T>::ChainIteratorOf(ChainOf<T> *chain):
			ChainIterator(chain)
	{
	}

	template <class T>
		ChainIteratorOf<T>::ChainIteratorOf(const ChainIteratorOf<T> &iterator):
			ChainIterator(iterator)
	{
	}

	template <class T> Iterator*
		ChainIteratorOf<T>::MakeClone()
	{
		return new ChainIteratorOf<T>(*this);
	}

	template <class T>
		ChainIteratorOf<T>::~ChainIteratorOf()
	{
	}

}
