//---------------------------------------------------------------------------
//
// Heap.cpp -- This file contains the definition for the Base HEAP
//		 			Manager Class.  The Base HEAP manager creates,
//		 			manages and destroys block of memory using Win32
//		 			Virtual memory calls.h
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files
#ifndef HEAP_H
#include"heap.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

#include"platform_windows.h"
#ifndef USE_GOS_HEAP
#include<imagehlp.h>
#endif

#include<gameos.hpp>

#ifndef USE_GOS_HEAP
#include<tchar.h>
#endif

#include"platform_str.h" 
#include <ctype.h> // toupper

//---------------------------------------------------------------------------
// Static Globals
static const char CorruptMsg[] = "Heap check failed.\n";
static const char pformat[] = "%s %s\n";

GlobalHeapRec HeapList::heapRecords[MAX_HEAPS];
HeapListPtr globalHeapList = NULL;

unsigned long memCoreLeft = 0;
unsigned long memTotalLeft = 0;

unsigned long totalSize = 0;
unsigned long totalCoreLeft = 0;
unsigned long totalLeft = 0;

bool HeapList::heapInstrumented = 0;

#ifndef USE_GOS_HEAP
//
// Returns a context ready for stack walking from current address
//
void GetCurrentContext( CONTEXT* Context )
{
	memset( Context, 0, sizeof(CONTEXT) );
	_asm
	{
		mov ebx,Context
		mov [ebx]+CONTEXT.Ebp,ebp 
		mov [ebx]+CONTEXT.Esp,esp 
		call GetEIP
		mov [ebx]+CONTEXT.Eip,eax
		jmp Exit
	GetEIP:
		pop eax
		push eax
		ret
	Exit:		
	}
}

void InitStackWalk(STACKFRAME *sf, CONTEXT *Context);
int WalkStack(STACKFRAME *sf);
char* DecodeAddress( DWORD Address , bool brief);

#endif // USE_GOS_HEAP

//---------------------------------------------------------------------------
// Macro definitions
#define USE_BEST_FIT

#ifdef _DEBUG
#define SAFE_HEAP
#endif

#ifdef _DEBUG
#define CHECK_HEAP
#endif

#ifdef LAB_ONLY
#define CHECK_HEAP
#endif

//---------------------------------------------------------------------------
// Class HeapManager Member Functions
HeapManager::~HeapManager (void)
{
	destroy();
}
		
//---------------------------------------------------------------------------
void HeapManager::destroy (void)
{
	long result = 0;
	
	//-----------------------------
	// Remove this from the UEBER HEAP 
#ifdef CHECK_HEAP
	globalHeapList->removeHeap(this);
#endif
	
	if (committedSize)
	{
		result = VirtualFree(heap,totalSize,MEM_DECOMMIT);
		if (result == FALSE)
			result = GetLastError();
	}

	if (totalSize && memReserved && heap)
	{                 
		result = VirtualFree(heap, totalSize, MEM_RELEASE);
		if (result == FALSE)
			result = GetLastError();
	}

	init();
}
		
//---------------------------------------------------------------------------
HeapManager::HeapManager (void)
{
	init();
}
		
//---------------------------------------------------------------------------
void HeapManager::init (void)
{
	heap = NULL;
	memReserved = FALSE;
	totalSize = 0;
	committedSize = 0;
	nxt = NULL;
}
		
//---------------------------------------------------------------------------
HeapManager::operator MemoryPtr (void)
{
	return getHeapPtr();
}
		
//---------------------------------------------------------------------------
MemoryPtr HeapManager::getHeapPtr (void)
{
	if (memReserved && totalSize && committedSize && heap)
		return heap;

	return NULL;
}

//---------------------------------------------------------------------------
long HeapManager::createHeap (unsigned long memSize)
{
	heap = (MemoryPtr)VirtualAlloc(NULL,memSize,MEM_RESERVE,PAGE_READWRITE);

	if (heap)
	{
		memReserved = TRUE;
		totalSize = memSize;
		return NO_ERR;
	}

	return COULDNT_CREATE;
}

//---------------------------------------------------------------------------
long HeapManager::commitHeap (unsigned long commitSize)
{
	if (commitSize == 0)
		commitSize = totalSize;
	
	if (commitSize > totalSize)
		return ALLOC_TOO_BIG;

	if (commitSize < totalSize)
		return COULDNT_COMMIT;

	unsigned long memLeft = totalSize - committedSize;
	
	if (!memLeft)
	{
		return OUT_OF_MEMORY;
	}

	if (memLeft < commitSize)
	{
		commitSize = memLeft;
	}

	MemoryPtr result = (MemoryPtr)VirtualAlloc(heap,commitSize,MEM_COMMIT,PAGE_READWRITE);

	if (result == heap)
	{
		long actualSize = commitSize;
		committedSize += actualSize;

		#ifdef CHECK_HEAP		
		//-----------------------------
		// Add this to the UEBER HEAP 
		globalHeapList->addHeap(this);
		#endif
		
		//------------------------------
		// Store off who called this.
		// If this was a UserHeap,
		// the UserHeap class will
		// do its own unwind.
		unsigned long currentEbp = 0;
		unsigned long prevEbp = 0;
		unsigned long retAddr = 0;

#ifdef PLATFORM_WINDOWS
#ifndef _WIN64
		__asm { mov currentEbp,esp }
#else	
		return NO_ERR;
#endif
#else
		// only correct for 64bit?
        // currentEbp = esp;
        asm("mov %%rsp, %0;"
            : "=r"(currentEbp)
            :
            :
        );
#endif
		prevEbp = *((unsigned long *)currentEbp);
		retAddr = *((unsigned long *)(currentEbp+4));
		whoMadeMe = retAddr;

		return NO_ERR;
	}

	return COULDNT_COMMIT;
}
		
//---------------------------------------------------------------------------
long HeapManager::decommitHeap (unsigned long decommitSize)
{
	long result = 0;
	
	if (decommitSize == 0)
		decommitSize = totalSize;
		
	if (decommitSize > committedSize)
		decommitSize = committedSize;

	if (decommitSize < committedSize)
		decommitSize = totalSize;

	unsigned long decommitAddress = decommitSize;
	committedSize -= decommitAddress;

	result = VirtualFree((void *)committedSize,decommitSize,MEM_DECOMMIT);
	if (result == FALSE)
		result = GetLastError();

	return NO_ERR;
}


/////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// Class UserHeap Member Functions
UserHeap::UserHeap (void) : HeapManager()
{
	heapStart = NULL;
	heapEnd = NULL;
	firstNearBlock = NULL;
	heapSize = 0;
	
	#ifdef CHECK_HEAP
	mallocFatals = TRUE;
	#else
	mallocFatals = FALSE;
	#endif	
		
	heapState = NO_ERR;
}

//---------------------------------------------------------------------------
long UserHeap::init (unsigned long memSize, const char *heapId, bool useGOS)
{
	if (heapId)
	{
		heapName = (char *)::gos_Malloc(strlen(heapId)+1);
		strcpy(heapName,heapId);
	}
	else
		heapName = NULL;
#ifndef USE_GOS_HEAP
	if (!useGOS)
	{
		long result = createHeap(memSize);
		
		if (result)
			STOP(("Could not create Heap %s.  Error:%x",heapId,result));
			
		result = commitHeap(memSize);
		
		if (result)
			STOP(("Could not create Heap %s.  Error:%x",heapId,result));
	
		//------------------------------
		// Store off who called this.
		// If this was a UserHeap,
		// the UserHeap class will
		// do its own unwind.
		unsigned long currentEbp;
		unsigned long prevEbp;
		unsigned long retAddr;
		
		__asm
		{
			mov currentEbp,esp
		}
		
		prevEbp = *((unsigned long *)currentEbp);
		retAddr = *((unsigned long *)(currentEbp+4));
		whoMadeMe = retAddr;
		
		//------------------------------------------------------------------------
		// Now that we have a pointer to the memory, setup the HEAP.
		unsigned long heapTop = (unsigned long)heap;
		heapTop += memSize;
		heapTop -= 16;				
		heapTop &= ~3;				//Force top to be DWORD boundary.
	
		unsigned long heapBottom = (unsigned long)heap;
	
		heapStart = (HeapBlockPtr)heapBottom;
		heapEnd = (HeapBlockPtr)heapTop;
	
		heapStart->blockSize = heapTop - heapBottom;
		heapStart->upperBlock = 0;							//Nothing above this in memory.
	
		heapEnd->blockSize = 1;								//Mark as last block.
		heapEnd->previous = (HeapBlockPtr)0x1572;		//Mark as last block.
		heapEnd->upperBlock = (HeapBlockPtr)heapBottom;
	
		//--------------------------------
		//	Set all free memory to -1.
		// Any access before ready and Exception city.
		MemoryPtr start = (MemoryPtr)heapBottom;
		start += sizeof(HeapBlock);
	
		unsigned long length = heapTop-heapBottom;
		length -= sizeof(HeapBlock);
	
		FillMemory(start,length,0xff);
	
		//----------------------------------
		// linkup heap blocks
		firstNearBlock = NULL;
		relink(heapStart);
	
		heapSize = memSize;
	
		#ifdef _DEBUG
		recordArray = NULL;
		recordCount = 0;
		logMallocs = FALSE;
		#endif;
		
		gosHeap = 0;
	}
	else
#endif
	{
		gosHeap = gos_CreateMemoryHeap(heapId,memSize);
		useGOSGuardPage = true;

		heapStart = NULL;
		heapEnd = NULL;
		firstNearBlock = NULL;
		heapSize = 0;

		heapName = NULL;
		heapState = NO_ERR;

		#ifdef _DEBUG
		recordArray = NULL;
		#endif
	}
	
	return NO_ERR;
}
	
#ifdef _DEBUG
//---------------------------------------------------------------------------
void UserHeap::startHeapMallocLog (void)
{
	if (!recordArray)
	{
		recordArray = new memRecord[NUMMEMRECORDS];
		memset(recordArray, 0, sizeof(memRecord) * NUMMEMRECORDS);
		recordCount = 0;
	}
	
	logMallocs = TRUE;
}	

//---------------------------------------------------------------------------
void UserHeap::stopHeapMallocLog (void)
{
	logMallocs = FALSE;
}	

//---------------------------------------------------------------------------
void UserHeap::dumpRecordLog (void)
{
#ifndef USE_GOS_HEAP
	if (recordArray)
	{
		File log;
		char msg[256];

		sprintf(msg,"heapdump.%s.log",heapName);
		log.create(msg);

		for (int i=0; i<NUMMEMRECORDS; i++)
		{
			if (recordArray[i].ptr)
			{
				sprintf(msg, "Allocated block at DS:%08X, size = %u\n", recordArray[i].ptr, recordArray[i].size);
				log.writeLine(msg);
				char* addressName = DecodeAddress(recordArray[i].stack[0],false);
				sprintf(msg, "Call stack: %08X : %s", recordArray[i].stack[0],addressName);
				log.writeLine(msg);
				for (int j=1; j<12; j++)
				{
				
					if (recordArray[i].stack[j] == 0x0)
						break;
					char* addressName = DecodeAddress(recordArray[i].stack[j],false);
					sprintf(msg, "            %08X : %s", recordArray[i].stack[j],addressName);
					log.writeLine(msg);
				}
				log.writeByte('\n');
			}
		}

		log.close();
	}
#endif
}	
#endif
	
//---------------------------------------------------------------------------
UserHeap::~UserHeap (void)
{
	destroy();
}

//---------------------------------------------------------------------------
void UserHeap::destroy (void)
{
	HeapManager::destroy();
#ifndef USE_GOS_HEAP	
	if (!gosHeap)
	{
		heapStart = NULL;
		heapEnd = NULL;
		firstNearBlock = NULL;
		heapSize = 0;

		if (heapName)
		{
			::gos_Free(heapName);
			heapName = NULL;
		}

		heapState = NO_ERR;

		#ifdef _DEBUG
		if (recordArray)
		{
			delete [] recordArray;
			recordArray = NULL;
		}
		#endif
	}
	else
#endif
	{
		gos_DestroyMemoryHeap(gosHeap,false);
		gosHeap = NULL;
	}
}
		
//---------------------------------------------------------------------------
unsigned long UserHeap::totalCoreLeft (void)
{
	unsigned long result = 0;

	if (gosHeap)
		return result;

#ifndef USE_GOS_HEAP

#ifdef SAFE_HEAP
	long localHeapState = heapState;
#endif

	HeapBlockPtr localFirst = firstNearBlock;
	long heapBlockSize = -(long)allocatedBlockSize;
	
	if (!firstNearBlock)
	{
#ifdef _DEBUG
		PAUSE(("Heap %s firstNearBlock is NULL.",heapName));
#endif
		return 0;
	}

#ifdef SAFE_HEAP
	__asm
	{
		cmp		localHeapState,0
		jne		error1
	}
#endif

	__asm
	{
		xor     eax,eax
		mov		edi,10000
		mov		ecx,heapBlockSize
		mov     ebx,localFirst
		mov     edx,ebx						//edx = is place holder for first node
	}
	
BytesLoop:

	__asm
	{
		mov     ebx,[ebx].next
		add     eax,[ebx].blockSize
		add		eax,ecx
		dec		edi
		je		short error1
		cmp     ebx,edx
		jne     short BytesLoop
		jmp		short DoneTC
	}
	
error1:

	__asm
	{
		xor		eax,eax
	}
	
DoneTC:

	__asm
	{
		mov		result,eax
	}

#endif // USE_GOS_HEAP
	return(result);
}

//---------------------------------------------------------------------------
unsigned long UserHeap::coreLeft (void)
{
	unsigned long result = 0;

	if (gosHeap)
		return result;

#ifndef USE_GOS_HEAP

#ifdef SAFE_HEAP
	long localHeapState = heapState;
#endif

	HeapBlockPtr localFirst = firstNearBlock;
	long heapBlockSize = -(long)allocatedBlockSize;
	
#ifdef USE_BEST_FIT

	__asm
	{
		xor		eax,eax
	}
	
#ifdef SAFE_HEAP

	__asm
	{
		cmp		localHeapState,0
		jne		short DoneCL
	}
	
#endif

	__asm
	{
		mov     ebx,localFirst
		or		ebx,ebx
		je		short DoneCL
		mov		ebx,[ebx].previous
		mov		eax,[ebx].blockSize   			// size of last block in list
		add		eax,heapBlockSize
	}
	
DoneCL:

	__asm
	{
		mov		result, eax
	}
	
	return(result);
	
#else   													// !DEFINED BEST_FIT

	__asm
	{
		xor		eax,eax
	}
	
#ifdef SAFE_HEAP

	__asm
	{
		cmp		localHeapState,0
		jne		short DoneCL
	}
	
#endif

	__asm
	{
		mov     ebx,localFirst
		or		ebx,ebx
		je		short DoneCL
		mov		ecx,10000
	}
	
TopLoop:

	__asm
	{
		cmp		[ebx].blockSize,eax
		jb		Next
		mov		eax,[ebx].blockSize
	}
	
Next:

	__asm
	{
		dec		ecx
		jle		DoneCL
		mov		ebx,[ebx].next
		cmp		ebx,localFirst
		jne		TopLoop
		add		eax,heapBlockSize
	}
	
DoneCL:
	__asm
	{
		mov		result, eax
	}
	
	
#endif

#endif // USE_GOS_HEAP
	return(result);
}			

//---------------------------------------------------------------------------
void * UserHeap::Malloc (size_t memSize)
{
	void * result = NULL;
	if (gosHeap)
	{
		gos_PushCurrentHeap( gosHeap );
		result = gos_Malloc(memSize);
		gos_PopCurrentHeap();

		return result;
	}
#ifndef USE_GOS_HEAP

	HeapBlockPtr blockOffs = NULL;
	HeapBlockPtr localFirst = firstNearBlock;
	long heapBlockSize = sizeof(HeapBlock);
	bool mf = mallocFatals;
	
	#ifdef _DEBUG
	heapBlockSize += 4;
	#endif
	
	long errorResult = 0;

	__asm
	{
		mov     eax,memSize
		or	    eax,eax
		je	    Alloc_zero
	}
		
#ifdef _DEBUG
	__asm
	{
		add		eax,4							// extra space for who made me pointer
	}
	
#endif

	__asm
	{
		add     eax,11						//force minimum allocation 8+3
		and 	al, 0xfc //NOT 3						//force dword alignment
		cmp		eax,heapBlockSize
		jae		short SizeDone
		mov		eax,heapBlockSize
	}
	
SizeDone:

	__asm
	{
		cmp		eax,memSize
		jb      Alloc_Overflow

		//search free list for first available block

		mov     ebx,localFirst
		or	    ebx,ebx
		je	    Zero_Free
		mov		edx,ebx	       			//marker to beginning of list
	}
	
SearchLoop:

	__asm
	{
		mov     ecx,[ebx].blockSize
		sub     ecx,eax						//unsigned math
		jnb     short FoundBlock
		mov     ebx,[ebx].next
		cmp     edx,ebx
		jne     SearchLoop					//have we come back to first node?

		//else block was not found
		
		add		ecx,eax						// ECX = biggest block
		mov		edx,[memSize]				// EDX = bytes asked for
     	mov		eax,OUT_OF_MEMORY			// EAX = Out of memory error number
		jmp		Alloc_error
	}
	
FoundBlock:

		// ebx = offset of found block
		// ecx = size of leftover block
		// eax = size of block to allocate

	__asm
	{
		mov		blockOffs,ebx
		cmp 	ecx,heapBlockSize				//any memory left to reallocate?
		jae		short UnlinkNormal
		or		[ebx].blockSize,1				//mark allocated
	}

	//This code is the unlink macro.
	__asm
	{
		cmp		[ebx].next,ebx
		jne		short ULine1
		
		//else list is now empty
		
		mov		localFirst,0
    	jmp		short ULine3
	}
	
ULine1:

	__asm
	{
		mov     edx,localFirst
		cmp     ebx,edx
		jne     short ULine2					//unlinking first element?
		mov     eax,[ebx].next
		mov     localFirst,eax
	}
	
ULine2:

	__asm
	{
		mov		edi,[ebx].next					//edi = ebx.next
		mov		ebx,[ebx].previous
		mov		[edi].previous,ebx   		//ebx.next.prev = ebx.prev
		mov		[ebx].next,edi					//ebx.prev.next = ebx.next
	}
	
ULine3:											//End of Unlink code

	__asm
	{
		mov		eax,blockOffs
		jmp		short Alloc_Done
	}
	

UnlinkNormal:

	__asm
	{
		mov		edi,ebx
		add		edi,[ebx].blockSize			//edi -> lower block
		add		[edi].upperBlock,ecx			//update lower pointer to the new block

		mov		[ebx].blockSize,ecx
		mov		edi,ebx
		add		edi,ecx							//edi -> newblock
		or		al,1								//mark new block as allocated
		mov		[edi].blockSize,eax
		mov		[edi].upperBlock,ebx
	}
	

	//-------------------------------------------------------------------------------------
	//SORT Routine
	
#ifdef USE_BEST_FIT
	__asm
	{
		pushad
	}
		
	//-------------------------------------
	//see if we have to do any work at all
	HeapBlockPtr localFirstSort = firstNearBlock;
	__asm
	{
		mov		edx,localFirstSort
		mov		ecx,[ebx].blockSize
		mov		edi,[ebx].next

		cmp		edi,[ebx].previous
		jne		short __Line1		//either 1 or two members in list
		cmp		edi,ebx
		je		__Done				//only one member in list

									//else there are only two

		mov		localFirstSort,ebx		//assume we are the smaller block
		cmp		ecx,[edi].blockSize
		jbe		__Done				//we were right
		mov		localFirstSort,edi		//else other guy is smaller
		jmp		__Done
	}

__Line1:  							// else see if we are not in the correct order

	__asm
	{
		cmp		ecx,[edi].blockSize
		jbe		short __Line2	

									//else see if next guy in line is the localFirst

		cmp		edi,edx				//did we just compare with Beginning of list?
		jne		short __Line3		//no
	}
	
__Line2:

	__asm
	{
		mov		edi,[ebx].previous
		cmp	    ecx,[edi].blockSize
		jae		short __Done

									//else we are less than guy to our left

		cmp		ebx,edx				//are we the first block in list?
		je		short __Done

__Line3: 							//else we must unlink our block, saving a pointer to lower neighbor

		push	edi
		push	ebx

		//Unlink Routine inline here
			
		cmp		[ebx].next,ebx
		jne		short _ULine1
		
	  								 //else list is now empty
		
		mov		localFirstSort,0
    	jmp		short _ULine3
	}
	
_ULine1:

	__asm
	{
		mov     edx,localFirstSort
		cmp     ebx,edx
		jne     short _ULine2					//unlinking first element?
		mov     eax,[ebx].next
		mov     localFirstSort,eax
	}
	
_ULine2:

	__asm
	{
		mov		edi,[ebx].next				  	//edi = ebx.next
		mov		ebx,[ebx].previous
		mov		[edi].previous,ebx   			//ebx.next.prev = ebx.prev
		mov		[ebx].next,edi					//ebx.prev.next = ebx.next
	}
	
_ULine3:

	__asm
	{
												// ecx = unchanged, the size of our block
		pop		ebx								// ebx -> our block we have just unlinked.
		pop		edi								// edi -> block to start our search on.

		mov 	edx,localFirstSort

		cmp		[ebx].previous,edi
		je		short __Line4					// search previous nodes
	}

												// stop when guy to the right is higher than us, or is the FirstMemBlock
__Loop1:

	__asm
	{
		mov		esi,edi
		mov		edi,[edi].next
		cmp		edi,edx							//have we wrapped around?
		je		short __FoundPlace
		cmp		[edi].blockSize,ecx
		jae		short __FoundPlace
		jmp		__Loop1
	}

__Line4:
												//stop when guy to the left is lower than us, or we stopped on the FirstMemBlock
__Loop2:

	__asm
	{
		mov		esi,edi
		mov		edi,[edi].previous
		cmp		esi,edx							//have we wrapped around?
		je		short __Line5
		cmp		[edi].blockSize,ecx
		ja		__Loop2
	}
	
__Line5:

	__asm
	{
		xchg		esi,edi
	}
	
__FoundPlace:									// esi-> first block, edi->second block, ebx -> us
												//ebx = offset of new block
	__asm
	{
		mov		[ebx].previous,esi
		mov		[ebx].next,edi
		mov		[esi].next,ebx
		mov		[edi].previous,ebx

		mov		edi,edx
		cmp		ecx,[edi].blockSize				//see if we are now smallest
		jae		short __Done
		mov		localFirstSort,ebx					//we are smallest
	}
	
__Done:

	firstNearBlock = localFirstSort;
	//-------------------------------------------------------------------------------------
	//SORT Routine
	
	__asm
	{
		POPAD
	}
	
#endif

	__asm
	{
		mov		eax,edi
		jmp		short Alloc_Done
	}
	

//-----------------------------------------error handling

Alloc_zero:

	__asm
	{
    	mov    	eax,ALLOC_ZERO
    	jmp    	short Alloc_error
	}
	
Zero_Free:

	__asm
	{
    	mov    	eax,NULL_FREE_LIST
    	jmp    	short Alloc_error
	}
	
Alloc_Overflow:

	__asm
	{
    	mov     eax,ALLOC_OVERFLOW
	}
	

Alloc_error:

	__asm
	{
		mov 	errorResult,eax
		cmp		mf,0
//		cmp		[this].mallocFatals,0

#ifdef CHECK_HEAP
		je		noFatal
#else
		jmp		noFatal
#endif


	}

	memCoreLeft = totalCoreLeft();
	memTotalLeft = coreLeft();
	
	walkHeap(TRUE,FALSE);
	if (memSize)
		STOP(("Heap %s is Out Of RAM.  HeapSize %d, CoreLeft %d, TotalLeft %d, SizeTried %d",heapName,heapSize,memCoreLeft,memTotalLeft,memSize));
	else
		STOP(("Heap %s Tried to Malloc Zero Bytes!"));
	
noFatal:

	__asm
	{
		mov		eax,-8
#ifdef _DEBUG
		mov		eax,-12		//Make that extra room for who made me
#endif
	}
	
Alloc_Done:

	__asm
	{
		add		eax,8			//skip over header
#ifdef _DEBUG
		add		eax,4			//Skip over extra for debugging
#endif
	}
	
#ifdef _DEBUG
	__asm
	{
		//if we allocated memory, store pointer to caller
		or		eax,eax
		je		NoAllocation
		mov		ebx,ebp
		mov		edx,dword ptr [ebx+4]	// get caller
		mov		[eax-4],edx					//Store caller in HeapBlock
	}
	
NoAllocation:
#endif

#ifdef HEAP_CHECK

	__asm
	{
		push 	eax
	}
	
	WalkHeap();
	
	__asm
	{
		pop 	eax
	}
	
#endif

	__asm
	{
		mov 	result,eax
	}

	firstNearBlock = localFirst;
	
	#ifdef _DEBUG
	if (logMallocs)
	{
		recordCount++;

		gosASSERT (recordCount<NUMMEMRECORDS);
		
		recordArray[recordCount].ptr = result;
		recordArray[recordCount].size = memSize;

		CONTEXT ourContext;
		STACKFRAME sf;
		GetCurrentContext(&ourContext);
		InitStackWalk(&sf,&ourContext);
		
		for (long i=0;i<12;i++)
		{
			recordArray[recordCount].stack[i] = WalkStack(&sf);
		}
	}
	#endif
	
#endif // USE_GOS_HEAP
	return(result);
}

//---------------------------------------------------------------------------
long UserHeap::Free (void *memBlock)
{
	if (gosHeap)
	{
		gos_PushCurrentHeap( gosHeap );
		gos_Free(memBlock);
		gos_PopCurrentHeap();

		return 0;
	}

	long result = 0;
#ifndef USE_GOS_HEAP
	HeapBlockPtr blockOffs = (HeapBlockPtr)memBlock;
	HeapBlockPtr sortBlock = NULL;

	//------------------------------------------
	// If freeing a NULL, we do nothing
	//------------------------------------------
	if (memBlock == NULL)
		return(NO_ERR);

	//-------------------------------------------------------------------
	// this is bad.  However, for release, just let it go on the really
	// remote chance it happens.
	if ((memBlock < getHeapPtr()) || (memBlock >= (getHeapPtr() + totalSize)))
	{
#ifdef _DEBUG
		PAUSE(("Tried to delete a bad pointer."));
#endif

		return (NO_ERR);
	}

	__asm
	{
		cmp		blockOffs,0
		je		Dealloc_Done

		sub		blockOffs,8   					//compensate for heading

#ifdef _DEBUG
		sub		blockOffs,4	 					//extra return pointer
#endif
		
	}

	//merge this block with lower one if possible
	long mergeResult = mergeWithLower(blockOffs);

	__asm
	{
		mov 	eax,mergeResult

#ifdef SAFE_HEAP
		or		eax,eax		   				//return 0 on error
		je      Dealloc_Done
#endif

		//if block above is "free", add our size to it. no relinking is needed

		mov     ebx,[blockOffs]
		mov     edi,[ebx].upperBlock
		or	    edi,edi
		je	    short Relink_needed			//no block above this one

		test    [edi].blockSize,1
		jne     short Relink_needed			//block above is allocated

		//else just add size

		mov     eax,[ebx].blockSize
		and		eax,0xfffffffe	//NOT 1
		add     [edi].blockSize,eax			//add to size of above block

		//inform new lower neighbor about the change

		mov		ebx,edi
		add		ebx,[edi].blockSize
		mov		[ebx].upperBlock,edi

#ifdef USE_BEST_FIT
		mov		ebx,edi
	}

	//-------------------------------------------------------------------------------------
	//SORT Routine
	__asm
	{
		pushad
	}
		
	//-------------------------------------
	//see if we have to do any work at all
	HeapBlockPtr localFirstSort = firstNearBlock;
	__asm
	{
		mov		edx,localFirstSort
		mov		ecx,[ebx].blockSize
		mov		edi,[ebx].next

		cmp		edi,[ebx].previous
		jne		short __Line1		//either 1 or two members in list
		cmp		edi,ebx
		je		__Done				//only one member in list

									//else there are only two

		mov		localFirstSort,ebx		//assume we are the smaller block
		cmp		ecx,[edi].blockSize
		jbe		__Done				//we were right
		mov		localFirstSort,edi		//else other guy is smaller
		jmp		__Done
	}

__Line1:  							// else see if we are not in the correct order

	__asm
	{
		cmp		ecx,[edi].blockSize
		jbe		short __Line2	

									//else see if next guy in line is the localFirst

		cmp		edi,edx				//did we just compare with Beginning of list?
		jne		short __Line3		//no
	}
	
__Line2:

	__asm
	{
		mov		edi,[ebx].previous
		cmp	    ecx,[edi].blockSize
		jae		short __Done

									//else we are less than guy to our left

		cmp		ebx,edx				//are we the first block in list?
		je		short __Done

__Line3: 							//else we must unlink our block, saving a pointer to lower neighbor

		push	edi
		push	ebx

		//Unlink Routine inline here
			
		cmp		[ebx].next,ebx
		jne		short _ULine1
		
	  								 //else list is now empty
		
		mov		localFirstSort,0
    	jmp		short _ULine3
	}
	
_ULine1:

	__asm
	{
		mov     edx,localFirstSort
		cmp     ebx,edx
		jne     short _ULine2					//unlinking first element?
		mov     eax,[ebx].next
		mov     localFirstSort,eax
	}
	
_ULine2:

	__asm
	{
		mov		edi,[ebx].next				  	//edi = ebx.next
		mov		ebx,[ebx].previous
		mov		[edi].previous,ebx   			//ebx.next.prev = ebx.prev
		mov		[ebx].next,edi					//ebx.prev.next = ebx.next
	}
	
_ULine3:

	__asm
	{
												// ecx = unchanged, the size of our block
		pop		ebx								// ebx -> our block we have just unlinked.
		pop		edi								// edi -> block to start our search on.

		mov 	edx,localFirstSort

		cmp		[ebx].previous,edi
		je		short __Line4					// search previous nodes
	}

												// stop when guy to the right is higher than us, or is the FirstMemBlock
__Loop1:

	__asm
	{
		mov		esi,edi
		mov		edi,[edi].next
		cmp		edi,edx							//have we wrapped around?
		je		short __FoundPlace
		cmp		[edi].blockSize,ecx
		jae		short __FoundPlace
		jmp		__Loop1
	}

__Line4:
												//stop when guy to the left is lower than us, or we stopped on the FirstMemBlock
__Loop2:

	__asm
	{
		mov		esi,edi
		mov		edi,[edi].previous
		cmp		esi,edx							//have we wrapped around?
		je		short __Line5
		cmp		[edi].blockSize,ecx
		ja		__Loop2
	}
	
__Line5:

	__asm
	{
		xchg		esi,edi
	}
	
__FoundPlace:									// esi-> first block, edi->second block, ebx -> us
												//ebx = offset of new block
	__asm
	{
		mov		[ebx].previous,esi
		mov		[ebx].next,edi
		mov		[esi].next,ebx
		mov		[edi].previous,ebx

		mov		edi,edx
		cmp		ecx,[edi].blockSize				//see if we are now smallest
		jae		short __Done
		mov		localFirstSort,ebx					//we are smallest
	}
	
__Done:

	firstNearBlock = localFirstSort;
	//-------------------------------------------------------------------------------------
	//SORT Routine
	
	__asm
	{
		POPAD
	}
	
	__asm
	{
#endif
		jmp     short Dealloc_Done
	}

Relink_needed:

	__asm
	{
		mov     ebx,[blockOffs]
		and		[ebx].blockSize,0xfffffffe	//NOT 1
		mov		sortBlock,ebx
	}

	relink(sortBlock);

Dealloc_Done:

#ifdef HEAP_CHECK
		walkHeap(FALSE,FALSE," Free Heap Ck ");
#endif

	__asm
	{
		mov		result,eax
	}

#ifdef _DEBUG
	if (logMallocs)
	{
		long count = 0;
		while (count<NUMMEMRECORDS && recordArray[count].ptr != memBlock)
			count++;

		//This may be OK?  Not logging when allocated!
		//gosASSERT (count < NUMMEMRECORDS);		
		
		recordArray[count].ptr = NULL;
		recordArray[count].size = 0;
		
		for (int i=0; i<12; i++)
			recordArray[count].stack[i] = 0;
	}
#endif
	
#endif // USE_GOS_HEAP

	return(result);
}

//---------------------------------------------------------------------------
void * UserHeap::calloc (size_t memSize)
{
	void * result = malloc(memSize);
	memset(result,0,memSize);

	return result;
}
		
//---------------------------------------------------------------------------
void UserHeap::walkHeap (bool printIt, bool skipAllocated)
{
	if (gosHeap)
	{
		gos_WalkMemoryHeap(gosHeap);
		return;
	}
#ifndef USE_GOS_HEAP
	HeapBlockPtr walker = heapStart;
	bool valid, allocated;
	unsigned long bSize;

	if (!walker || (heapState != NO_ERR))
		return;

	#ifdef _DEBUG
	File logFile;
	logFile.create("walkdump.log");
	#endif
	
	while (walker->blockSize != 1)
	{
		//--------------------
		// check for validity
		//--------------------
		valid = TRUE;

		if (walker->upperBlock)
		{
			bSize = (walker->upperBlock->blockSize & ~1);
			bSize += (unsigned long)walker->upperBlock;
			valid = (bSize == (unsigned long)walker);
		}

		if (valid)
		{
			bSize = (unsigned long)walker + (walker->blockSize & ~1);
			valid = (HeapBlockPtr(bSize)->upperBlock == walker);
		}
		else
		{
			//--------------------------
			//	Failed upper block check
			//--------------------------
			heapState = HEAP_CORRUPTED;
			STOP(("Heap %s Upper Block Check Failed",heapName));
			return;
		}

		if (valid && !(walker->blockSize & 1)) // if free block
		{
			valid = ((walker->previous->next == walker) &&
					 (walker->next->previous == walker));
		}
		else if (!valid)
		{
			//--------------------------
			// Failed lower block check
			//--------------------------
			heapState = HEAP_CORRUPTED;
			STOP(("Heap %s Lower Block Check Failed",heapName));
			return;
		}

		if (!valid)
		{
			//--------------------------
			//	Failed linked list check
			//--------------------------
			heapState = HEAP_CORRUPTED;
			STOP(("Heap %s LinkedList Check Failed",heapName));
			return;
		}

		allocated = walker->blockSize & 1;
		if ((printIt && !allocated) || (printIt && !skipAllocated))
		{
			char errMessage[256];

			#ifdef _DEBUG
			if (allocated)
			{
				sprintf(errMessage, "Allocated block at DS:%08X, size = %u, owner at CS:%08X\n",
						walker,(walker->blockSize & ~1),walker->previous);
					
				//--------------------------------------------------------------------------------------------
				// A size and/or address check can be put here to inspect the block and determine other info	
				long magicNumber = 0x6726FB;
				if ((walker->previous) == (void*)magicNumber)
					printf("magicNumber");
				//--------------------------------------------------------------------------------------------
			}
			else
			{
				sprintf(errMessage, "Free block at DS:%08X, size = %u \n", walker,(walker->blockSize & ~1));
			}

			#ifndef _CONSOLE
			logFile.writeLine(errMessage);
			#else
			printf(errMessage);
			#endif

			#else
			sprintf(errMessage, "%s block at DS:%08X, size = %u \n",
					(allocated)?"Allocated":"Free",walker,(walker->blockSize & ~1));

			#ifndef _CONSOLE
			OutputDebugString(errMessage);
			#else
			printf(errMessage);
			#endif

			#endif
		}

		walker = HeapBlockPtr((unsigned long)walker + (walker->blockSize & ~1));
	}

#endif // USE_GOS_HEAP
}

//---------------------------------------------------------------------------
long UserHeap::getLastError (void)
{
	return heapState;
}
#ifndef USE_GOS_HEAP

//---------------------------------------------------------------------------
void UserHeap::relink (HeapBlockPtr newBlock)
{
	HeapBlockPtr localFirst = firstNearBlock;
	
	//empty list?
	__asm
	{
		mov		ebx, newBlock
		cmp		localFirst,0
    	jne		short Line1
		
		//else this is the only block in the list
		
		mov     localFirst,ebx
		mov     [ebx].previous,ebx
		mov     [ebx].next,ebx
		jmp     short Line2
	}
	
Line1:

	__asm
	{
		//ebx = address of new block
		mov     edi,localFirst

#ifdef USE_BEST_FIT
		mov		edx,edi
		mov		ecx,[ebx].blockSize
		cmp		ecx,[edi].blockSize
		jae		short Line3
		mov		localFirst,ebx
		jmp		short Line4
	}
	
Line3:

	__asm
	{
		mov		edi,[edi].next
		cmp		edi,edx
		je		short Line4
		cmp		ecx,[edi].blockSize
		ja		Line3
	}
	
Line4:

	__asm
	{
		mov		edi,[edi].previous
#endif

		mov		eax,[edi].next
		mov		[ebx].next,eax
		mov		[ebx].previous,edi
		mov		[edi].next,ebx
		mov		edi,[ebx].next
		mov		[edi].previous,ebx
	}
	
Line2:

	firstNearBlock = localFirst;
}

//---------------------------------------------------------------------------
void UserHeap::unlink (HeapBlockPtr oldBlock)
{
	HeapBlockPtr localFirst = firstNearBlock;
	
	__asm
	{
		mov 	ebx,oldBlock
		cmp		[ebx].next,ebx
		jne		short ULine1
		
	   //else list is now empty
		
		mov		localFirst,0
    	jmp		short ULine3
	}
	
ULine1:

	__asm
	{
		mov     edx,localFirst
		cmp     ebx,edx
		jne     short ULine2					//unlinking first element?
		mov     eax,[ebx].next
		mov     localFirst,eax
	}
	
ULine2:

	__asm
	{
		mov		edi,[ebx].next					//edi = ebx.next
		mov		ebx,[ebx].previous
		mov		[edi].previous,ebx   		//ebx.next.prev = ebx.prev
		mov		[ebx].next,edi					//ebx.prev.next = ebx.next
	}
	
ULine3:

	firstNearBlock = localFirst;
}

//---------------------------------------------------------------------------
bool UserHeap::mergeWithLower (HeapBlockPtr block)
{
	HeapBlockPtr localFirst = firstNearBlock;
	bool result = FALSE;
	
	__asm
	{
		mov     edi,block
		mov     ebx,[edi].blockSize
	}
	
#ifdef SAFE_HEAP

	__asm
	{
		test	ebx,1
		je	    short Fatal1	      			//else failed alloc check
		cmp		[edi+ebx-1].upperBlock,edi
		jne		short Fatal1                 	//else failed lower check
		mov		esi,[edi].upperBlock
		or		esi,esi
		je		short check_ok
		add		esi,[esi].blockSize
		and		esi,0xfffffffe	//NOT 1
		cmp		esi,edi
		je		short check_ok
	}
	
Fatal1:

	walkHeap(FALSE,FALSE);
	//Fatal(HEAP_CORRUPTED);

	__asm
	{
		xor		eax,eax			//In theory, this will never execute since the Fatal Exits
		jmp		DoneML			//Maybe if Fatal is not operating?
	}
	
check_ok:
#endif

	__asm
	{
		and     ebx,0xfffffffe	//NOT 1					//make size be acurate
		mov     esi,ebx
		add     esi,edi		    			//esi = offset of lower neighbor
		mov     ecx,[esi].blockSize
		test    ecx,1
		jne     short NoMerge	    		//lower block is allocated
		add     ebx,ecx		    			//ebx = new size of block
		mov     [edi].blockSize,ebx
		or		[edi].blockSize,1   		//dealloc() expects allocated block
		mov		eax,edi
		add		ebx,eax						//ebx -> new lower neigbor
		mov		[ebx].upperBlock,edi		//inform new neighbor about the change
		mov     ebx,esi		    			//ebx = offset of old lower neighbor
	}

	//This is the UNLINK routine directly.	
	__asm
	{
		cmp		[ebx].next,ebx
		jne		short ULine1
		
	   //else list is now empty
							
		mov     localFirst,0
    	jmp     short ULine3
	}
	
ULine1:

	__asm 
	{
		mov     edx,localFirst
		cmp     ebx,edx
		jne     short ULine2						//unlinking first element?
		mov     eax,[ebx].next
		mov     localFirst,eax
	}
	
ULine2:

	__asm
	{
		mov		edi,[ebx].next				//edi = bx.next
		mov		ebx,[ebx].previous
		mov		[edi].previous,ebx   	//ebx.next.prev = ebx.prev
		mov		[ebx].next,edi				//ebx.prev.next = bx.next
	}
	
ULine3:
NoMerge:

	__asm
	{
		mov		eax,1							//Return TRUE
	}
	
#ifdef SAFE_HEAP
DoneML:
#endif

	__asm
	{
		mov		result,al						//Move eax into result
	}
	

	firstNearBlock = localFirst;

	return(result);
}

#endif // USE_GOS_HEAP
		
//---------------------------------------------------------------------------
void HeapList::addHeap (HeapManagerPtr newHeap)
{
	for (long i=0;i<MAX_HEAPS;i++)
	{
		if (heapRecords[i].thisHeap == NULL)
		{
			heapRecords[i].thisHeap = newHeap;
			heapRecords[i].heapSize = newHeap->tSize();
			return;
		}
	}
}

//---------------------------------------------------------------------------
void HeapList::removeHeap (HeapManagerPtr oldHeap)
{
	for (long i=0;i<MAX_HEAPS;i++)
	{
		if (heapRecords[i].thisHeap == oldHeap)
		{
			heapRecords[i].thisHeap = NULL;
			heapRecords[i].heapSize = 0;
			return;
		}
	}
}

void HeapList::initializeStatistics()
{
	if (heapInstrumented == 0)
	{
			StatisticFormat( "" );
			StatisticFormat( "MechCommander 2 Heaps" );
			StatisticFormat( "======================" );
			StatisticFormat( "" );

			AddStatistic("Total Memory","bytes",gos_DWORD, &(totalSize), Stat_AutoReset | Stat_Total);

			AddStatistic("Total Memory Core Left","bytes",gos_DWORD, &(totalCoreLeft), Stat_AutoReset | Stat_Total);

			AddStatistic("Total Memory Left","bytes",gos_DWORD, &(totalLeft), Stat_AutoReset | Stat_Total);

			StatisticFormat( "" );
			StatisticFormat( "" );
	
			for (long i=0;i<50;i++)
			{
				char heapString[255];
				sprintf(heapString,"Heap %d - HeapSize",i);
				AddStatistic(heapString,"bytes",gos_DWORD, &(heapRecords[i].heapSize), Stat_AutoReset | Stat_Total);

				sprintf(heapString,"Heap %d - TotalLeft",i);
				AddStatistic(heapString,"bytes",gos_DWORD, &(heapRecords[i].totalCoreLeft), Stat_AutoReset | Stat_Total);

				sprintf(heapString,"Heap %d - CoreLeft",i);
				AddStatistic(heapString,"bytes",gos_DWORD, &(heapRecords[i].coreLeft), Stat_AutoReset | Stat_Total);

				StatisticFormat( "" );
			}

		heapInstrumented = true;
	}
}
	
//---------------------------------------------------------------------------
void HeapList::update (void)
{
	totalSize = totalCoreLeft = totalLeft = 0;
	for (long i=0;i<50;i++)
	{
		if (heapRecords[i].thisHeap && (heapRecords[i].thisHeap->heapType() == USER_HEAP))
		{
			heapRecords[i].heapSize = ((UserHeapPtr)heapRecords[i].thisHeap)->tSize();
			totalSize += heapRecords[i].heapSize;

			heapRecords[i].coreLeft = ((UserHeapPtr)heapRecords[i].thisHeap)->coreLeft();
			totalLeft += heapRecords[i].coreLeft;

			heapRecords[i].totalCoreLeft = ((UserHeapPtr)heapRecords[i].thisHeap)->totalCoreLeft();
			totalCoreLeft += heapRecords[i].totalCoreLeft;
		}
		else if (heapRecords[i].thisHeap)
		{
			heapRecords[i].heapSize = heapRecords[i].thisHeap->tSize();
			totalSize += heapRecords[i].heapSize;

			heapRecords[i].coreLeft = 0;
			heapRecords[i].totalCoreLeft = 0;
		}
	}
}

//---------------------------------------------------------------------------
unsigned long textToLong (char *num)
{
	long result = 0;
	char *hexOffset = num;
	
	hexOffset += 2;
	long numDigits = strlen(hexOffset)-1;
	long power = 0;
	
	for (long count = numDigits;count >= 0;count--,power++)
	{
		unsigned char currentDigit = toupper(hexOffset[count]);
		
		if (currentDigit >= 'A' && currentDigit <= 'F')
		{
			result += (currentDigit - 'A' + 10)<<(4*power);
		}
		else if (currentDigit >= '0' && currentDigit <= '9')
		{
			result += (currentDigit - '0')<<(4*power);
		}
		else
		{
			//---------------------------------------------------------
			// There is a digit in here I don't understand.  Return 0.
			result = 0;
			break;
		}
	}

	return(result);
}

//-----------------------------------------------------------
long longToText (char *result, long num, unsigned long bufLen)
{
	char temp[250];
	sprintf(temp,"%08X",num);

	unsigned long numLength = strlen(temp);
	if (numLength >= bufLen)
		return(0);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERR);
}	

//--------------------------------------------------------------------------
long getStringFromMap (File &mapFile, unsigned long addr, char *result)
{
	//----------------------------------------
	// Convert function address to raw offset
	#ifdef TERRAINEDIT
	unsigned long offsetAdd = 0x00601000;
	#else
	unsigned long offsetAdd = 0x00601000;
	#endif
	
	unsigned long function = addr;
	function -= offsetAdd;
	
	char actualAddr[10];
	longToText(actualAddr,function,9);
	
	//------------------------------------
	// Find the first code entry address.
	// This is the first line encountered with "  Address" as the first nine characters.
	char mapFileLine[512];
	
	mapFile.seek(0);
	mapFile.readLine((MemoryPtr)mapFileLine,511);
	while (strstr(mapFileLine,"  Address") == NULL)
	{
		mapFile.readLine((MemoryPtr)mapFileLine,511);
	}
	
	mapFile.readLine((MemoryPtr)mapFileLine,511);
	mapFile.readLine((MemoryPtr)mapFileLine,511);
	//-------------------------------------------------------------
	// We've found the first code entry.  Now, scan until
	// the current address is greater than the address asked for.
	// The previous function name is the function in question.
	char *currentAddress = &(mapFileLine[6]);
	char previousAddress[511] = { 0 };
	strncpy(previousAddress,&(mapFileLine[6]),510);
	
	while (strstr(mapFileLine,"0001:") != NULL)
	{
		if (S_strnicmp(currentAddress,actualAddr,8) > 0)
		{
			//-----------------------------------------------
			// We've found it, print the previous address.
			strncpy(result,previousAddress,510);
			return(strlen(result));
		}
		
		strncpy(previousAddress,&(mapFileLine[6]),510);
		mapFile.readLine((MemoryPtr)mapFileLine,511);
	}
	
	return(0);
}
		
//---------------------------------------------------------------------------
void HeapList::dumpLog (void)
{
	//----------------------------------------------
	// This function dumps information on each heap
	// to a log file.
	File logFile;
	logFile.create("heap.dump.log");
	
	File mapFile;
	
	long mapResult = 0;
	#ifdef _DEBUG
	#ifdef TERRAINEDIT
	mapResult = mapFile.open("teditor.map");
	#else
	mapResult = mapFile.open("mechcmdrdbg.map");
	#endif
	#endif	

	HeapManagerPtr currentHeap = NULL;
	unsigned long heapNumber = 1;
	unsigned long mapStringSize = 0;
	char msg[1024];
	char mapInfo[513];

	unsigned long totalCommit = 0;
	unsigned long totalFree = 0;
	
	for (long i=0;i<MAX_HEAPS;i++)
	{
		currentHeap = heapRecords[i].thisHeap;
		
		if (currentHeap)
		{
			sprintf(msg,"ListNo: %d     Heap: %d     Type: %d     Made by: %08ld",i,heapNumber,currentHeap->heapType(),currentHeap->owner());
			logFile.writeLine(msg);

			if (mapResult == NO_ERR)
			{
				mapStringSize = getStringFromMap(mapFile,currentHeap->owner(),mapInfo);
				if (mapStringSize)
				{
					sprintf(msg,"Made in Function : %s",mapInfo);
					logFile.writeLine(msg);
				}
			}
			
			sprintf(msg,"HeapSize: %d     HeapStart: %08p",currentHeap->tSize(),currentHeap->getHeapPtr());
			logFile.writeLine(msg);

			totalCommit += currentHeap->tSize();
			
			if (currentHeap->heapType() == 1)
			{
				UserHeapPtr userHeap = (UserHeapPtr)currentHeap;
				sprintf(msg,"TotalCoreLeft: %d     CoreLeft: %d",userHeap->totalCoreLeft(),userHeap->coreLeft());
				logFile.writeLine(msg);
				
				sprintf(msg,"Frag Level: %f       PercentFree: %f",(float(userHeap->coreLeft())/float(userHeap->totalCoreLeft())),1.0 - (float(currentHeap->tSize()-userHeap->coreLeft())/float(currentHeap->tSize())) );
				logFile.writeLine(msg);
				
				totalFree += userHeap->coreLeft();
			}
			
			currentHeap = currentHeap->nxt;
			heapNumber++;
		}
		else
		{
			sprintf(msg,"ListNo: %d  is Freed",i);
			logFile.writeLine(msg);
		}
		
		sprintf(msg,"---------------------------");
		logFile.writeLine(msg);
	}	
	
	sprintf(msg,"Total Committed Memory: %d      Total Free in Commit: %d",totalCommit,totalFree);
	logFile.writeLine(msg);
	sprintf(msg,"---------------------------");
	logFile.writeLine(msg);
		
	logFile.close();
}

bool UserHeap::pointerOnHeap (void *ptr)
{
    // sebi NB!
    ENTER_DEBUGGER;
#ifndef USE_GOS_HEAP
	if (IsBadReadPtr(getHeapPtr(),totalSize))
		return false;

	if ((ptr < getHeapPtr()) || (ptr >= (getHeapPtr() + totalSize)))
		return false;
#endif
	return true;
}

//---------------------------------------------------------------------------
//
// Edit Log
//
//---------------------------------------------------------------------------

