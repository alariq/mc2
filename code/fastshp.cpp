//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef VFX_H
#include"vfx.h"
#endif

extern char AlphaTable[256*256];



static long			paneX0;
static long 		paneY0;
static long 		paneX1;
static long 		paneY1;
static long			shapeInfo;
static long 		yOffsetTable;
static long 		DestWidth,pwYMax;
static long 		scanLines;
static long 		scanCol;
static long 		topLeftX;
static long 		topLeftY;
static long 		firstScanOffset;
static long 		lastScanOffset;
static long			totalBlt;
static long			scanStart;
static long			screenBuffer;
static long			currentOffset;
static long 		leftClipX, rightClipX;

#define X0_OFFSET			4
#define Y0_OFFSET			8
#define X1_OFFSET			12
#define Y1_OFFSET			16
#define WIN_X_MAX_OFFSET	4
#define WIN_Y_MAX_OFFSET	8
#define SCANLINE_OFFSET		8
#define SCANCOLUMN_OFFSET	10
#define HSX_OFFSET			4
#define HSY_OFFSET			6





long fastShapeDraw (PANE* pane, void *shp, LONG frameNum, LONG hotX, LONG hotY, MemoryPtr fadeTable)
{
	//---------------------------------------------------------------------------------
	// Format of tile shape header is.
	//		long	-- ShapeId
	//		long	-- NumFrames
	//		long[x]	-- Offset to each shape
	//
	// Format of tile shape data is.
	//		long 	-- ShapeId			Always 0x48414e44 'HAND'
	//		short	-- HotSpot X
	//		short	-- HotSpot Y
	//		short	-- shapeWidth
	//		short	-- shapeHeight
	//		short[x]-- Offset table to each shape in block of memory
	//		shape0	-- Data for shape
	//
	//		REGS:
	//			esi		Pointer to Shape Data
	//			edi		Pointer to Screen Memory
	//			
	//---------------------------------------------------------------------------------
	__asm
	{
		mov		esi, shp
		mov		eax, frameNum

		mov		edi, pane
		xor		edx, edx

		mov		eax,[esi+eax*4+8]
		mov		leftClipX,edx

		add		esi,eax								// ESI points at start of shape
		mov		ecx, [edi]							// Get window pointer from pane

		mov		shapeInfo, esi
		mov		edx,[esi+SCANLINE_OFFSET]			// SCANCOLUMN_OFFSET in high word
	
		mov		eax,edx
		and		edx,0ffffh
		
		shr		eax,16
		mov		scanLines, edx						// Got height of sprite

		mov		scanCol, eax						// Got width of sprite
		mov		edx, [ecx+WIN_Y_MAX_OFFSET]

		mov		ecx, [ecx+WIN_X_MAX_OFFSET]
		mov		eax, [edi+X1_OFFSET]

		mov		pwYMax, edx
		inc 	ecx

		mov		DestWidth, ecx
		dec		ecx									// Clip plane X1 and Y1 (without branches)

		xor		ebx,ebx
		sub		eax,ecx

		setge	bl

		dec		ebx
		nop

		and		eax,ebx
		mov		ebx, [edi+Y1_OFFSET]

		add		eax,ecx
		xor		ecx,ecx

		mov		paneX1, eax
		sub		ebx,edx

		setge	cl

		dec		ecx
		mov		eax, [edi+X0_OFFSET]

		and		ebx,ecx
		mov		ecx, [edi+Y0_OFFSET]				// Now clip pane X0 and Y0 to window

		add		ebx,edx
		mov		edx,ecx								// If less than 0, make 0
		
		mov		paneY1, ebx
		mov		ebx,eax

		sar		ebx,31
		xor		edx,-1

		sar		edx,31
		xor		ebx,-1

		and		eax,ebx
		and		ecx,edx

		mov		paneX0,eax
		mov		paneY0,ecx

	//------------------------------------------------------------
	// Calculate topLeft X and Y position on Screen from HotSpots
	//long topLeftX = (hotX + paneX0) - shapeInfo->HSX;
	//long topLeftY = (hotY + paneY0) - shapeInfo->HSY;
	//
	//MemoryPtr screenBuffer = pane->window->buffer;
	//-----------------------------------------------------------

		mov		ebx,hotX
		mov		edx,hotY

		add		eax,ebx
		add		ecx,edx

		movsx	ebx,word ptr [esi+HSX_OFFSET]

		movsx	edx,word ptr [esi+HSY_OFFSET]

		sub		eax,ebx
		sub		ecx,edx

		mov		topLeftX, eax
		mov		topLeftY, ecx

		mov		ebx, [edi]
		mov		edi, [ebx]

		mov		ebx, scanCol
		mov		edx, paneX0

	//-----------------------------------------------------------
	// Check topLeftX and topLeftY to see if shape is completely
	// off of the screen.
	//if (topLeftX < (paneX0 - scanCol))
	//{
	//	return(FULLY_CLIPPED);
	//}
	//
	//if (topLeftY < (paneY0 - scanLines))
	//{
	//	return(FULLY_CLIPPED);
	//}
	//
	//if (topLeftX > paneX1)
	//{
	//	return(FULLY_CLIPPED);
	//}
	//
	//if (topLeftY > paneY1)
	//{
	//	return(FULLY_CLIPPED);
	//}
	//
	// At this point, eax is topLeftX, ecx is topLeftY, edi is screenBuffer
	//-----------------------------------------------------------
		cmp		eax, paneX1
		jge		Exit

		cmp		ecx, paneY1
		jge		Exit

		sub		edx, ebx
		nop

		cmp		eax, edx
		jle		Exit

		mov		edx, paneY0
		mov		screenBuffer, edi

		sub		edx, scanLines
		nop

		cmp		ecx, edx
		jle		Exit
				
	//-----------------------------------------------------------
	// Find pointer to screenBuffer where tile will start.
	// Also find total Blt Length
	//long yOffset = paneY0;
	//
	//if (topLeftY > paneY0)
	//	yOffset = topLeftY;
	//	
	//screenBuffer += (yOffset * DestWidth);
	//MemoryPtr scanStart = screenBuffer;
	//-----------------------------------------------------------

		cmp		eax, paneX0			//eax is still topLeftX
		jl		LEFTXCLIP

		add		eax, ebx
		nop

		cmp		eax, paneX1
		jg		RIGHTXCLIP
		
			
FIND_BUF:		
		inc		ebx
		mov 	eax, paneY0

		mov		totalBlt, ebx
		xor		ebx,ebx

		sub		eax,ecx
		nop

		setle	bl

		dec		ebx
		nop

		and		eax,ebx
		mov		ebx, scanLines

		add		eax,ecx
		nop

		mul		DestWidth

		add		edi, eax
		mov		eax,paneY0
	//-----------------------------------------------------------
	// Find scanLine Information
	//if (topLeftY < paneY0)
	//	firstScanOffset = paneY0 - topLeftY;
	//
	//long lastScanOffset = scanLines;
	//if ((topLeftY + scanLines) > paneY1)
	//	lastScanOffset = paneY1 - topLeftY + 1;
	//-----------------------------------------------------------
		xor		edx,edx
		sub		eax,ecx

		setl	dl

		dec		edx
		mov		scanStart, edi

		and		edx,eax
		mov		eax, ebx

		mov		firstScanOffset,edx
		add		eax, ecx

		cmp		eax, paneY1
		jg		OffBottom

LINE311:
		mov		edx, firstScanOffset
		mov 	ecx, shapeInfo

		mov		lastScanOffset, ebx
		push	ebp

		lea		ebx,[ecx+edx*2+12]
		mov		ebp,fadeTable

		movzx	ecx, WORD PTR [ebx]

		mov		currentOffset, ecx
		mov		yOffsetTable, ebx
//
//
// Now check for color 0 pixel in top left (alpha palette)
//
//
		movzx	eax,word ptr [esi+12]		;Get offset 
		add		eax,esi						;Get pointer to top left of sprite data
		cmp		word ptr [eax],1			; Single pixel (01h), color 0 (00h)
		jz		AlphaBlit
//
// Check for fade table or not
//
		test	ebp,ebp
		jnz		fade_loop

		mov		ebp,totalBlt
		nop
//
//
// Loop with no fade table
//
//
LOOP_TOP:
		cmp		edx, lastScanOffset
		jge		ExitPop
		
		xor		ebx, ebx				//Setup for this scanline
		xor		ecx, ecx

		mov		esi, shapeInfo
		mov		eax, currentOffset

		add		esi, eax
		xor		eax, eax				//Clear Color Store
										//Check if we need to skip pixels in (i.e. Left Clipped)
		cmp		leftClipX, 0
		jle		SKIP_IN					//We are on the correct scanLine of screen but must skip IN if not leftClipped

SKIP_X:

		xor		ecx,ecx
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String

		cmp		ecx, 0x80				//Check if Run or String
		jb		RLE1

		sub		ecx, 0x80				//Get String Length
		inc		esi

		add		ebx, ecx				//Add to total runs found so far.
		nop

		cmp		ebx, leftClipX			//Is this enough yet?
		jg		ENOUGH_SKIP_STR

		add		esi, ecx				//Otherwise, skip the string completely.
		jmp		SKIP_X					//Onto the next one.
	
RLE1:
		add		ebx, ecx				//Add to Total Run
		inc		esi

		cmp		ebx, leftClipX			//Check if this run is enough
		jg		ENOUGH_SKIP_RLE

		inc		esi						//Otherwise, skip the color completely
		jmp		SKIP_X					//Onto the next one.

//We have skipped in enough.
//Now we need to draw any remainder in the current String
ENOUGH_SKIP_STR:

		sub 	ebx, leftClipX			//How much do we need to draw
		add		esi, ecx				//Skip to end of string.

		mov		ecx, ebx				//This is the string length to draw.
		add		edi, paneX0

		sub		esi, ecx				//Skip back to clip start
		nop

		rep		movsb
		jmp		BLT

//We have skipped in enough.
//Now we need to draw any remainder in the current RLE
ENOUGH_SKIP_RLE:

		sub		ebx, leftClipX
		mov		ecx, ebx
		mov		al, BYTE PTR [esi]		//Get Byte
		inc		esi
		
		add		edi, paneX0
		cmp		al, 0xff
		jne		DO_BLT1
		add		edi, ecx
		jmp		BLT
	
DO_BLT1:
		rep		stosb					//Store color on screen
		jmp		BLT




SKIP_IN:
		add		edi, topLeftX			//Skip in to the hotspot for this scanLine if NOT LeftClipped!
	
BLT:
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String
		inc		esi

		sub		ecx, 128				//Check if Run or String
		jb		BLT_RLE1				//Jump to RUN Code if RUN

		add		ebx, ecx				//Continue to update BltLength
		cmp		ebx, ebp				//See if we have reached the end of this scanline

		jle		GO_STRING1
		
		sub		ebx, ebp				//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, ebp				//Force total Blted to totalBlt
		
GO_STRING1:
		mov al,[esi]
		inc esi
		mov [edi],al
		inc edi
		dec ecx
		jnz GO_STRING1

//CHK_END_STRING:

		cmp		ebx, ebp				//Are we Done with this scanLine?
		je		NEXT_SCANLINE			//yes, next
		jmp		BLT						//No, keep Blting


BLT_RLE1:
		lea		ebx,[ebx+ecx+128]		//Continue to update BltLength
		add		ecx,128

		cmp		ebx, ebp				//See if we have reached the end of this scanline
		jle		GO_RLE1
		
		sub		ebx, ebp				//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, ebp				//Force total Blted to totalBlt
		
GO_RLE1:
		mov		al, BYTE PTR [esi]		//Get Byte
		inc		esi						//Move ShapePtr

		cmp		al, 0xff
		jne		DO_BLT2

		add		edi, ecx
		nop
			
		cmp		ebx, ebp				//Are we Done with this scanLine?
		jne		BLT						//yes, next

NEXT_SCANLINE:
		mov		edi,scanStart			//Move Screen Buffer down to next scanline
		mov		ebx,DestWidth

		inc 	edx						//Move to next scanLine
		add		edi,ebx

		mov		ebx, yOffsetTable		
		mov		scanStart, edi

		movzx	ecx, WORD PTR [ebx]		//Get next yOffset

		add		ebx, 2					//Move the yOffsetTable Up by one short
		mov		currentOffset, ecx

		mov		yOffsetTable, ebx
		jmp		LOOP_TOP

	
DO_BLT2:
		rep		stosb					//Blt the run to the screen
		cmp		ebx, ebp				//Are we Done with this scanLine?
		jne		BLT						//yes, next
		jmp		NEXT_SCANLINE
















//
//
// Loop with a fade table
//
//
fade_loop:
		cmp		edx, lastScanOffset
		jge		ExitPop
		
		xor		ebx, ebx				//Setup for this scanline
		xor		ecx, ecx

		mov		esi, shapeInfo
		mov		eax, currentOffset

		add		esi, eax
		xor		eax,eax
										//Check if we need to skip pixels in (i.e. Left Clipped)
		cmp		leftClipX, 0
		jle		fSKIP_IN				//We are on the correct scanLine of screen but must skip IN if not leftClipped

fSKIP_X:

		xor		ecx,ecx
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String

		cmp		ecx, 0x80				//Check if Run or String
		jb		fRLE1

		sub		ecx, 0x80				//Get String Length
		inc		esi

		add		ebx, ecx				//Add to total runs found so far.
		nop

		cmp		ebx, leftClipX			//Is this enough yet?
		jg		fENOUGH_SKIP_STR

		add		esi, ecx				//Otherwise, skip the string completely.
		jmp		fSKIP_X					//Onto the next one.
	
fRLE1:

		add		ebx, ecx				//Add to Total Run
		inc		esi

		cmp		ebx, leftClipX			//Check if this run is enough
		jg		fENOUGH_SKIP_RLE

		inc		esi						//Otherwise, skip the color completely
		jmp		fSKIP_X					//Onto the next one.

//We have skipped in enough.
//Now we need to draw any remainder in the current String
fENOUGH_SKIP_STR:

		sub 	ebx, leftClipX			//How much do we need to draw
		add		esi, ecx				//Skip to end of string.

		mov		ecx, ebx				//This is the string length to draw.
		add		edi, paneX0

		sub		esi, ecx				//Skip back to clip start
		nop

//fFADE_STRING_LOOP1:
		test	cl,cl					//Check loop end
		je 		fBLT

fd1:	mov		al, BYTE PTR [esi]		//Get Byte
		inc		esi

		dec		cl						//stringLength--, shapeData++, ScreenBuffer++
		inc		edi

		mov		ch, BYTE PTR [eax+ebp]	//Get New Color
		test	cl,cl
		
		mov		BYTE PTR [edi], ch		//Put color on screen
		jnz		fd1

		jmp 	fBLT					//Back to Loop Top


//We have skipped in enough.
//Now we need to draw any remainder in the current RLE
fENOUGH_SKIP_RLE:

		sub		ebx, leftClipX
		mov		al, BYTE PTR [esi]		//Get Byte

		mov		ecx, ebx
		inc		esi
		
		mov		al, BYTE PTR [eax+ebp]	//Get New Color
		add		edi, paneX0

		cmp		al, 0xff
		jne		fDO_BLT1

		add		edi, ecx
		jmp		fBLT
	
fDO_BLT1:
		rep		stosb					//Store color on screen
		jmp		fBLT




fSKIP_IN:
		add		edi, topLeftX			//Skip in to the hotspot for this scanLine if NOT LeftClipped!
	
fBLT:
		xor		ecx,ecx
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String

		cmp		ecx, 128				//Check if Run or String
		jb		fBLT_RLE1				//Jump to RUN Code if RUN

		sub		ecx, 128				//Get String Length
		inc		esi

		add		ebx, ecx				//Continue to update BltLength
		nop

		cmp		ebx, totalBlt			//See if we have reached the end of this scanline
		jle		fFADE_STRING_LOOP2
		
		sub		ebx, totalBlt			//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, totalBlt			//Force total Blted to totalBlt
		
fFADE_STRING_LOOP2:

		xor		eax,eax
		test	cl,cl					//Check loop end

		je 		fCHK_END_STRING
		mov		al, BYTE PTR [esi]		//Get Byte

		dec		cl						//stringLength--, shapeData++, ScreenBuffer++
		inc		esi

		inc		edi
		mov		ch, BYTE PTR [eax+ebp]	//Get New Color

		cmp		ch, 0xff
		je		fFADE_STRING_LOOP2

		mov		BYTE PTR [edi-1], ch	//Put color on screen
		jmp 	fFADE_STRING_LOOP2		//Back to Loop Top


fCHK_END_STRING:

		cmp		ebx, totalBlt			//Are we Done with this scanLine?
		je		fNEXT_SCANLINE			//yes, next
		jmp		fBLT						//No, keep Blting

fBLT_RLE1:

		add		ebx, ecx				//Continue to update BltLength
		inc		esi

		cmp		ebx, totalBlt			//See if we have reached the end of this scanline
		jle		fGO_RLE1
		
		sub		ebx, totalBlt			//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, totalBlt			//Force total Blted to totalBlt
		
fGO_RLE1:
		mov		al, BYTE PTR [esi]		//Get Byte
		inc		esi						//Move ShapePtr
		mov		al, BYTE PTR [eax+ebp]	//Get New Color
		cmp		al, 0xff
		jne		fDO_BLT2
		add		edi, ecx

		cmp		ebx, totalBlt			//Are we Done with this scanLine?
		jne		fBLT					//yes, next


fNEXT_SCANLINE:
		mov		edi,scanStart			//Move Screen Buffer down to next scanline
		mov		ebx,DestWidth

		inc 	edx						//Move to next scanLine
		add		edi,ebx

		mov		ebx, yOffsetTable		
		mov		scanStart, edi

		movzx	ecx, WORD PTR [ebx]		//Get next yOffset

		add		ebx, 2					//Move the yOffsetTable Up by one short
		mov		currentOffset, ecx

		mov		yOffsetTable, ebx
		jmp		fade_loop

fDO_BLT2:
		rep		stosb					//Blt the run to the screen
		cmp		ebx, totalBlt			//Are we Done with this scanLine?
		je		fNEXT_SCANLINE			//yes, next
		jmp		fBLT						//No, keep Blting



















//
// Check for fade table or not
//
AlphaBlit:
		test	ebp,ebp
		jnz		afade_loop

		mov		ebp,totalBlt
		nop
//
//
// Loop with no fade table
//
//
aLOOP_TOP:
		cmp		edx, lastScanOffset
		jge		ExitPop
		
		xor		ebx, ebx				//Setup for this scanline
		xor		ecx, ecx

		mov		esi, shapeInfo
		mov		eax, currentOffset

		add		esi, eax
		xor		eax, eax				//Clear Color Store
										//Check if we need to skip pixels in (i.e. Left Clipped)
		cmp		leftClipX, 0
		jle		aSKIP_IN					//We are on the correct scanLine of screen but must skip IN if not leftClipped

aSKIP_X:

		xor		ecx,ecx
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String

		cmp		ecx, 0x80				//Check if Run or String
		jb		aRLE1

		sub		ecx, 0x80				//Get String Length
		inc		esi

		add		ebx, ecx				//Add to total runs found so far.
		nop

		cmp		ebx, leftClipX			//Is this enough yet?
		jg		aENOUGH_SKIP_STR

		add		esi, ecx				//Otherwise, skip the string completely.
		jmp		aSKIP_X					//Onto the next one.
	
aRLE1:
		add		ebx, ecx				//Add to Total Run
		inc		esi

		cmp		ebx, leftClipX			//Check if this run is enough
		jg		aENOUGH_SKIP_RLE

		inc		esi						//Otherwise, skip the color completely
		jmp		aSKIP_X					//Onto the next one.

//We have skipped in enough.
//Now we need to draw any remainder in the current String
aENOUGH_SKIP_STR:

		sub 	ebx, leftClipX			//How much do we need to draw
		add		esi, ecx				//Skip to end of string.

		mov		ecx, ebx				//This is the string length to draw.
		add		edi, paneX0

		sub		esi, ecx				//Skip back to clip start
		nop

		test	ecx,ecx
		jz		aBLT
		xor		eax,eax

aes:	mov		ah,[esi]
		inc		esi
		mov		al,[edi]
		inc		edi
		dec		ecx
		mov		al,AlphaTable[eax]
		mov		[edi-1],al
		jnz		aes

		jmp		aBLT

//We have skipped in enough.
//Now we need to draw any remainder in the current RLE
aENOUGH_SKIP_RLE:

		sub		ebx, leftClipX
		mov		ecx, ebx
		xor		eax,eax
		mov		ah, BYTE PTR [esi]		//Get Byte
		inc		esi
		
		add		edi, paneX0
		cmp		ah, 0xff
		jne		aDO_BLT1
		add		edi, ecx
		jmp		aBLT
	
aDO_BLT1:
		test	ecx,ecx
		jz		aBLT
aes1:	mov		al,[edi]
		inc		edi
		dec		ecx
		mov		al,AlphaTable[eax]
		mov		[edi-1],al
		jnz		aes1
		jmp		aBLT




aSKIP_IN:
		add		edi, topLeftX			//Skip in to the hotspot for this scanLine if NOT LeftClipped!
	
aBLT:
		xor		ecx,ecx
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String

		cmp		ecx, 128				//Check if Run or String
		jb		aBLT_RLE1				//Jump to RUN Code if RUN

		sub		ecx, 128				//Get String Length
		inc		esi

		add		ebx, ecx				//Continue to update BltLength
		xor		eax,eax

		cmp		ebx, ebp				//See if we have reached the end of this scanline
		jle		aGO_STRING1
		
		sub		ebx, ebp				//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, ebp				//Force total Blted to totalBlt
		
aGO_STRING1:
		mov al,[edi]
		inc edi
		mov ah,[esi]
		inc esi
		mov al,AlphaTable[eax]
		dec ecx
		mov [edi-1],al
		jnz aGO_STRING1

//aCHK_END_STRING:

		cmp		ebx, ebp				//Are we Done with this scanLine?
		je		aNEXT_SCANLINE			//yes, next
		jmp		aBLT						//No, keep Blting


aBLT_RLE1:
		add		ebx, ecx				//Continue to update BltLength
		inc		esi

		cmp		ebx, ebp				//See if we have reached the end of this scanline
		jle		aGO_RLE1
		
		sub		ebx, ebp				//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, ebp				//Force total Blted to totalBlt
		
aGO_RLE1:
		xor		eax,eax
		mov		al, BYTE PTR [esi]		//Get Byte

		cmp		al, 0xff
		jne		aDO_BLT2

		add		edi, ecx
		inc		esi						//Move ShapePtr
			
		cmp		ebx, ebp				//Are we Done with this scanLine?
		jne		aBLT						//yes, next

aNEXT_SCANLINE:
		mov		edi,scanStart			//Move Screen Buffer down to next scanline
		mov		ebx,DestWidth

		inc 	edx						//Move to next scanLine
		add		edi,ebx

		mov		ebx, yOffsetTable		
		mov		scanStart, edi

		movzx	ecx, WORD PTR [ebx]		//Get next yOffset

		add		ebx, 2					//Move the yOffsetTable Up by one short
		mov		currentOffset, ecx

		mov		yOffsetTable, ebx
		jmp		aLOOP_TOP

	
aDO_BLT2:
		inc		esi						//Move ShapePtr
		mov		ah,al
		test ecx,ecx
		jz aes2
aes3:	mov		al,[edi]
		inc		edi
		dec		ecx
		mov		al,AlphaTable[eax]
		mov		[edi-1],al
		jnz		aes3

aes2:	cmp		ebx, ebp				//Are we Done with this scanLine?
		jne		aBLT						//yes, next
		jmp		aNEXT_SCANLINE

//
//
// ALPHA Loop with a fade table
//
//
afade_loop:
		cmp		edx, lastScanOffset
		jge		ExitPop
		
		xor		ebx, ebx				//Setup for this scanline
		xor		ecx, ecx

		mov		esi, shapeInfo
		mov		eax, currentOffset

		add		esi, eax
		xor		eax,eax
										//Check if we need to skip pixels in (i.e. Left Clipped)
		cmp		leftClipX, 0
		jle		afSKIP_IN				//We are on the correct scanLine of screen but must skip IN if not leftClipped

afSKIP_X:

		xor		ecx,ecx
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String

		cmp		ecx, 0x80				//Check if Run or String
		jb		afRLE1

		sub		ecx, 0x80				//Get String Length
		inc		esi

		add		ebx, ecx				//Add to total runs found so far.
		nop

		cmp		ebx, leftClipX			//Is this enough yet?
		jg		afENOUGH_SKIP_STR

		add		esi, ecx				//Otherwise, skip the string completely.
		jmp		afSKIP_X					//Onto the next one.
	
afRLE1:

		add		ebx, ecx				//Add to Total Run
		inc		esi

		cmp		ebx, leftClipX			//Check if this run is enough
		jg		afENOUGH_SKIP_RLE

		inc		esi						//Otherwise, skip the color completely
		jmp		afSKIP_X					//Onto the next one.

//We have skipped in enough.
//Now we need to draw any remainder in the current String
afENOUGH_SKIP_STR:

		sub 	ebx, leftClipX			//How much do we need to draw
		add		esi, ecx				//Skip to end of string.

		mov		ecx, ebx				//This is the string length to draw.
		add		edi, paneX0

		sub		esi, ecx				//Skip back to clip start
		nop

//afFADE_STRING_LOOP1:
		test	cl,cl					//Check loop end
		je 		afBLT

afd1:	xor		eax,eax
		mov		al, BYTE PTR [esi]		//Get Byte

		inc		esi
		inc		edi

		mov		ah, BYTE PTR [eax+ebp]	//Get New Color
		dec		cl
		mov		al,[edi]
		mov		al,AlphaTable[eax]
		mov		BYTE PTR [edi], al		//Put color on screen
		jnz		afd1

		jmp 	afBLT					//Back to Loop Top


//We have skipped in enough.
//Now we need to draw any remainder in the current RLE
afENOUGH_SKIP_RLE:

		sub		ebx, leftClipX
		mov		al, BYTE PTR [esi]		//Get Byte

		mov		ecx, ebx
		inc		esi
		
		mov		ah, BYTE PTR [eax+ebp]	//Get New Color
		add		edi, paneX0

		cmp		ah, 0xff
		jne		afDO_BLT1

		add		edi, ecx
		jmp		afBLT
	
afDO_BLT1:
		test	ecx,ecx
		jz		afBLT
aes4:	mov		al,[edi]
		inc		edi
		dec		ecx
		mov		al,AlphaTable[eax]
		mov		[edi-1],al
		jnz		aes4
		jmp		afBLT




afSKIP_IN:
		add		edi, topLeftX			//Skip in to the hotspot for this scanLine if NOT LeftClipped!
	
afBLT:
		xor		ecx,ecx
		mov		cl, BYTE PTR [esi]		//Get the first data byte.  This is a runLength for RLE or String

		cmp		ecx, 128				//Check if Run or String
		jb		afBLT_RLE1				//Jump to RUN Code if RUN

		sub		ecx, 128				//Get String Length
		inc		esi

		add		ebx, ecx				//Continue to update BltLength
		nop

		cmp		ebx, totalBlt			//See if we have reached the end of this scanline
		jle		afFADE_STRING_LOOP2
		
		sub		ebx, totalBlt			//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, totalBlt			//Force total Blted to totalBlt
		
afFADE_STRING_LOOP2:

		test	cl,cl					//Check loop end
		je 		afCHK_END_STRING

		xor		eax,eax
		mov		al, BYTE PTR [esi]		//Get Byte

		inc		esi
		dec		cl						//stringLength--, shapeData++, ScreenBuffer++

		inc		edi
		mov		ah, BYTE PTR [eax+ebp]	//Get New Color

		cmp		ah, 0xff
		je		afFADE_STRING_LOOP2

		mov		al,[edi-1]
		mov		al,AlphaTable[eax]

		mov		BYTE PTR [edi-1], al	//Put color on screen
		jmp 	afFADE_STRING_LOOP2		//Back to Loop Top


afCHK_END_STRING:

		cmp		ebx, totalBlt			//Are we Done with this scanLine?
		je		afNEXT_SCANLINE			//yes, next
		jmp		afBLT						//No, keep Blting

afBLT_RLE1:

		add		ebx, ecx				//Continue to update BltLength
		inc		esi

		cmp		ebx, totalBlt			//See if we have reached the end of this scanline
		jle		afGO_RLE1
		
		sub		ebx, totalBlt			//Find out how many over we are
		sub		ecx, ebx				//Shorten string by that amount
		mov		ebx, totalBlt			//Force total Blted to totalBlt
		
afGO_RLE1:
		xor		eax,eax
		mov		al, BYTE PTR [esi]		//Get Byte
		
		inc		esi						//Move ShapePtr
		mov		ah, BYTE PTR [eax+ebp]	//Get New Color
		cmp		ah, 0xff
		jne		afDO_BLT2
		add		edi, ecx

		cmp		ebx, totalBlt			//Are we Done with this scanLine?
		jne		afBLT					//yes, next


afNEXT_SCANLINE:
		mov		edi,scanStart			//Move Screen Buffer down to next scanline
		mov		ebx,DestWidth

		inc 	edx						//Move to next scanLine
		add		edi,ebx

		mov		ebx, yOffsetTable		
		mov		scanStart, edi

		movzx	ecx, WORD PTR [ebx]		//Get next yOffset

		add		ebx, 2					//Move the yOffsetTable Up by one short
		mov		currentOffset, ecx

		mov		yOffsetTable, ebx
		jmp		afade_loop

afDO_BLT2:
		test	ecx,ecx
		jz		afBLT
aes5:	mov		al,[edi]
		inc		edi
		dec		ecx
		mov		al,AlphaTable[eax]
		mov		[edi-1],al
		jnz		aes5
		cmp		ebx, totalBlt			//Are we Done with this scanLine?
		je		afNEXT_SCANLINE			//yes, next
		jmp		afBLT						//No, keep Blting
















LEFTXCLIP:
		mov		eax, paneX0
		sub		eax, topLeftX
		sub		ebx, eax
		mov		leftClipX, eax
		jmp		FIND_BUF

RIGHTXCLIP:
		sub		eax, paneX1
		mov		ebx, scanCol
		sub		ebx, eax
		jmp		FIND_BUF

OffBottom:
		mov		ebx, paneY1
		sub		ebx, ecx
		inc 	ebx
		jmp		LINE311


ExitPop:
		pop ebp

Exit:	
	}
	return(0);
}









