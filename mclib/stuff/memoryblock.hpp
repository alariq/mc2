//===========================================================================//
// File:	memblock.hh                                                      //
// Contents: Interface specification of the memory block class               //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryBlockHeader ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	class MemoryBlockHeader
	{
	public:
		MemoryBlockHeader
			*nextBlock;
		size_t
			blockSize;

		void
			TestInstance()
				{}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryBlockBase ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class MemoryBlockBase
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		void
			TestInstance()
				{Verify(blockMemory != NULL);}

		static void
			UsageReport();
		static void
			CollapseBlocks();

	protected:
		const char
			*blockName;

		MemoryBlockHeader
			*blockMemory;			// the first record block allocated

		size_t
			blockSize,				// size in bytes of the current record block
			recordSize,				// size in bytes of the individual record
			deltaSize;				// size in bytes of the growth blocks

		BYTE
			*firstHeaderRecord,	// the beginning of useful free space
			*freeRecord,			// the next address to allocate from the block
			*deletedRecord;		// the next record to reuse

		MemoryBlockBase(
			size_t rec_size,
			size_t start,
			size_t delta,
			const char* name,
			HGOSHEAP parent = ParentClientHeap
		);
		~MemoryBlockBase();

		void*
			Grow();

		HGOSHEAP
			blockHeap;

	private:
		static MemoryBlockBase
			*firstBlock;
		MemoryBlockBase
			*nextBlock,
			*previousBlock;

		void
			Collapse();
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryBlock ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class MemoryBlock:
		public MemoryBlockBase
	{
	public:
		static bool
			TestClass();

		MemoryBlock(
			size_t rec_size,
			size_t start,
			size_t delta,
			const char* name,
			HGOSHEAP parent = ParentClientHeap
		):
			MemoryBlockBase(rec_size, start, delta, name, parent)
				{}

		void*
			New();
		void
			Delete(void *Where);

		void*
			operator[](size_t Index);
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryBlockOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class MemoryBlockOf:
		public MemoryBlock
	{
	public:
		MemoryBlockOf(
			size_t start,
			size_t delta,
			const char* name,
			HGOSHEAP parent = ParentClientHeap
		):
			MemoryBlock(sizeof(T), start, delta, name, parent)
		{}

		T*
			New()
				{return Cast_Pointer(T*, MemoryBlock::New());}
		void
			Delete(void *where)
				{MemoryBlock::Delete(where);}

		T*
			operator[](size_t index)
				{return Cast_Pointer(T*, MemoryBlock::operator[](index));}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryStack ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class MemoryStack:
		public MemoryBlockBase
	{
	public:
		static bool
			TestClass();

	protected:
		BYTE
			*topOfStack;
	
		MemoryStack(
			size_t rec_size,
			size_t start,
			size_t delta,
			const char* name,
			HGOSHEAP parent = ParentClientHeap
		):
			MemoryBlockBase(rec_size, start, delta, name, parent)
				{topOfStack = NULL;}

		void*
			Push(const void *What);
		void*
			Push();
		void*
			Peek()
				{return topOfStack;}
		void
			Pop();
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryStackOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T> class MemoryStackOf:
		public MemoryStack
	{
	public:
		MemoryStackOf(
			size_t start,
			size_t delta,
			const char *name,
			HGOSHEAP parent = ParentClientHeap
		):
			MemoryStack(sizeof(T), start, delta, name, parent)
		{}

		T*
			Push(const T *what)
				{return Cast_Pointer(T*, MemoryStack::Push(what));}
		T*
			Peek()
				{return static_cast<T*>(MemoryStack::Peek());}
		void
			Pop()
				{MemoryStack::Pop();}
	};

}
