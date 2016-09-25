//--------------------------------------------------------------------------
// LZ Compress Routine
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef _MBCS
#include<gameos.hpp>

#ifndef HEAP_H
#include"heap.h"
#endif

#else
#include<assert.h>
#include<malloc.h>
#define gosASSERT assert
#define gos_Malloc malloc
#define gos_Free free
#endif

#include "zlib.h"

// 128K and more cold be faster
#define CHUNK 16384

//---------------------------------------------------------------------------
// Static Globals

#ifndef NULL
#define NULL			0
#endif

typedef unsigned char* MemoryPtr;

#ifndef LINUX_BUILD

//-----------------------------
//Used by Compressor Routine
MemoryPtr		LZCHashBuf = NULL;

unsigned long 	InBufferUpperLimit = 0;
unsigned long 	InBufferPos = 0;	
MemoryPtr		InBuffer = NULL;

unsigned long 	OutBufferPos = 0;
MemoryPtr 		OutBuffer = NULL;

unsigned long 	PrefixCode = 0;
unsigned long 	FreeCode = 0;
unsigned long 	MaxCode = 0;
unsigned long 	NBits = 0;
unsigned long 	BitOffset = 0;
unsigned char 	K = 0;

unsigned long	codeToWrite = 0;

#define MaxMax		4096
#define Clear		256
#define EOF			257
#define First_Free	258

struct Hash
{
	unsigned long hashFirst;
	unsigned long hashNext;
	unsigned char hashChar;
};



static unsigned char		tag_LZCHashBuf[sizeof(Hash) * MaxMax + 1024];
//-----------------------------

//-------------------------------------------------------------------------------
// LZ Compress Routine
// Takes a pointer to dest buffer, a pointer to source buffer and len of source.
// returns length of compressed image.
long LZCompress (MemoryPtr dest, MemoryPtr src, unsigned long srcLen)
{
	long result = 0;

	if (!LZCHashBuf)
	{
		/* allocating LZCHashBuf off a gos heap causes problems for applications that need
		to reset gos or its heaps*/
		LZCHashBuf = (MemoryPtr)&(tag_LZCHashBuf[0]);
	}
	
//Initialize:
	
	unsigned long clearSize = sizeof(Hash) * 256;

	__asm
	{
		mov		eax,[dest]
		mov		[OutBufferPos],eax
		mov		[OutBuffer],eax
		mov		eax,[src]
		mov		[InBufferPos],eax
		mov		[InBuffer],eax
		add		eax,[srcLen]
		mov		[InBufferUpperLimit],eax

		mov		[BitOffset],0         		//zero out BitOffset
		
		//call	InitTable
		mov		[NBits],9          			//Starting with 9 bit codes
		mov		[MaxCode],512      			//10 0000 0000 b
		mov		eax,-1              		//Marked as unused
		mov		ecx,[clearSize]				//Clear first 256 entries
		mov		edi,[LZCHashBuf]   			//Start at the start of buffer
		rep		stosb
		mov		[FreeCode],First_Free 		//Next code to use is first_free

		mov		eax,Clear             		//first byte in buffer or file is CLEAR
		
		//call	writeCode
		xor		edx,edx					//make sure the DL is CLEAR
		mov		edi,[OutBufferPos]   	//obtain destination address
		mov		ecx,[BitOffset]        	//get bitposition
		jecxz	save1
	}
		
shift1:

	__asm
	{
		shl		ax,1
		rcl		edx,1
		loop	shift1
		or		al,[edi]
	}
	
save1:

	__asm
	{
		stosw
		mov		al,dl
		stosb
		
		//AdvanceBuffer
		
		mov		ecx,[NBits]                 ;get number of bits to advance
		mov		eax,[BitOffset]             ;get low word of OutBuffer
		add		eax,ecx
		mov		cl,al
		shr		al,3
		add		[OutBufferPos],eax
		and		cl,7
		movzx	ecx,cl
		mov		[BitOffset],ecx

//-------------------------------------------------------------------------
// This is the main compression loop                                       
//-------------------------------------------------------------------------

		//call	ReadChar              		//get byte from source
		mov		esi,[InBufferPos]    		//get address
   		cmp		esi,[InBufferUpperLimit]	//Check to see if we are done
		cmc									//compliment carry
		jc		doneRC1

		lodsb                       		//load byte
		mov		[InBufferPos],esi
		clc
	}
	
doneRC1:
		
Make_Into_Code:

	__asm
	{
		movzx	eax,al                 		//turn char into code
	}
		
//-------------------------------------------------------------------------
Set_AX_Prefix:

	__asm
	{
		mov		[PrefixCode],eax       		//into prefix code
		
		//call	ReadChar              		//more...
		mov		esi,[InBufferPos]    		//get address
   		cmp		esi,[InBufferUpperLimit]	//Check to see if we are done
		cmc									//compliment carry
		jc		doneRC2

		lodsb                       		//load byte
		mov		[InBufferPos],esi
		clc
	}
		
doneRC2:

	__asm
	{
		jc		FoundEOF            		//No more input
		mov		[K],al                		//Save returned char
		mov		ebx,[PrefixCode]       		//check for this pair
		//call	LookUpCode            		//in the table

		//call	Index                      	//index into current table address
		lea		esi,[ebx*8]
		add		esi,ebx            			//EBX * 9
		add		esi,[LZCHashBuf]
		
		xor		edi,edi                     //flag destination
		cmp		[esi].hashFirst,-1 			//see if code is used
		je		short exit1            		//if == then CARRY = CLEAR, set it    
		inc		edi                        	//flag as used
		mov		ebx,[esi].hashFirst			//get prefix code
	}
		
lookloop:

	__asm
	{
		//call	Index                      	//translate prefix or table to index
		lea		esi,[ebx*8]
		add		esi,ebx            			//EBX * 9
		add		esi,[LZCHashBuf]
		
		cmp		[esi].hashChar,al    		//is the suffix the same? if yes
		jne		notSame          			//then we can compress this a
		clc                              	//little more by taking this prefix
		mov		eax,ebx                     //code and getting a new suffix
		jmp		short exit2               	//in a moment
	}

notSame:

	__asm
	{
		cmp		[esi].hashNext,-1 			//found a new pattern so exit
		je		exit1                    	//if == then CARRY = CLEAR, set it
		mov		ebx,[esi].hashNext 			//continue through chain to get to
		jmp		short lookloop             	//end of chain
	}
	
exit1:
	__asm
	{
		stc
	}
	
exit2:

	__asm
	{
		jnc		Set_AX_Prefix       		//new prefix in EAX
		
//------------------------------------------------------------------------

		//call	AddCode               		//Add to table
		mov		ebx,[FreeCode]         		//get the next available hash table code
		push	ebx
		or		edi,edi                 	//is this first use of prefix?
		je		short newprefix
		mov		[esi].hashNext,ebx 			//if not, then set next code dest
		jmp		short addcode
	}
		
newprefix:

	__asm
	{
		mov		[esi].hashFirst,ebx			//Mark first as used
	}
		
addcode:

	__asm
	{
		cmp		ebx,MaxMax                  //is this the last code?
		je		exitAC

		//call	Index                     	//create new entry
		lea		esi,[ebx*8]
		add		esi,ebx            			//EBX * 9
		add		esi,[LZCHashBuf]
			
		mov		[esi].hashFirst,-1 			//mark new entry as unused
		mov		[esi].hashNext,-1
		mov		[esi].hashChar,al  			//and save suffix
		inc		ebx                         //set up for next code available
	}
		
exitAC:

	__asm
	{
		mov	[FreeCode],ebx
		pop	ebx
	}

	__asm
	{
		push	ebx                    		//save new code
		mov		eax,[PrefixCode]       		//write the old prefix code
		
		//call	writeCode
		xor		edx,edx					//make sure the DL is CLEAR
		mov		edi,[OutBufferPos]   	//obtain destination address
		mov		ecx,[BitOffset]        	//get bitposition
		jecxz	save5
	}
		
shift5:

	__asm
	{
		shl		ax,1
		rcl		edx,1
		loop	shift5
		or		al,[edi]
	}
	
save5:

	__asm
	{
		stosw
		mov		al,dl
		stosb
		
		//AdvanceBuffer
		
		mov		ecx,[NBits]                 ;get number of bits to advance
		mov		eax,[BitOffset]             ;get low word of OutBuffer
		add		eax,ecx
		mov		cl,al
		shr		al,3
		add		[OutBufferPos],eax
		and		cl,7
		movzx	ecx,cl
		mov		[BitOffset],ecx
		
		pop		ebx                    		//back
		mov		al,[K]
		cmp		ebx,[MaxCode]          		//exceeded size?
		jb		Make_Into_Code        		//no
		cmp		[NBits],12            		//less than 12 bit encoding?
		jb		Another_Bit           		//give it one more

//---------------------------------------------------------------
// All codes up to 12 bits are used: clear table and restart
//---------------------------------------------------------------

		mov		eax,Clear             	//write a clear code.....out of bits
		
		//call writeCode
		xor		edx,edx					//make sure the DL is CLEAR
		mov		edi,[OutBufferPos]   	//obtain destination address
		mov		ecx,[BitOffset]        	//get bitposition
		jecxz	save2
	}
		
shift2:

	__asm
	{
		shl		ax,1
		rcl		edx,1
		loop	shift2
		or		al,[edi]
	}
	
save2:

	__asm
	{
		stosw
		mov		al,dl
		stosb
		
		//AdvanceBuffer
		
		mov		ecx,[NBits]                 ;get number of bits to advance
		mov		eax,[BitOffset]             ;get low word of OutBuffer
		add		eax,ecx
		mov		cl,al
		shr		al,3
		add		[OutBufferPos],eax
		and		cl,7
		movzx	ecx,cl
		mov		[BitOffset],ecx
		
		//call	InitTable             		//Cleanup table
		mov		[NBits],9          			//Starting with 9 bit codes
		mov		[MaxCode],512      			//10 0000 0000 b
		mov		eax,-1              		//Marked as unused
		mov		ecx,clearSize				//Clear first 256 entries
		mov		edi,[LZCHashBuf]   			//Start at the start of buffer
		rep		stosb
		mov		[FreeCode],First_Free 		//Next code to use is first_free

		
		mov		al,[K]                		//Last char
		jmp		Make_Into_Code
	}

//---------------------------------------------------------------
// Extended bit length by a bit, adjusting Max_Code accordingly
//---------------------------------------------------------------
Another_Bit:

	__asm
	{
		inc		[NBits]               		//one more
		shl		[MaxCode],1           		//Double it's size
		jmp		Make_Into_Code
	}

//----------------------------------------------------------------
// No more input: flush what's left and perform housekeeping
//----------------------------------------------------------------

FoundEOF:

	__asm
	{
		mov		eax,[PrefixCode]			//write the last code
		
		//call	writeCode
		xor		edx,edx					//make sure the DL is CLEAR
		mov		edi,[OutBufferPos]   	//obtain destination address
		mov		ecx,[BitOffset]        	//get bitposition
		jecxz	save3
	}
		
shift3:

	__asm
	{
		shl		ax,1
		rcl		edx,1
		loop	shift3
		or		al,[edi]
	}
	
save3:

	__asm
	{
		stosw
		mov		al,dl
		stosb
		
		//AdvanceBuffer
		
		mov		ecx,[NBits]                 ;get number of bits to advance
		mov		eax,[BitOffset]             ;get low word of OutBuffer
		add		eax,ecx
		mov		cl,al
		shr		al,3
		add		[OutBufferPos],eax
		and		cl,7
		movzx	ecx,cl
		mov		[BitOffset],ecx
		
		mov		eax,EOF               		//write EOF code
		
		//call	writeCode
		xor		edx,edx					//make sure the DL is CLEAR
		mov		edi,[OutBufferPos]   	//obtain destination address
		mov		ecx,[BitOffset]        	//get bitposition
		jecxz	save4
	}
		
shift4:

	__asm
	{
		shl		ax,1
		rcl		edx,1
		loop	shift4
		or		al,[edi]
	}
	
save4:

	__asm
	{
		stosw
		mov		al,dl
		stosb
		
		//AdvanceBuffer
		
		mov		ecx,[NBits]                 ;get number of bits to advance
		mov		eax,[BitOffset]             ;get low word of OutBuffer
		add		eax,ecx
		mov		cl,al
		shr		al,3
		add		[OutBufferPos],eax
		and		cl,7
		movzx	ecx,cl
		mov		[BitOffset],ecx
		
		mov		eax,[BitOffset]       		//bits waiting to go?
		or		eax,eax
		je		CompressDone       			//no....just close things up and go back
		inc		[OutBufferPos]
	}
	
CompressDone:

	__asm
	{
		//	return number of bytes in compressed buffer
		mov	eax,[OutBufferPos]
		sub	eax,[OutBuffer]
		
		mov [result],eax
	}

	return(result);
}

#else // LINUX_BUILD

// LZ Compress Routine
// Takes a pointer to dest buffer, a pointer to source buffer and len of source.
// returns length of compressed image.
size_t LZCompress (MemoryPtr dest, MemoryPtr src, size_t srcLen)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    int compression_level = -1; // tradeoff between speed/memeory used = to 6, available 0 - 9

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, compression_level);
    if (ret != Z_OK)
        return 0;

    MemoryPtr dataptr = src;
    size_t len_left = srcLen;
    size_t out_size = 0;
    do {

        strm.avail_in = len_left >= CHUNK ? CHUNK : len_left;
        flush = len_left > CHUNK ? Z_NO_FLUSH : Z_FINISH ;
        memcpy(in, dataptr, strm.avail_in);
        strm.next_in = in;

        dataptr += strm.avail_in;
        len_left -= strm.avail_in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = deflate(&strm, flush);    /* no bad return value */
            gosASSERT(ret != Z_STREAM_ERROR);  /* state not clobbered */

            have = CHUNK - strm.avail_out;
            memcpy(dest, out, have);
            dest += have;
            out_size += have;

        } while(strm.avail_out == 0);
        gosASSERT(strm.avail_in == 0);     /* all input will be used */

    /* done when last data in file processed */
    } while (flush != Z_FINISH);
    gosASSERT(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return out_size;
}
#endif

