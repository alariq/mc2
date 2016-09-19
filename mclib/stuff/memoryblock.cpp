//===========================================================================//
// File:        memblock.cc                                                  //
// Contents: Implementation details of the blockMemory block class           //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//#define MEMORY_VERIFY
//#define MEMORY_BLOCK_VERIFY

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryBlockBase ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MemoryBlockBase*
	MemoryBlockBase::firstBlock = NULL;

//
//#############################################################################
//
//   This function constructs a MemoryBlockBase variable, allocating a block of
// blockMemory from heap, preparing it for suballocation of a constant record
// blockSize.
//
// NOTE:  Record blockSize must be large enough to contain a void pointer in order
//        to properly set up the re-use chain!
//
//   Rec_Size - blockSize in bytes of the suballocation unit
//   Start    - number of records to allocate initially
//   Delta    - number of records to allocate when growing the blockMemory block
//
//#############################################################################
//
MemoryBlockBase::MemoryBlockBase(
	size_t rec_size,
	size_t start,
	size_t delta,
	const char* name,
	HGOSHEAP parent
)
{
	//
	//-----------------------------------------------------------------
	// Make sure that the requested record blockSize is large enough if
	// debugging is enabled
	//-----------------------------------------------------------------
	//
	Verify(rec_size >= sizeof(MemoryBlockHeader));

	//
	//-------------------------------------------------------------------------
	// Set up the blockSize variables for the blockMemory block, and figure out
	// the byte sizes of the initial block and delta blocks
	//-------------------------------------------------------------------------
	//
    // sebi 
	//blockHeap = gos_CreateMemoryHeap(const_cast<char*>(name), 0, parent);
	blockHeap = gos_CreateMemoryHeap(name, 0, parent);
	recordSize = (rec_size+3)&~3;
	blockSize = start * recordSize;
	deltaSize = delta * recordSize;
	blockName = name;

	//
	//-------------------------------------------------------------------------
	// Allocate a block big enough for the requested blockMemory plus a link to
	// the next blockMemory block, initializing this link to NULL
	//-------------------------------------------------------------------------
	//
	#if !defined(MEMORY_VERIFY)
		blockMemory =
			Cast_Pointer(
				MemoryBlockHeader*,
				new(blockHeap) BYTE[sizeof(MemoryBlockHeader) + blockSize]
			);
		Check_Object(blockMemory);
		blockMemory->nextBlock = NULL;
		blockMemory->blockSize = blockSize;

		//
		//--------------------------------------------------------------------
		// Establish the beginning of the firstHeaderRecord record block, and
		// point the freeRecord record pointer to the beginning.  There are no
		// deletedRecord records yet, so make sure Deleted is NULL
		//--------------------------------------------------------------------
		//
		firstHeaderRecord = Cast_Pointer(BYTE*, blockMemory + 1);
		Check_Pointer(firstHeaderRecord);
		freeRecord = firstHeaderRecord;
		deletedRecord = NULL;
	#endif

	//
	//--------------------------------------------------------------------------
	// If this is the first memory block, set the first block pointer and
	// initialize the ring.  If not, insert the new block at the end of the ring
	//--------------------------------------------------------------------------
	//
	if (!firstBlock)
	{
		firstBlock = nextBlock = previousBlock = this;
	}
	else
	{
		MemoryBlockBase *block = firstBlock;
		Check_Object(block);
		block = block->previousBlock;
		Check_Object(block);
		nextBlock = firstBlock;
		previousBlock = block;
		block->nextBlock = this;
		firstBlock->previousBlock = this;
	}
}

//
//#############################################################################
//
//   This function destroys the MemoryBlockBase object, deleting any additional
// record blocks which were allocated
//
//#############################################################################
//
MemoryBlockBase::~MemoryBlockBase()
{
#if !defined(MEMORY_VERIFY)
	//
	//------------------------------------------------------------------------
	// Find the address of the first record block, then delete blocks until we
	// come to the end of the chain
	//------------------------------------------------------------------------
	//
	Check_Object(this);
	MemoryBlockHeader *block = blockMemory;

	while (block)
	{

		//
		//---------------------------------------------------------------------
		// Save the address of the next blockMemory block, then delete this one
		// and get ready to delete the next one
		//---------------------------------------------------------------------
		//
		Check_Object(block);
		MemoryBlockHeader *next_block = block->nextBlock;
		delete block;
		block = next_block;
	}
#endif

	//
	//------------------------------------------------------------------------
	// Remove the block from the ring.  If the block is the first one, set the
	// first block pointer correctly.  Note that we memoryblocks, as static
	// objects, should deconstruct in the opposite order they constructed, so
	// when we reach the first block, it should be the only one in the queue.
	//------------------------------------------------------------------------
	//
	if (firstBlock == this)
	{
		if (nextBlock == this)
		{
			firstBlock = NULL;
		}
		else
		{
			firstBlock = nextBlock;
			goto Unlink;
		}
	}
	else
	{
Unlink:
		MemoryBlockBase *next = nextBlock;
		Check_Object(next);
		MemoryBlockBase *prev = previousBlock;
		Check_Object(prev);
		next->previousBlock = prev;
		prev->nextBlock = next;
	}
}

//
//#############################################################################
//
//   This function allocates a fixed blockSize record from the record blocks
//
//#############################################################################
//
void*
	MemoryBlockBase::Grow()
{
	#if defined(MEMORY_VERIFY)
		STOP(("MemoryBlockBase::Grow() not available!\n"));
	#endif
	//
	//------------------------------------------------------------------------
	// If we have freeRecord space left in the firstHeaderRecord record block,
	// allocate the new record from here, updating the next freeRecord record
	// pointer
	//------------------------------------------------------------------------
	//
	Check_Object(this);
	if (freeRecord - firstHeaderRecord <= blockSize - recordSize)
	{
		void *result = freeRecord;
		freeRecord += recordSize;
		return result;
	}

	//
	//-------------------------------------------------------------------------
	// Allocate a new block of records using the growth blockSize, and make the
	// link field of the firstHeaderRecord block point to the new block.  Then
	// make the new block the firstHeaderRecord block, and make its next link
	// NULL, as it is the end of the chain
	//-------------------------------------------------------------------------
	//
	blockSize = deltaSize;
	firstHeaderRecord -= sizeof(MemoryBlockHeader);
	BYTE *new_block = new(blockHeap) BYTE[blockSize + sizeof(MemoryBlockHeader)];
	Check_Pointer(new_block);

	MemoryBlockHeader *header =
		Cast_Pointer(MemoryBlockHeader*, firstHeaderRecord);
	Check_Object(header);
	header->nextBlock = Cast_Pointer(MemoryBlockHeader*, new_block);
	Check_Object(header->nextBlock);

	firstHeaderRecord = new_block;
	header = Cast_Pointer(MemoryBlockHeader*, firstHeaderRecord);
	Check_Object(header);
	header->nextBlock = NULL;
	header->blockSize = deltaSize;

	//
	//--------------------------------------------------------------------------
	// Make firstHeaderRecord point to the first available address in the new
	// block (having skipped the first field allocated to the link pointer), and
	// allocate this first block to the caller.  Update the freeRecord pointer
	// to reflect this allocation
	//--------------------------------------------------------------------------
	//
	firstHeaderRecord += sizeof(MemoryBlockHeader);
	freeRecord = firstHeaderRecord + recordSize;
	return firstHeaderRecord;
}

//
//#############################################################################
//#############################################################################
//
void
	MemoryBlockBase::UsageReport()
{
#if !defined(MEMORY_VERIFY)
	SPEW((GROUP_STUFF_MEMORY, " #Bytes  #Rcds  #Dels  #Free Use% Name"));
	SPEW((GROUP_STUFF_MEMORY, "------- ------ ------ ------ ---- -------------------------------"));
	MemoryBlockBase *block = firstBlock;
	do
	{
		Check_Object(block);
		MemoryBlockHeader *header = block->blockMemory;
		size_t byte_count = 0;
		size_t record_count = 0;
		size_t deletion_count = 0;
		size_t unused_count =
			block->blockSize - (block->freeRecord - block->firstHeaderRecord);
		unused_count /= block->recordSize;
		while (header)
		{
			Check_Object(header);
			record_count += header->blockSize / block->recordSize;
			byte_count += header->blockSize + sizeof(MemoryBlockHeader);
			header = header->nextBlock;
		}
		BYTE *deletion = block->deletedRecord;
		while (deletion)
		{
			++deletion_count;
			deletion = *Cast_Pointer(BYTE**, deletion);
		}
		record_count -= deletion_count + unused_count;
		if( record_count!=0 )
		{
			SPEW((
			GROUP_STUFF_MEMORY,
			"%7d%7d%7d%7d%4d%%+",
			byte_count,
			record_count,
			deletion_count,
			unused_count,
			static_cast<int>((1.0f - (deletion_count+unused_count)/(float)record_count)*100.0f)
		));
		}
		if (block->blockName)
			SPEW((GROUP_STUFF_MEMORY, " %s", block->blockName));
		SPEW((GROUP_STUFF_MEMORY, ""));
		block = block->nextBlock;
	}
	while (block != firstBlock);
	SPEW((GROUP_STUFF_MEMORY, ""));
#endif
}

//
//#############################################################################
//#############################################################################
//
void
	MemoryBlockBase::CollapseBlocks()
{
	#if defined(LAB_ONLY)
		SPEW((GROUP_STUFF_MEMORY, "MemoryBlock statistics prior to collapsing..."));
		UsageReport();
	#endif
	MemoryBlockBase *block = firstBlock;
	do
	{
		Check_Object(block);
		block->Collapse();
		block = block->nextBlock;
	}
	while (block != firstBlock);
}

#if 0
//
//#############################################################################
//#############################################################################
//
static int
	compare_function(const void* a, const void* b)
{
	return static_cast<const char*>(a) - static_cast<const char*>(b);
}
#endif

//
//#############################################################################
//#############################################################################
//
void
	MemoryBlockBase::Collapse()
{
#if !defined(MEMORY_VERIFY)
	//
	//---------------------------------------
	// count up the number of deleted records
	//---------------------------------------
	//
	BYTE *deletion = deletedRecord;
	size_t deletion_count = 0;
	while (deletion)
	{
		Check_Pointer(deletion);
		++deletion_count;
		deletion = *Cast_Pointer(BYTE**, deletion);
	}
	if (!deletion_count)
	{
		return;
	}

	//
	//--------------------------------------------------------------------------
	// Now, build a table of pointers that big and fill it in with the deletion
	// pointers.  Fill it in backwards because the last blocks to be allocated
	// will probably be the last blocks to be deleted, and thus show up first in
	// the chain
	//--------------------------------------------------------------------------
	//
	BYTE **deletions = new(blockHeap) BYTE*[deletion_count+1];
	deletion = deletedRecord;
	int i=deletion_count-1;
	deletions[deletion_count] = reinterpret_cast<BYTE*>(SNAN_NEGATIVE_LONG);
	deletions[i--] = deletion;
	deletion = *Cast_Pointer(BYTE**, deletion);
	int j;
	while (deletion)
	{
		//
		//---------------------------------------------------
		// We might as well build in the insertion stuff here
		//---------------------------------------------------
		//
		Check_Pointer(deletion);
		j=i--;
		while (deletions[j+1] < deletion)
		{
			Verify(j < deletion_count);
			deletions[j] = deletions[j+1];
			++j;
		}
		deletions[j] = deletion;
		deletion = *Cast_Pointer(BYTE**, deletion);
	}
	Verify(i == -1);

	//
	//------------------------------------------------------------------------
	// Now, step through each additional block and see if it can be eliminated
	//------------------------------------------------------------------------
	//
	MemoryBlockHeader *last_real_block = blockMemory;
	MemoryBlockHeader *header = blockMemory->nextBlock;
	while (header)
	{
		//
		//--------------------------------------------------------------------
		// try to locate the first data entry in the block in the sorted table
		//--------------------------------------------------------------------
		//
		Check_Object(header);
		i = 0;
		j = deletion_count - 1;
		int m = 0;
		BYTE *key = Cast_Pointer(BYTE*, header) + sizeof(*header);
		while (i <= j)
		{
			m = (i + j) >> 1;
			if (deletions[m] == key)
			{
				break;
			}
			else if (deletions[m] > key)
			{
				j = m - 1;
			}
			else
			{
				i = m + 1;
			}
		}

		//
		//---------------------------------------------------------------------
		// If the record wasn't found, then this block cannot be deleted, so it
		// is marked as the last real block and is so linked
		//---------------------------------------------------------------------
		//
		if (i > j)
		{
Real_Block:
			last_real_block->nextBlock = header;
			last_real_block = header;
			header = header->nextBlock;
			continue;
		}

		//
		//----------------------------------------------------------------------
		// We found the beginning, so now we have to see if we can find the end.
		// If this block has a link, it was all used up.  Otherwise, we have to
		// ignore the unused space at the end of the block
		//----------------------------------------------------------------------
		//
		BYTE **start = &deletions[m];
		if (header->nextBlock)
		{
			i = header->blockSize / recordSize;
		}
		else
		{
			i = freeRecord - firstHeaderRecord;
			i /= recordSize;
		}

		//
		//---------------------------------------------------------------------
		// Move the pointer to where the last entry for this block should be if
		// all used records where later deleted.  If this is an impossible
		// location, or if the addresses do not match, this block cannot be
		// collapsed
		//---------------------------------------------------------------------
		//
		BYTE **end = start + i - 1;
		if (
			end - deletions >= deletion_count
			 || *end !=
				Cast_Pointer(BYTE*, header) + sizeof(*header) + (i-1)*recordSize
		)
		{
			goto Real_Block;
		}

		//
		//------------------------------------------------------------------
		// This block may now be killed, but save the next block address for
		// further checking
		//------------------------------------------------------------------
		//
		MemoryBlockHeader *next_block = header->nextBlock;
		delete header;

		//
		//-----------------------------------------------------------------------
		// Now, move everything following our section of deleted records to where
		// our section begins, but only if there is something to move
		//-----------------------------------------------------------------------
		//
		++end;
		j =
			Cast_Pointer(BYTE*, &deletions[deletion_count])
			 - Cast_Pointer(BYTE*, end);
		deletion_count -= i;
		if (j>0)
		{
			memmove(start, end, j);
		}

		//
		//----------------------------
		// Check the next header block
		//----------------------------
		//
		header = next_block;
	}

	//
	//--------------------------------------------------------------------
	// Now, clean up the deletion array and reset all the current deletion
	// pointers
	//--------------------------------------------------------------------
	//
	deletion = NULL;
	for (i=0; i<deletion_count; ++i)
	{
		Check_Pointer(deletions[i]);
		*Cast_Pointer(BYTE**, deletions[i]) = deletion;
		deletion = deletions[i];
	}
	deletedRecord = deletion;
	delete[] deletions;

	//
	//--------------------------------------------------------------------------
	// Set the rest of the pointers to deal with this new last block if the last
	// block is different than what it used to be.  Note that this block should
	// be marked as full
	//--------------------------------------------------------------------------
	//
	last_real_block->nextBlock = NULL;
	if (
		firstHeaderRecord !=
			Cast_Pointer(BYTE*, last_real_block) + sizeof(*last_real_block)
	)
	{
		blockSize = last_real_block->blockSize;
		firstHeaderRecord =
			Cast_Pointer(BYTE*, last_real_block) + sizeof(*last_real_block);
		freeRecord = firstHeaderRecord + blockSize;
	}
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryBlock ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
//
//   This function allocates a fixed blockSize record from the record blocks
//
//#############################################################################
//
void*
	MemoryBlock::New()
{
	void *result;
	#if defined(MEMORY_VERIFY)
		result = new(blockHeap) char[recordSize];
	#else
		//
		//----------------------------------------------------------------------
		// If we have a deletedRecord record, go ahead and reuse it, updating the next
		// deletedRecord record value from the firstHeaderRecord one.  This chain is independant
		// of the firstHeaderRecord record block.  If not, grow the blockMemory block by one
		// record
		//----------------------------------------------------------------------
		//
		Check_Object(this);
		if (deletedRecord)
		{
			result = deletedRecord;
			deletedRecord = *Cast_Pointer(BYTE**, deletedRecord);
		}
		else
		{
			result = Grow();
		}
	#endif

	//
	//----------------------------------------------------------------------
	// If we are checking for unassigned variables, initialize the allocated
	// memory with NANs
	//----------------------------------------------------------------------
	//
	#if defined(_ARMOR)
		Flood_Memory_With_NAN(result, recordSize);
	#endif

	//
	//-------------------------------------
	// Return the address of the new record
	//-------------------------------------
	//
	return result;
}

//
//#############################################################################
//
//   This function deallocates a record, making it available for re-use
//
//#############################################################################
//
void
	MemoryBlock::Delete(
		void* where
	)
{
	#if defined(MEMORY_VERIFY)
		delete where;
	#else
		//
		//--------------------------------------------------------------------
		// If we are in debug2 mode, check to see if the deletedRecord region really
		// belongs to us
		//--------------------------------------------------------------------
		//
		Check_Object(this);

		#if defined(MEMORY_BLOCK_VERIFY)

			//
			//--------------------------------------------------------------------
			// Make sure the address of this record is not already in the deletion
			// chain
			//--------------------------------------------------------------------
			//
			void *record;
			for (
				record = deletedRecord;
				record;
				record = *Cast_Pointer(BYTE**, record)
			)
			{
				if (record == where)
				{
					break;
				}
			}
			Verify(!record);

			//
			//------------------------------------------------------------------
			// Find the address of the first record block, then check each block
			// until we come to the end of the chain
			//------------------------------------------------------------------
			//
			unsigned
				offset;
			MemoryBlockHeader*
				block = blockMemory;

			while (block)
			{

				//
				//---------------------------------------------------------------
				// If the record is in this block and is positioned correctly, go
				// ahead and break as we have found a legal place within a block
				//---------------------------------------------------------------
				//
				Check_Object(block);
				offset =
					static_cast<unsigned>(
						Cast_Pointer(BYTE*, where) - Cast_Pointer(BYTE*, block + 1)
					);
				if (offset < block->blockSize)
				{
					Verify(!(offset % recordSize));
					break;
				}

				//
				//---------------------
				// Go to the next block
				//---------------------
				//
				block = block->nextBlock;
			}
			Verify(block);

			//
			//-----------------------------------------------------------
			// Now make sure that the address is not in our future region
			//-----------------------------------------------------------
			//
			Verify(block->nextBlock || offset < freeRecord-firstHeaderRecord);

		#endif

		//
		//----------------------------------------------------------------------
		// Make the first few bytes of the record act as the link pointer to the
		// beginning of the firstHeaderRecord deletedRecord chain, then make the first record
		// available for reuse this one
		//----------------------------------------------------------------------
		//
		*Cast_Pointer(BYTE**, where) = deletedRecord;
		deletedRecord = Cast_Pointer(BYTE*, where);
	#endif
}

//
//#############################################################################
//#############################################################################
//
void*
	MemoryBlock::operator[](size_t index)
{
	#if defined(MEMORY_VERIFY)
		STOP(("MemoryBlock::operator[] not available!\n"));
	#endif

	//
	//------------------------------------------------------------------
	// Find the address of the first record block, then check each block
	// until we come to the end of the chain
	//------------------------------------------------------------------
	//
	Check_Object(this);
	MemoryBlockHeader
		*block = blockMemory;

	while (block)
	{
		//
		//---------------------------------------------------------------
		// If the index is in this block, go ahead and return its address
		//---------------------------------------------------------------
		//
		Check_Object(block);
		Verify(recordSize);
		int
			records = block->blockSize / recordSize;
		if (index < records)
		{
			return Cast_Pointer(BYTE*, block + 1) + index * recordSize;
		}

		//
		//--------------------------------------------------------------------
		// Save the address of the next blockMemory block, then delete this one and
		// get ready to delete the next one
		//--------------------------------------------------------------------
		//
		index -= records;
		block = block->nextBlock;
	}

	//
	//-----------------------------------------
	// The record doesn't exist, so return NULL
	//-----------------------------------------
	//
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MemoryStack ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
//#############################################################################
//
void*
	MemoryStack::Push(const void* what)
{
	Check_Object(this);
	Check_Pointer(what);

	BYTE *block = firstHeaderRecord;
	topOfStack = freeRecord;
	BYTE *target = static_cast<BYTE*>(Grow());
	Mem_Copy(
		target,
		what,
		recordSize,
		blockSize - (target - firstHeaderRecord)
	);
	if (firstHeaderRecord != block)
	{
		topOfStack = firstHeaderRecord;
	}
	return topOfStack;
}

//
//#############################################################################
//#############################################################################
//
void*
	MemoryStack::Push()
{
	Check_Object(this);
	BYTE *block = firstHeaderRecord;
	topOfStack = freeRecord;
	Grow();

	if (firstHeaderRecord != block)
	{
		topOfStack = firstHeaderRecord;
	}
	return topOfStack;
}

//
//###########################################################################
//###########################################################################
//
void
	MemoryStack::Pop()
{
	MemoryBlockHeader
		*block,
		*new_block;

	//
	//-----------------------------------------
	// Make sure that something is in the stack
	//-----------------------------------------
	//
	Check_Object(this);
	if (topOfStack)
	{
		//
		//--------------------------------------------------------------------
		// If the topOfStack of the stack is not at the bottom of a blockMemory block, the
		// freeRecord and topOfStack pointers can move normally
		//--------------------------------------------------------------------
		//
		if (topOfStack != firstHeaderRecord)
		{
			//
			//-------------------------------------------------------------------
			// If the freeRecord pointer is at the bottom of a block, we have to delete
			// the block and update the variables to the previous block
			//-------------------------------------------------------------------
			//
			if (freeRecord == firstHeaderRecord)
			{
				new_block =
					Cast_Pointer(MemoryBlockHeader*, firstHeaderRecord - 1);
				Check_Object(new_block);
				for (
					block = blockMemory;
					block->nextBlock != new_block;
					block = block->nextBlock
				)
				{
					Check_Object(block);
				}

				delete new_block;
				block->nextBlock = NULL;
				firstHeaderRecord = Cast_Pointer(BYTE*, block + 1);
				blockSize = block->blockSize;
			}

			//
			//-----------------------------------------------
			// Move the topOfStack and freeRecord pointers back one record
			//-----------------------------------------------
			//
			freeRecord = topOfStack;
			topOfStack -= recordSize;
		}

		//
		//--------------------------------------------------------------------
		// Otherwise, we have to wrap the top of the stack pointer back to the
		// previous block.  If there is only one allocated block, then the
		// stack is empty
		//--------------------------------------------------------------------
		//
		else if (!blockMemory->nextBlock)
		{
			topOfStack = NULL;
		}

		//
		//--------------------------------------------------------------------
		// Point the top of the stack to the last record of the previous block
		//--------------------------------------------------------------------
		//
		else
		{
			new_block = Cast_Pointer(MemoryBlockHeader*, firstHeaderRecord - 1);
			Check_Object(new_block);
			for (
				block = blockMemory;
				block->nextBlock != new_block;
				block = block->nextBlock
			)
			{
				Check_Object(block);
			}
			topOfStack =
				Cast_Pointer(BYTE*, block + 1) + block->blockSize - recordSize;
		}
	}
}

