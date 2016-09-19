//===========================================================================//
// File:	plug.hpp                                                         //
// Contents: Interface specifications for plugs and their iterators          //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"registeredclass.hpp"
#include"iterator.hpp"
#include"link.hpp"

namespace Stuff {

	//##########################################################################
	//############################    Plug    ##################################
	//##########################################################################

	class PlugIterator;

	typedef RegisteredClass__ClassData Plug__ClassData;

	extern HGOSHEAP ConnectionEngineHeap;

	class Plug :
		public RegisteredClass
	{
		friend class Link;
		friend class PlugIterator;
		friend class Socket;

	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor/Destructor
	//
	public:
		~Plug();

	protected:
		explicit Plug(ClassData *class_data);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
	public:
		typedef Plug__ClassData ClassData;

		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Utilities
	//
	public:
		CollectionSize
			GetSocketCount();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Private methods
	//
	private:
		void
			RemoveSocket(Socket *socket);
		bool
			IsSocketMember(Socket *socket);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Private data
	//
	private:
		Link *linkHead;
	};

	//##########################################################################
	//############################    PlugOf    ################################
	//##########################################################################

	template <class T> class PlugOf:
		public Plug
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor, Destructor
	//
	public:
		//
		//--------------------------------------------------------------------
		// Constructor, Destructor
		//--------------------------------------------------------------------
		//
		explicit PlugOf(const T &item);
		~PlugOf();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Accessors, Casting
	//
	public:
		T
			GetItem() const
         	{return item;}
		T*
			GetPointer()
         	{return &item;}

		operator T() const
			{return item;}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Private data
	//
	private:
		T item;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PlugOf templates ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> PlugOf<T>::PlugOf(const T &the_item):
		Plug(DefaultData)
	{
		item = the_item;
	}

	template <class T> PlugOf<T>::~PlugOf()
	{
	}

	//##########################################################################
	//########################    PlugIterator    ##############################
	//##########################################################################

	class PlugIterator:
		public Iterator
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor, destructor & testing
	//
	public:
		explicit PlugIterator(
			Plug *plug,
			RegisteredClass::ClassID class_to_iterate=NullClassID
		);
		PlugIterator(const PlugIterator &iterator);
		~PlugIterator();

		void
			TestInstance() const;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Iterator methods (see Iterator for full listing)
	//
	public:
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

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Protected implementation
	//
	protected:
		void*
			ReadAndNextImplementation();
		void*
			ReadAndPreviousImplementation();
		void*
			GetCurrentImplementation();
		void*
			GetNthImplementation(CollectionSize index);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Protected data
	//
	protected:
		Plug
			*plug;
		Link
			*currentLink;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Private methods and data
	//
   private:
      void
      	NextNode();
      void
      	PreviousNode();

		RegisteredClass::ClassID
      	classToIterate;
	};

	//##########################################################################
	//#######################    PlugIteratorOf    #############################
	//##########################################################################

	template <class T> class PlugIteratorOf:
		public PlugIterator
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructor, destructor
	//
	public:
		PlugIteratorOf(
			Plug *plug,
			RegisteredClass::ClassID class_to_iterate=NullClassID
		);
		PlugIteratorOf(
			Plug &plug,
			RegisteredClass::ClassID class_to_iterate=NullClassID
		);
		PlugIteratorOf(const PlugIteratorOf<T> &iterator);
		Iterator*
			MakeClone()
				{Check_Object(this); return new PlugIteratorOf<T>(*this);}

		~PlugIteratorOf();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Iterator methods (see Iterator for full listing)
	//
	public:
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
	};

	//~~~~~~~~~~~~~~~~~~~~~~~ PlugIteratorOf templates ~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T>
		PlugIteratorOf<T>::PlugIteratorOf(
      	Plug *plug,
         RegisteredClass::ClassID class_to_iterate
		):
			PlugIterator(plug, class_to_iterate)
	{
	}

	template <class T>
		PlugIteratorOf<T>::PlugIteratorOf(
			Plug &plug,
			RegisteredClass::ClassID class_to_iterate
		):
			PlugIterator(&plug, class_to_iterate)
	{
	}

	template <class T>
		PlugIteratorOf<T>::PlugIteratorOf(const PlugIteratorOf<T> &iterator):
			PlugIterator(iterator)
	{
	}

	template <class T>
		PlugIteratorOf<T>::~PlugIteratorOf()
	{
	}

}
