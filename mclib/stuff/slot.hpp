//===========================================================================//
// File:	slot.hh                                                          //
// Contents: Interface specification for slot class                          //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"socket.hpp"
#include"memoryblock.hpp"

namespace Stuff {

	class Slot;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SlotLink ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	enum {
		SlotLink_MemoryBlock_Allocation = 100
	};

	class SlotLink:
		public Link
	{
		friend class Slot;

	public:
		static void
			InitializeClass(
				size_t block_count = SlotLink_MemoryBlock_Allocation,
				size_t block_delta = SlotLink_MemoryBlock_Allocation
			);
		static void
			TerminateClass();

	public:
		~SlotLink();

	private:
		SlotLink(
			Slot *slot,
			Plug *plug
		);

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

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ Slot ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Slot : public Socket
	{
		friend class SlotLink;

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
		explicit Slot(Node *node);
		~Slot();

		void
			TestInstance();
			
		//
		//--------------------------------------------------------------------
		// Remove	- Remove the link
		//--------------------------------------------------------------------
		//
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
		void
			AddImplementation(Plug *plug);
		Plug*
			GetCurrentPlug();

	private:
		//
		//--------------------------------------------------------------------
		// Private data
		//--------------------------------------------------------------------
		//
		SlotLink
			*slotLink;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ SlotOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class SlotOf:
		public Slot
	{
	public:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Public interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		explicit SlotOf(Node *node);
		~SlotOf();

		//
		//--------------------------------------------------------------------
		// Socket methods (see Socket for full listing)
		//--------------------------------------------------------------------
		//
		void
			Add(T plug)
				{AddImplementation(Cast_Object(Plug*,plug));}

		T
			GetCurrent()
				{return (T)GetCurrentPlug();}
	};


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~ SlotOf templates ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T>
		SlotOf<T>::SlotOf(Node *node):
			Slot(node)
	{
	}

	template <class T>
		SlotOf<T>::~SlotOf()
	{
	}

}

