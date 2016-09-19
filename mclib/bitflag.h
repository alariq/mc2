//******************************************************************************************
//	bitflag.h - This file contains the definitions for the bitflag class
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef BITFLAG_H
#define BITFLAG_H
//----------------------------------------------------------------------------------
// Include Files
#ifndef DBITFLAG_H
#include"dbitflag.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef VFX_H
#include"vfx.h"
#endif

//----------------------------------------------------------------------------------
// Macro Definitions
#define BITS_PER_BYTE	8

#define NO_RAM_FOR_FLAG_HEAP	0xFFFA0000
#define NUM_BITS_NOT_SUPPORTED	0xFFFA0001

//----------------------------------------------------------------------------------
// Class Definitions
class BitFlag
{
	//Data Members
	//-------------
	protected:
	
		HeapManagerPtr		flagHeap;
		unsigned char		numBitsPerFlag;
		unsigned long		rows;
		unsigned long		columns;
		unsigned char		maskValue;
		unsigned long		divValue;
		unsigned long		colWidth;
		unsigned long		totalFlags;
		unsigned long		totalRAM;
		
	//Member Functions
	//-----------------
	public:
	
		void init (void)
		{
			flagHeap = NULL;
			numBitsPerFlag = 0;
			rows = columns = 0;
			maskValue = 0;
			divValue = 1;
			colWidth = 1;
		}
		
		BitFlag (void)
		{
			init();
		}
		
		long init (unsigned long numRows, unsigned long numColumns, unsigned long initialValue = 0);
		void destroy (void);
		
		~BitFlag (void)
		{
			destroy();
		}
	
		void resetAll (unsigned long bits);
			
		void setFlag (unsigned long r, unsigned long c);
		void clearFlag (unsigned long r, unsigned long c);
		
		void setGroup (unsigned long r, unsigned long c, unsigned long length);
		void clearGroup (unsigned long r, unsigned long c, unsigned long length);
		
		unsigned char getFlag (unsigned long r, unsigned long c);
};

//----------------------------------------------------------------------------------
class ByteFlag
{
	//Data Members
	//-------------
	protected:
	
		HeapManagerPtr		flagHeap;
		unsigned long		rows;
		unsigned long		columns;
		unsigned long		totalFlags;
		unsigned long		totalRAM;
		
		PANE				*flagPane;
		WINDOW				*flagWindow;
		
	//Member Functions
	//-----------------
	public:
	
		void init (void)
		{
			flagHeap = NULL;
			rows = columns = 0;
			
			flagPane = NULL;
			flagWindow = NULL;
		}
		
		ByteFlag (void)
		{
			init();
		}
		
		long init (unsigned long numRows, unsigned long numColumns, unsigned long initialValue = 0);
		void initTGA (char *tgaFileName);

		void destroy (void);
		
		~ByteFlag (void)
		{
			destroy();
		}
	
		MemoryPtr memDump (void)
		{
			return (flagHeap->getHeapPtr());
		}
	
		long getWidth (void)
		{
			return rows;
		}

		long getHeight (void)
		{
			return columns;
		}

		void resetAll (unsigned long byte);
			
		void setFlag (unsigned long r, unsigned long c);
		void clearFlag (unsigned long r, unsigned long c);
		
		void setGroup (unsigned long r, unsigned long c, unsigned long length);
		void clearGroup (unsigned long r, unsigned long c, unsigned long length);
		
		unsigned char getFlag (unsigned long r, unsigned long c);
		
		void setCircle (unsigned long x, unsigned long y, unsigned long radius, unsigned char value);
		void clearCircle (unsigned long x, unsigned long y, unsigned long radius, unsigned char value);
};

//----------------------------------------------------------------------------------
#endif
