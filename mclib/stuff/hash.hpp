//===========================================================================//
// File:	hash.hh                                                          //
// Contents:                                                                 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"sortedsocket.hpp"
#include"sortedchain.hpp"

namespace GetHashFunctions {
	inline Stuff::IteratorPosition
		GetHashValue(int value_to_hash)
	{
		return value_to_hash;
	}
};

// sebi same crosserefence in header roblem, need to add declaration here explicitly
namespace GetHashFunctions {
	Stuff::IteratorPosition
		GetHashValue(const Stuff::MString &value);
}

namespace Stuff {

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Hash ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Hash:
		public SortedSocket
	{
		friend class HashIterator;

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
		// Constructor, Destructor and Testing
		//--------------------------------------------------------------------
		//
		Hash(
	      	CollectionSize size,
			Node *node,
			bool has_unique_entries
		);
		~Hash();

		void
			TestInstance();
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

		//
		//-----------------------------------------------------------------------
		// Unimplemented
		//-----------------------------------------------------------------------
		//
		void
			RemovePlug(Plug *plug);
		bool
			IsPlugMember(Plug *plug);

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
		void
			AddValueImplementation(
				Plug *plug,
				const void *value
			);
		Plug
			*FindImplementation(const void *value);

		//
		//--------------------------------------------------------------------
		// Protected data
		//--------------------------------------------------------------------
		//
		SortedChain **hashTable;
		CollectionSize hashTableSize;

	private:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Private interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		virtual SortedChain*
			MakeSortedChain();

		virtual IteratorPosition
			GetHashIndex(const void *value);

		void
			BuildHashTable();

		bool
			CheckForPrimeSize();
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Hash inlines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	inline void
		Hash::RemovePlug(Plug *plug)
	{
		STOP((
			"Hash::RemovePlug - no efficient implementation, use iterator find & remove"
		));
	}

	inline bool
		Hash::IsPlugMember(Plug *plug)
	{
		STOP((
			"Hash::IsPlugMember - no efficient implementation, use find"
		));
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HashOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T, class V> class HashOf:
		public Hash
	{
	public:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Public interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		HashOf(
			CollectionSize size,
			Node *node,
			bool has_unique_entries
		);
		~HashOf();

		//
		//--------------------------------------------------------------------
		// Socket methods (see Socket for full listing)
		//--------------------------------------------------------------------
		//
		void
			AddValue(
				T plug,
				const V &value
			)
				{AddValueImplementation(Cast_Object(Plug*,plug), &value);}
		T
			Find(const V &value)
				{return (T)FindImplementation(&value);}

	private:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		// Private interface
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		SortedChain*
			MakeSortedChain();

		IteratorPosition
			GetHashIndex(const void *value);
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HashOf templates ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T, class V>
		HashOf<T, V>::HashOf(
			CollectionSize size,
			Node *node,
			bool has_unique_entries
		):
			Hash(
				size,
				node,
				has_unique_entries
			)
	{
	}

	template <class T, class V>
		HashOf<T, V>::~HashOf()
	{
		#if defined(_ARMOR)
			CollectionSize over_loaded_bins = 0;
			for (int i = 0; i < hashTableSize; i++)
			{
				Check_Pointer(hashTable);
				if (hashTable[i] != NULL)
				{
					Check_Object(hashTable[i]);
					SortedChainIteratorOf<T, V> 
						iterator((SortedChainOf<T, V>*)hashTable[i]);
					if (iterator.GetSize() > 6)
					{
						over_loaded_bins++;
					}
				}
			}
			Warn(over_loaded_bins != 0);
		#endif
	}

	template <class T, class V> SortedChain*
		HashOf<T, V>::MakeSortedChain()
	{
		return new SortedChainOf<T, V>(GetReleaseNode(), HasUniqueEntries());
	}

	template <class T, class V> IteratorPosition
		HashOf<T, V>::GetHashIndex(const void *value)
	{
		Check_Pointer(value);
		return (GetHashFunctions::GetHashValue(*Cast_Pointer(const V*, value)) % hashTableSize);
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HashIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class HashIterator:
		public SortedIterator
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
		explicit HashIterator(Hash *hash);
		Iterator*
			MakeClone();
		~HashIterator();
		void
			TestInstance();

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
			GetCurrentImplementation();
		Plug*
			FindImplementation(const void *value);

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

		void
			DeleteSortedChainIterator();

		void
			NextSortedChainIterator(IteratorPosition index);

		//
		//--------------------------------------------------------------------
		// Private data
		//--------------------------------------------------------------------
		//
		SortedChain
			**hashTable;
		CollectionSize
			hashTableSize;
		IteratorPosition
			currentPosition;
		SortedChainIterator
			*vchainIterator;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HashIteratorOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T, class V> class HashIteratorOf:
		public HashIterator
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
		explicit HashIteratorOf(HashOf<T, V> *hash);
		Iterator*
			MakeClone();
		~HashIteratorOf();

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
		T
			Find(const V &value)
				{return (T)FindImplementation(&value);}
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~ HashIteratorOf templates ~~~~~~~~~~~~~~~~~~~~~~~~

	template <class T, class V>
		HashIteratorOf<T, V>::HashIteratorOf(HashOf<T, V> *hash):
			HashIterator(hash)
	{
	}

	template <class T, class V> Iterator*
		HashIteratorOf<T, V>::MakeClone()
	{
		return new HashIteratorOf<T,V>(*this);
	}

	template <class T, class V>
		HashIteratorOf<T, V>::~HashIteratorOf()
	{
	}

}
