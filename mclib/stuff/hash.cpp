//===========================================================================//
// File:	hash.cc                                                             //
// Project: MUNGA           Brick: Connection Library                        //
// Contents: Implementation details of Hash class                            //
//---------------------------------------------------------------------------//
//   Date    Who  Modification                                               //
// --------  ---  ---------------------------------------------------------- //
// 12/15/94  ECH  Initial coding.                                            //
//---------------------------------------------------------------------------//
// Copyright (C) 1994-1995, Virtual World Entertainment, Inc.                //
//                      PROPRIETARY AND CONFIDENTIAL                         //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~ Hash ~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define Verify_Index(x) Verify(0 <= (x) && (x) < hashTableSize)

//
//###########################################################################
// Hash
//###########################################################################
//
Hash::Hash(
	CollectionSize size,
	Node *node,
	bool has_unique_entries
):
	SortedSocket(node, has_unique_entries)
{
	hashTableSize = size;
	
	//
	// If the size is not prime, warn and select new size
	//
	Warn(!CheckForPrimeSize());
	if (!CheckForPrimeSize())
	{
		while (!CheckForPrimeSize())
		{
			hashTableSize++;
		}
		STOP(("Correct hashTableSize=%d", hashTableSize));
	}

	BuildHashTable();
}

//
//###########################################################################
// ~Hash
//###########################################################################
//
Hash::~Hash()
{
	Check_Object(this);
	int i;

	for (i = 0; i < hashTableSize; i++)
	{
		Check_Pointer(hashTable);
		Verify_Index(i);
		if (hashTable[i] != NULL)
		{
			Unregister_Object(hashTable[i]);
			delete hashTable[i];
		}
	}

	Unregister_Pointer(hashTable);
	delete[] hashTable;
	hashTable = NULL;
	hashTableSize = 0;
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	Hash::TestInstance()
{
	SortedSocket::TestInstance();
	Check_Pointer(hashTable);
	Verify(hashTableSize > 0);
}

//
//###########################################################################
// AddImplementation
//###########################################################################
//
void
	Hash::AddImplementation(Plug *plug)
{
	Check_Object(this);
	STOP(("Hash::AddImplementation - Must add with value"));
}

//
//###########################################################################
// AddValueImplementation
//###########################################################################
//
void
	Hash::AddValueImplementation(
		Plug *plug,
		const void *value
	)
{
	Check_Object(this);
	Check_Object(plug);
	Check_Pointer(value);

	//
	//-------------------------------------------------------------
	// Verify that value has not been added
	//-------------------------------------------------------------
	//
	Verify(HasUniqueEntries() ? (FindImplementation(value) == NULL) : (bool)true);

	//
	//-------------------------------------------------------------
	// Find hash entry
	//-------------------------------------------------------------
	//
	IteratorPosition index;

	index = GetHashIndex(value);

	//
	//-------------------------------------------------------------
	// Get vchain for this index
	//-------------------------------------------------------------
	//
	SortedChain *vchain;

	Check_Pointer(hashTable);
	Verify_Index(index);
	if ((vchain = hashTable[index]) == NULL)
	{
		vchain = MakeSortedChain();
		Register_Object(vchain);
		hashTable[index] = vchain;
	}

	//
	//-------------------------------------------------------------
	// Add to the vchain
	//-------------------------------------------------------------
	//
	Check_Object(vchain);
	vchain->AddValuePlug(plug, value);
}

//
//###########################################################################
// FindImplementation
//###########################################################################
//
Plug*
	Hash::FindImplementation(
		const void *value
	)
{
	Check_Object(this);
	Check_Pointer(value);

	//
	//-------------------------------------------------------------
	// Find hash entry
	//-------------------------------------------------------------
	//
	IteratorPosition index;

	index = GetHashIndex(value);

	//
	//-------------------------------------------------------------
	// Get vchain for this index
	//-------------------------------------------------------------
	//
	SortedChain *vchain;

	Check_Pointer(hashTable);
	Verify_Index(index);
	if ((vchain = hashTable[index]) == NULL)
	{
		return NULL;
	}

	//
	//-------------------------------------------------------------
	// Find in vchain
	//-------------------------------------------------------------
	//
	Check_Object(vchain);
	return vchain->FindPlug(value);
}

//
//#############################################################################
// IsEmpty
//#############################################################################
//
bool
	Hash::IsEmpty()
{
	Check_Object(this);
	
	for (int i = 0; i < hashTableSize; i++)
	{
		Check_Pointer(hashTable);
		Verify_Index(i);
		if (hashTable[i] != NULL)
		{
			if (!hashTable[i]->IsEmpty())
			{
				return false;
			}
		}
	}
	return true;
}

//
//###########################################################################
// MakeSortedChain
//###########################################################################
//
SortedChain*
	Hash::MakeSortedChain()
{
	Check_Object(this);
	STOP(("Hash::MakeSortedChain - Should never reach here"));
   return NULL;
}

//
//###########################################################################
// MakeSortedChain
//###########################################################################
//
IteratorPosition
	Hash::GetHashIndex(const void*)
{
	Check_Object(this);
	STOP(("Hash::GetHashIndex - Should never reach here"));
   return 0;
}

//
//###########################################################################
// BuildHashTable
//###########################################################################
//
void
	Hash::BuildHashTable()
{
	Check_Signature(this);
	int i;

	hashTable = new SortedChain*[hashTableSize];
	Register_Pointer(hashTable);

	for (i = 0; i < hashTableSize; i++)
	{
		Check_Pointer(hashTable);
		Verify_Index(i);
		hashTable[i] = NULL;
	}
}

//
//###########################################################################
// CheckForPrimeSize
//###########################################################################
//
bool
	Hash::CheckForPrimeSize()
{
	Check_Signature(this);
	Verify(hashTableSize > 2);

	CollectionSize upper_bound =
		static_cast<CollectionSize>(Sqrt(static_cast<Scalar>(hashTableSize)));

	for (CollectionSize i = 2; i < upper_bound; i++)
	{
		if ((hashTableSize % i) == 0)
			return false;
	} 
	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~ HashIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~

const IteratorPosition HashIteratorNullPosition = -1;

//
//###########################################################################
// HashIterator
//###########################################################################
//
HashIterator::HashIterator(Hash *hash):
	SortedIterator(hash)
{
	hashTable = hash->hashTable;
	hashTableSize = hash->hashTableSize;
	currentPosition = HashIteratorNullPosition;
	vchainIterator = NULL;
	First();
}

Iterator*
	HashIterator::MakeClone()
{
	Check_Object(this);
	return new HashIterator(*this);
}

HashIterator::~HashIterator()
{
	Check_Object(this);
	hashTable = NULL;
	hashTableSize = 0;
	currentPosition = HashIteratorNullPosition;
	DeleteSortedChainIterator();
}

//
//###########################################################################
// TestInstance
//###########################################################################
//
void
	HashIterator::TestInstance()
{
	SortedIterator::TestInstance();
	Check_Pointer(hashTable);
	Verify(hashTableSize > 0);
	if (vchainIterator != NULL)
	{
		Check_Object(vchainIterator);
	}
}

//
//###########################################################################
// First
//###########################################################################
//
void
	HashIterator::First()
{
	Check_Object(this);
	NextSortedChainIterator(0);
}

//
//###########################################################################
// Last
//###########################################################################
//
void
	HashIterator::Last()
{
	Check_Object(this);
	STOP(("Shouldn't reach here"));
}

//
//###########################################################################
// Next
//###########################################################################
//
void
	HashIterator::Next()
{
	Check_Object(this);
	if (vchainIterator != NULL)
	{
		Check_Object(vchainIterator);

		if (vchainIterator->GetCurrentPlug() != NULL)
		{
			//
			// Try to step to the next item in this list
			//
			vchainIterator->Next();
		}
		if (vchainIterator->GetCurrentPlug() == NULL)
		{
			//
			// At end of list, step to the next list
			//
			NextSortedChainIterator(currentPosition+1);
		}
	}
}

//
//###########################################################################
// Previous
//###########################################################################
//
void
	HashIterator::Previous()
{
	Check_Object(this);
	STOP(("Not implemented"));
}

//
//###########################################################################
// GetCurrentImplementation
//###########################################################################
//
void
	*HashIterator::GetCurrentImplementation()
{
	Check_Object(this);
	if (vchainIterator != NULL)
	{
		Check_Object(vchainIterator);

		if (vchainIterator->GetCurrentPlug() != NULL)
		{
			return vchainIterator->GetCurrentPlug();
		}

		//
		// List was emptied, step to next list
		//
		NextSortedChainIterator(currentPosition+1);

		if (vchainIterator != NULL)
		{
			Check_Object(vchainIterator);
			Verify(vchainIterator->GetCurrentPlug() != NULL);
			return vchainIterator->GetCurrentPlug();
		}
	}
	return NULL;
}

//
//###########################################################################
// GetSize
//###########################################################################
//
CollectionSize
	HashIterator::GetSize()
{
	Check_Object(this);
	HashIterator	iterator(Cast_Object(Hash*, socket));
	CollectionSize i = 0;

	while (iterator.ReadAndNextImplementation() != NULL)
	{
		i++;
	}
	return(i);
}

//
//###########################################################################
// Remove
//###########################################################################
//
void
	HashIterator::Remove()
{
	Check_Object(this);
	if (vchainIterator != NULL)
	{
		Check_Object(vchainIterator);

		if (vchainIterator->GetCurrentPlug() != NULL)
		{
			vchainIterator->Remove();
			return;
		}

		//
		// List was emptied, step to next list
		//
		NextSortedChainIterator(currentPosition+1);

		if (vchainIterator != NULL)
		{
			Check_Object(vchainIterator);
			vchainIterator->Remove();
			return;
		}
	}
}

//
//###########################################################################
// FindImplementation
//###########################################################################
//
Plug
	*HashIterator::FindImplementation(
		const void*
	)
{
	Check_Object(this);
	STOP(("Not implemented"));
	return NULL;
}

//
//###########################################################################
// ReceiveMemo
//###########################################################################
//
void
	HashIterator::ReceiveMemo(
		IteratorMemo,
		void*
	)
{
	Check_Object(this);
}

//
//###########################################################################
//###########################################################################
//
void
	HashIterator::DeleteSortedChainIterator()
{
	if (vchainIterator != NULL)
	{
		Unregister_Object(vchainIterator);
		delete vchainIterator;
      vchainIterator = NULL;
	}
}

//
//###########################################################################
//###########################################################################
//
void
	HashIterator::NextSortedChainIterator(IteratorPosition index)
{
	Check_Object(this);
	int i;

	DeleteSortedChainIterator();
	currentPosition = HashIteratorNullPosition;

	for (i = index; i < hashTableSize; i++)
	{
		Check_Pointer(hashTable);
		Verify_Index(i);

		if (hashTable[i] != NULL)
		{
			//
			// This index contains a vchain
			//
			Check_Object(hashTable[i]);

			//
			// Create a vchain iterator
			//
			vchainIterator = new SortedChainIterator(hashTable[i]);
			Register_Object(vchainIterator);
			if (vchainIterator->GetCurrentPlug() != NULL)
			{
				//
				// The vchain contains items
				//
				currentPosition = i;
				return;
			}

			//
			// Destroy the vchain iterator
			//
			DeleteSortedChainIterator();
		}
	}
}
