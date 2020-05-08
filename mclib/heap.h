//---------------------------------------------------------------------------
//
// Heap.h -- This file contains the definition for the Base HEAP
//		 			Manager Class.  The Base HEAP manager creates,
//					manages and destroys block of memory using Win32
//					virtual memory calls.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef HEAP_H
#define HEAP_H

#ifdef LINUX_BUILD
#define USE_GOS_HEAP
#endif


//---------------------------------------------------------------------------
// Include Files
#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef DHEAP_H
#include"dheap.h"
#endif

#include<memory.h>

#include"gameos.hpp"
//---------------------------------------------------------------------------
// Macro Definitions
#ifndef NO_ERR
#define NO_ERR	0
#endif

#define OUT_OF_MEMORY	0xBADD0001
#define ALLOC_ZERO		0xBADD0002
#define ALLOC_OVERFLOW	0xBADD0003
#define NULL_FREE_LIST	0xBADD0004
#define HEAP_CORRUPTED	0xBADD0005
#define BAD_FREED_PTR	0xBADD0006
#define ALLOC_TOO_BIG	0xBADD0007
#define COULDNT_COMMIT	0xBADD0008
#define COULDNT_CREATE	0xBADD0009

#ifdef _DEBUG
#define allocatedBlockSize 12
#else
#define allocatedBlockSize 8
#endif

#define BASE_HEAP		0
#define USER_HEAP		1

#define MAX_HEAPS		256
//---------------------------------------------------------------------------
extern UserHeapPtr systemHeap;

//---------------------------------------------------------------------------
// Additional Debug Information
#ifdef _DEBUG
#define NUMMEMRECORDS 128000

typedef struct _memRecord 
{
	void*			ptr;
	unsigned long	size;
	unsigned long	stack[12];
} memRecord;
#endif

typedef struct _GlobalHeapRec
{
	HeapManagerPtr 	thisHeap;
	unsigned long	heapSize;
	unsigned long   totalCoreLeft;
	unsigned long	coreLeft;
} GlobalHeapRec;

//---------------------------------------------------------------------------
// Class Definitions
class HeapManager
{
	//Data Members
	//-------------
	protected:

		MemoryPtr				heap;
		bool					memReserved;
		unsigned long   		totalSize;
		unsigned long   		committedSize;
		unsigned long			whoMadeMe;

//		BOOL	VMQuery (PVOID pvAddress, PVMQUERY pVMQ);
//		LPCTSTR GetMemStorageText (DWORD dwStorage);
//		LPTSTR	GetProtectText (DWORD dwProtect, LPTSTR szBuf, BOOL fShowFlags);
//		void	ConstructRgnInfoLine (PVMQUERY pVMQ, LPTSTR szLine, int nMaxLen);
//		void	ConstructBlkInfoLine (PVMQUERY pVMQ, LPTSTR szLine, int nMaxLen);

	public:
		HeapManagerPtr			nxt;
	
	//Member Functions
	//-----------------
	public:

		void init (void);
		HeapManager (void);
		
		void destroy (void);
		virtual ~HeapManager (void);

		long createHeap (unsigned long memSize);
		long commitHeap (unsigned long commitSize = 0);
		long decommitHeap (unsigned long decommitSize = 0);
		
		MemoryPtr getHeapPtr (void);
		operator MemoryPtr (void);

		void MemoryDump();
		
		virtual unsigned char heapType (void)
		{
			return BASE_HEAP;
		}
		
		unsigned long owner (void)
		{
			return whoMadeMe;
		}
		
		unsigned long tSize (void)
		{
			return committedSize;
		}

};

//---------------------------------------------------------------------------
struct HeapBlock
{
	unsigned long 	blockSize;
	HeapBlockPtr 	upperBlock;
	HeapBlockPtr	previous;
	HeapBlockPtr	next;
};

//---------------------------------------------------------------------------
class UserHeap : public HeapManager
{
	//Data Members
	//-------------	
	protected:
		HeapBlockPtr		heapStart;
		HeapBlockPtr		heapEnd;
		HeapBlockPtr		firstNearBlock;
		unsigned long		heapSize;
		bool				mallocFatals;
		
		long				heapState;

		char				*heapName;

		bool				useGOSGuardPage;
		
        HGOSHEAP			gosHeap;

		#ifdef _DEBUG
		memRecord			*recordArray;
		long				recordCount;
		bool				logMallocs;
		#endif

	//Member Functions
	//-----------------
	protected:
		void	relink (HeapBlockPtr newBlock);
		void	unlink (HeapBlockPtr oldBlock);
		bool	mergeWithLower (HeapBlockPtr block);
		
	public:
	
		UserHeap (void);
#ifdef USE_GOS_HEAP
		long init (unsigned long memSize, const char *heapId = NULL, bool useGOS = true);
#else
		long init (unsigned long memSize, const char *heapId = NULL, bool useGOS = false);
#endif
		
		~UserHeap (void);
		void destroy (void);
		
		unsigned long totalCoreLeft (void);
		unsigned long coreLeft (void);
		unsigned long size (void) { return heapSize;}
		
		void *Malloc (size_t memSize);
		long Free (void *memBlock);
		
		void *calloc (size_t memSize);
		
		void walkHeap (bool printIt = FALSE, bool skipAllocated = FALSE);
		
		long getLastError (void);

		bool heapReady (void)
		{
#ifdef USE_GOS_HEAP
            return gosHeap;
#else
			return (heapSize != 0);
#endif
		}

		void setMallocFatals (bool fatalFlag)
		{
			mallocFatals = fatalFlag;
		}
		
		virtual unsigned char heapType (void)
		{
			return USER_HEAP;
		}
		
		char *getHeapName (void)
		{
			return heapName;
		}

		bool pointerOnHeap (void *ptr);

		#ifdef _DEBUG
		void startHeapMallocLog (void);		//This function will start recoding each malloc and
											//free to insure that there are no leaks.
											
		void stopHeapMallocLog (void);		//This can be used to suspend logging when mallocs
											//that are not desired to be logged are called.
											
		void dumpRecordLog (void);
											
		#else
		void startHeapMallocLog (void)		//DOES NOTHING IN RELEASE/PROFILE BUILDS!
		{
		}
										
		void stopHeapMallocLog (void)
		{
		}
		
		void dumpRecordLog (void)
		{
		}
		#endif
};

//---------------------------------------------------------------------------
// This class tracks each heap which is created to provide more accurate
// memory map information about Honor Bound.
// NOTE this class just records heaps.  It does NO allocating of its own.
class HeapList
{
	//Data Members
	//-------------
		protected:
			static GlobalHeapRec heapRecords[MAX_HEAPS];
			static bool heapInstrumented;
		
	//Member Functions
	//----------------
		public:
			void init (void)
			{
				memset(heapRecords,0,sizeof(GlobalHeapRec)*MAX_HEAPS);

				heapInstrumented = false;
			}
			
			HeapList (void)
			{
				init();
			}
			
			void destroy (void)
			{
				init();
			}
			
			~HeapList (void)
			{
				destroy();
			}
			
			void addHeap (HeapManagerPtr newHeap);
			void removeHeap (HeapManagerPtr oldHeap);
			
			void update (void);			//Called every frame in Debug to monitor heaps!

			void dumpLog (void);

			static void initializeStatistics();

};

//---------------------------------------------------------------------------
typedef HeapList *HeapListPtr;
extern HeapListPtr globalHeapList;

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
//---------------------------------------------------------------------------

