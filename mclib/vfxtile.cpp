//-----------------------------------------------------------
// 
// FASA Interactive Technologies
//
// VFX Tile Draw Routine
//
// Started April 30, 1996
// FFS
//
//-----------------------------------------------------------

//-----------------------------------------------------------
// Include Files
#ifndef DSTD_H
#include"dstd.h"
#endif

#ifndef VFX_H
#include"vfx.h"
#endif

#include<cassert>

static unsigned long pwXMax,count;		// Must be static for assembly optimizations
static unsigned char *FadeTable;		// Must be static for assembly optimizations

//-----------------------------------------------------------
struct tileStruct
{
	byte tileHotSpotX;
	byte tileHotSpotY;
	byte numScanLines;
	byte numScanColumns;
};

//-----------------------------------------------------------
struct newShapeStruct
{
	long header;
	short tileHotSpotX;
	short tileHotSpotY;
	short numScanLines;
	short numScanColumns;
};

#define FULLY_CLIPPED		0xCDCF0001
#define FORMAT_SPEC			0xFEEDABBA

#define BLACK				0x10101010

//-----------------------------------------------------------
// This one is called if the cameraScale is 100.  No drop
// of any pixels.  Straight BLT to screen.
long VFX_nTile_draw (PANE* pane, void *tile, LONG hotX, LONG hotY, MemoryPtr fadeTable)
{

#ifdef LINUX_BUILD
#else
	//-----------------------------------------
	// Format of tile shape data is NEW!!
	// Byte -- Hot Spot X.
	// Byte -- Hot Spot Y.
	// Byte -- NumScanLines.  (height)
	// Byte -- NumScanColumns. (width)
	// Word -- One word for each scanline indicating offset into main data for that scanline.
	// Bytes -- Main Shape data.  Each scanline pointed to by above table.
	// The main difference between .fsx and .gsx files are in the main data.
	// Because there is no end scanline marker, the length of each Blt can be
	// determined and a MUCH faster rep movsd can be used to Blt the data.
	//------------------------------------------------------

	//-------------------------------------------------------
	// Create Important Data from tile data.
	MemoryPtr tileData = (MemoryPtr)tile + sizeof(tileStruct);
	tileStruct *tileInfo = (tileStruct *)tile;
	
	unsigned long *yOffsetTable = (unsigned long *)(tileData);

	pwXMax = pane->window->x_max+1;

	long paneX0 = (pane->x0 < 0) ? 0 : pane->x0;
	long paneY0 = (pane->y0 < 0) ? 0 : pane->y0;
	long paneX1 = (pane->x1 >= (long)pwXMax) ? pane->window->x_max : pane->x1;
	long paneY1 = (pane->y1 >= (pane->window->y_max+1)) ? pane->window->y_max : pane->y1;

	long scanLines = tileInfo->numScanLines;
	long scanCol = tileInfo->numScanColumns;

	long topLeftX = (hotX + paneX0) - tileInfo->tileHotSpotX;
	long topLeftY = (hotY + paneY0) - tileInfo->tileHotSpotY;  //In theory, tileHotSpotY is always zero!
	
	if ( (topLeftX >= paneX1) ||
		 (topLeftY >= paneY1) ||
		 (topLeftX <= (paneX0 - scanCol)) ||
		 (topLeftY <= (paneY0 - scanLines)))
		return(FULLY_CLIPPED);

	MemoryPtr screenBuffer = pane->window->buffer + ((topLeftY > paneY0) ? topLeftY*pwXMax : paneY0*pwXMax);

	long firstScanOffset = (topLeftY < paneY0) ? paneY0 - topLeftY : 0;
	
	long lastScanOffset = ((topLeftY + scanLines) > paneY1) ? paneY1 - topLeftY + 1 : scanLines;

	yOffsetTable += firstScanOffset;

	if( (topLeftX>paneX0) && ((topLeftX+scanCol) < paneX1) )
	{
		FadeTable=fadeTable;
		screenBuffer+=topLeftX;
		count=lastScanOffset-firstScanOffset;
//
// No clipping, whole tile is on screen
//
		_asm{
			push ebp
			mov edx,screenBuffer

			mov esi,tile
			mov ebp,yOffsetTable

			mov eax,FadeTable
			add esi,[ebp]

			cmp eax,-1
			jz DoBlack

			test eax,eax
			jnz DoFade

//
// Normal
//
DisplayTile:
			xor ebx,ebx
			mov eax,[ebp+4]

			mov ecx,[ebp]
			lea ebp,[ebp+4]

			sub eax,ecx
			mov bl,[esi]

			mov ecx,0
			jz DisplayTile1

			lea edi,[ebx+edx]
			dec eax

			sub ecx,edi
			inc esi

			and ecx,3
			sub eax,ecx
			jle DisplayTile2
			rep movsb
			mov ecx,eax
			and eax,3				; Main loop, copying an unclipped tile
			shr ecx,2
			jz DisplayTile2
DisplayTile3:
			mov ebx,[esi]
			add esi,4
			mov [edi],ebx
			add edi,4
			dec ecx
			jnz DisplayTile3
DisplayTile2:
			add eax,ecx
			jz DisplayTile1
DisplayTile4:
			mov bl,[esi]
			inc esi
			mov [edi],bl
			inc edi
			dec eax
			jnz DisplayTile4

DisplayTile1:
			mov ebx,pwXMax
			mov eax,count

			add edx,ebx
			dec eax

			mov count,eax
			jnz DisplayTile

			jmp done

//
// Black
//
DoBlack:	xor ebx,ebx
			mov eax,[ebp+4]

			mov ecx,[ebp]
			lea ebp,[ebp+4]

			sub eax,ecx
			mov bl,[esi]

			mov ecx,0
			jz DoBlack1

			lea edi,[ebx+edx]
			dec eax

			sub ecx,edi
			lea esi,[esi+eax+1]

			and ecx,3
			mov ebx,eax

			mov eax,BLACK
			sub ebx,ecx
			jle DoBlack2
			rep stosb
			mov ecx,ebx
			and ebx,3				; Main loop, blacking out an unclipped tile
			shr ecx,2
			rep stosd
DoBlack2:	add ecx,ebx
			rep stosb

DoBlack1:	mov ebx,pwXMax
			mov eax,count

			add edx,ebx
			dec eax

			mov count,eax
			jnz DoBlack

			jmp done

//
// Fade
//
DoFade:		xor ebx,ebx
			mov ecx,[ebp+4]

			mov eax,[ebp]
			lea ebp,[ebp+4]

			sub ecx,eax
			mov bl,[esi]

			lea ecx,[ecx-1]
			jz DoFade1

			lea edi,[ebx+edx]
			inc esi

// Fade inner loop
			push edx
			mov edx,FadeTable

			mov bl,[esi]
			inc esi

DoFade0:	inc edi
			dec ecx

			mov al,[edx+ebx]
			mov bl,[esi]

			mov [edi-1],al
			lea esi,[esi+1]
				
			jnz DoFade0
			pop edx
			dec esi

DoFade1:	mov ebx,pwXMax
			mov eax,count

			add edx,ebx
			dec eax

			mov count,eax
			jnz Dofade
done:
			pop ebp
		}

	}
	else
//
// This tile needs to be clipped
//
	{
		unsigned long currentOffset = *yOffsetTable++;
		unsigned long nextOffset=*yOffsetTable++;

		for (long scanLine = firstScanOffset; scanLine<lastScanOffset; scanLine++)
		{
			__asm
			{
				mov esi, tile
				xor ebx, ebx

				add esi, currentOffset
				xor eax, eax

				mov	al, BYTE PTR [esi]
				inc esi

				add eax, topLeftX
				mov edi, screenBuffer

				add edi, eax
				mov ecx, nextOffset

				sub ecx, currentOffset
				xor edx, edx

				dec ecx
				mov edx, eax

				add edx, ecx
				cmp eax, paneX0

				jge SHORT LINE207
				
				mov ebx, paneX0
				sub ebx, eax
				
				add esi, ebx
				add edi, ebx
LINE207:
				cmp edx, paneX1
				jle SHORT LINE214
				
				sub edx, paneX1
				dec edx
				jmp SHORT LEAVE_EDX
			
LINE214:
				xor edx, edx

LEAVE_EDX:
				sub ecx, ebx
				sub ecx, edx
				
				cmp ecx, 0
				jle SHORT NO_DRAW
			
				mov eax,fadeTable
				cmp eax,-1
				jz SHORT BLT_BLACK
				
				test eax,eax
				jz BLT_FAST
			
//FADE_BLT_TOP:
				xor edx, edx
				mov ebx,fadeTable

				mov dl, [esi]
				inc esi
Blt_fade1:
				inc edi
				dec ecx

				mov al, [edx+ebx]
				mov dl, [esi]

				mov BYTE PTR [edi-1], al
				lea esi,[esi+1]
					
				jnz Blt_fade1
				jmp short NO_DRAW

BLT_BLACK:
				mov ebx,ecx				; DWORD align edi when possible
				sub ecx,edi
				mov eax,BLACK
				sub ecx,ebx
				and ecx,3
				sub ebx,ecx
				jle blt_black1
				rep stosb
				mov ecx,ebx
				and ebx,3
				shr ecx,2
				rep stosd
blt_black1:		add ecx,ebx
				rep stosb
				jmp SHORT NO_DRAW

BLT_FAST:
				mov eax,ecx				; DWORD align edi when possible
				sub ecx,edi
				sub ecx,eax
				and ecx,3
				sub eax,ecx
				jle blt_fast1
				rep movsb
				mov ecx,eax
				and eax,3
				shr ecx,2
				rep movsd
blt_fast1:		add ecx,eax
				rep movsb
NO_DRAW:
			}

			screenBuffer += pwXMax;
			currentOffset = nextOffset;
			nextOffset = *yOffsetTable++;
		}
	}
	//-----------------------------------------------------------
#endif // LINUX_BUILD
	return(0x00);
}

//----------------------------------------------------------------------------
//
// long VFX_shape_origin(void *shape_table, LONG shape_number);
//
// Returns hotspot of the shape (in pixels) relative to the upper left bounds).
// (E)AX=x E(AX)=y.
//
//----------------------------------------------------------------------------

long VFX_shape_origin(void *shape_table, LONG shape_number)
{
	long result = -1;

#ifdef LINUX_BUILD
#else
	__asm
	{
                mov esi,shape_table
                add esi,8                       //skip to offsets
                
                mov eax,shape_number            //point to shape offset ptr 
                shl eax,3                       //mul eax by sizeof 2 longs
                add esi,eax                     

                mov esi,[esi]                   //get shape offset ptr
                add esi,shape_table				//add base address

                mov eax,[esi+4]					//SHAPEHEADER.origin

				mov	result,eax
	}
#endif 
	return result;
}

//----------------------------------------------------------------------------
//
// long VFX_shape_resolution(void *shape_table, LONG shape_number);
//
// Returns x,y resolution (image size) in pixels.  (E)AX=x E(AX)=y.
//
//----------------------------------------------------------------------------

long VFX_shape_resolution(void *shape_table, LONG shape_number)
{
	long result = 0;

#ifdef LINUX_BUILD
#else
	__asm
	{
				mov esi,shape_table
                add esi,8                       //skip to offsets

                
                mov eax,shape_number            //point to shape offset ptr 
                shl eax,3                       //mul eax by sizeof 2 longs
                add esi,eax                     

                mov esi,[esi]                   //get shape offset ptr
                add esi,shape_table	            //add base address

                mov eax,[esi+16]				//SHAPEHEADER.xmax  ;eax = xmax - xmin
                sub eax,[esi+8]					//SHAPEHEADER.xmin
                inc eax                         //      + 1

                mov ebx,[esi+20]				//SHAPEHEADER.ymax  ;ebx = ymax - ymin
                sub ebx,[esi+12]				//SHAPEHEADER.ymin
                inc ebx                         //      + 1

                shl eax,16
				and eax,0xffff0000
                add eax,ebx

				mov	result,eax
	}
#endif // LINUX_BUILD
	return result;
}

//----------------------------------------------------------------------------
//
// long VFX_shape_minxy(void *shape_table, LONG shape_number);
//
// Returns min x,min y in pixels.  (E)AX=x E(AX)=y.
//
//---------------------------------------------------------------------------

long VFX_shape_minxy(void *shape_table, LONG shape_number)
{
	long result = 0;

#ifdef LINUX_BUILD
#else
	__asm
	{
                mov esi,shape_table
                add esi,8                       //skip to offsets
                
                mov eax,shape_number            //point to shape offset ptr 
                shl eax,3                       //mul eax by sizeof 2 longs
                add esi,eax                     

                mov esi,[esi]						//get shape offset ptr
                add esi,shape_table				//add base address

                mov eax,[esi+8]					//SHAPEHEADER.xmin
                shl eax,16

				mov ebx,[esi+12]				//SHAPEHEADER.ymin
				and eax,0xffff0000
				add eax,ebx

				mov result, eax
	}
#endif // LINUX_BUILD
	return result;
}

//;----------------------------------------------------------------------------
//;
//; long VFX_shape_bounds(void *shape_table, LONG shape_number);
//;
//; Returns width,height of the shape (including transparent areas) in pixels.  (E)AX=x E(AX)=y.
//; (E)AX=x E(AX)=y.
//;
//;----------------------------------------------------------------------------

int VFX_shape_bounds (void *shape_table, long shape_number)
{
	int boundsResult = -1;

#ifdef LINUX_BUILD
    assert(0);
#else
	__asm
	{
		mov esi,shape_table
		add esi,8                       //skip to offsets
                
		mov eax,shape_number            //point to shape offset ptr 
		shl eax,3                       //mul eax by sizeof 2 longs
		add esi,eax                     

		mov esi,[esi]                   //get shape offset ptr
		add esi,shape_table				//add base address

		mov eax,[esi]					//Bounds is First Member of struct
		mov boundsResult, eax
	}
#endif // LINUX_BUILD
	return boundsResult;
}

//;----------------------------------------------------------------------------
//;
//; long VFX_shape_count(void *shape_table);
//;
//; Returns number of shapes references in the specified shape table. Each
//; shape may have one or more references.
//;
//;----------------------------------------------------------------------------

long VFX_shape_count (void *shape_table)
{
	long countResult = 0;

#ifdef LINUX_BUILD
#else
	__asm
	{
		mov esi,shape_table
		mov eax,[esi+4]
		mov countResult,eax
	}
#endif // LINUX_BUILD
	return countResult;
}


//;----------------------------------------------------------------------------
//;
//; int cdecl VFX_line_draw (PANE *panep, int x0, int y0, int x1, int y1,
//;                                                   int mode, int parm);
//;
//; This function clips and draws a line to a pane.
//;
//; The panep parameter specifies the pane.
//;
//; The x0 and y0 parameters specify the initial endpoint of the line.
//; The x1 and y1 parameters specify the final endpoint of the line.
//;
//; The mode parameter specifies the operation to perform on each point
//; in the line.
//;
//; If mode is...   parm specifies...
//;
//;   DRAW            a color
//;   TRANSLATE       the address of a color translation table
//;   EXECUTE         the address of a caller-provided function
//;
//; If mode is DRAW (0), the line is drawn in a solid color specified by 
//; parm. 
//;
//; If mode is TRANSLATE (1), the line is drawn with color translation. 
//; Each pixel along the path of the line is replaced with the correspond-
//; ing entry in the color translation table specified by parm.
//;
//; If mode is EXECUTE (2), the line is drawn with the aid of a callback 
//; function specifed by parm.  For each point on the line, VFX_line_draw() 
//; executes the callback function, passing it the coordinates of the point.
//;
//; The callback function must use cdecl parameter passing, and its para-
//; meter list must be (int x, int y).  The function's return type is not
//; important; VFX_line_draw() ignores the return value (if any).  
//;
//; VFX_line_draw() clips the line to the pane.  The locus of the clipped 
//; line is the same for all modes and is guaranteed to be identical to the 
//; intersection of the loci of the unclipped line and the pane.  Moreover, 
//; plotting always proceeds from (x0,y0) to (x1,y1) regardless of the 
//; relative orientation of the two points.
//;
//; The locus of the clipped line consists of all points in the pane whose 
//; minor-axis distance* from the ideal line is less than or equal to 1/2,
//; with the following exception.  In places where the ideal line passes 
//; exactly halfway between two pixels which share the same major-axis co-
//; ordinate, only one of the two points is plotted.  The selection method 
//; is unspecified, but is consistent throughout the line.
//;
//; * The minor-axis distance from a point P to a line L is the absolute 
//;   difference between the minor-axis coordinates of P and Q where Q is
//;   the point on L having the same major-axis coordinate as P.  (Here,
//;   major-axis and minor axis are determined by L.  The major axis is
//;   the axis in which the endpoints of L differ the most.  Likewise the
//;   minor-axis is the one in which the endpoints differ the least).
//;
//; VFX_line_draw is reentrant, so callback functions can use it.
//;
//;
//; Examples:
//;
//;    #define HELIOTROPE 147
//;    UBYTE color_negative[256];
//;    void cdecl DrawDiamond (int x, int y);
//;
//;    VFX_line_draw (pane, Px, Py, Qx, Qy, DRAW, HELIOTROPE);
//;       draws a line from (Px,Py) to (Qx,Qy) using the color HELIOTROPE.
//;
//;    VFX_line_draw (pane, Px, Py, Qx, Qy, TRANSLATE, (int) color_negative);
//;       draws a line from (Px,Py) to (Qx,Qy) replacing each pixel with its
//;       color negative (as specified by the table color_negative).
//;
//;    VFX_line_draw (pane, Px, Py, Qx, Qy, EXECUTE, (int) DrawDiamond);
//;       draws a line of diamonds from (Px,Py) to (Qx,Qy) using the
//;       caller-provided function DrawDiamond().
//;
//; Return values:
//;
//;   -2: pane was malformed or completely outside its window
//;   -1: window was malformed
//;    0: all of the line was inside the pane and was drawn without clipping
//;    1: some of the line was inside the pane and was drawn after clipping
//;    2: the line was completely outside the pane and was not drawn
//;
//;----------------------------------------------------------------------------

LONG VFX_line_draw (PANE *panep, LONG x0, LONG y0, 
                                 LONG x1, LONG y1, LONG mode, ptrdiff_t parm)

{

	long	lineResult = 0;
#ifdef LINUX_BUILD
	assert(0 && "VFX_line_draw is not implemented");
#else
	long _dx, absdx, sgndx;
	long _dy, absdy, sgndy;
	long sgndxdy, slope;
	long x0_, y0_, x1_, y1_;
	long clip_flags;
	    
	long		_L;	//Leftmost pixel in Window coord.
	long		_T;	//Top
	long		_R;	//Right
	long		_B;	//Bottom
	              
	MemoryPtr	_A;	//Base address of Clipped Pane
	long		_W;	//Width of underlying window (bytes)
	
	long		_CX;	//Window x coord. = Pane x coord. + CP_CX
	long		_CY;	//Window y coord. = Pane x coord. + CP_CY


	__asm
	{
		cld

	}

	//Clip Pane to Window Routine

		// get panep (esi)
        // windowp (ebx) = panep->win

		//ASSUME  esi:PPANE
		//ASSUME  ebx:PWIN

		__asm
			{
                mov     esi,panep
                mov     ebx,[esi]		//This is the Window Pointer

                // _W = windowp->wnd_x1 + 1
                // if _W <= 0, return bad window
    
                mov     eax,[ebx+4]		//X1 Window Coord
                inc     eax
                mov     _W,eax
                jle     ReturnBadWindow
    
                // ecx = Ysize = windowp->wnd_y1 + 1
                // if <= 0, return bad window

                mov     eax,[ebx+8]		//y1 Window Coord
                inc     eax
                mov     ecx,eax
                jle     ReturnBadWindow

                // clip pane to window:
                //  pane_x0 = max (pane->x0, 0)
                //  pane_y0 = max (pane->y0, 0)
                //  pane_x1 = min (pane->x1, _W - 1)
                //  pane_y1 = min (pane->x1, (Ysize=ecx) - 1)

                mov     eax,[esi+4]		//x0 in Pane Coord
                mov     _CX,eax

	            cmp     eax,0
	            jg      around1
		        mov     eax,0
around1:
                mov     _L,eax
    
                mov     eax,[esi+8]		//y0 in Pane Coord
                mov     _CY,eax

	            cmp     eax,0
	            jg      around2
		        mov     eax,0
around2:
                mov     _T,eax
    
                mov     eax,[esi+12]	//X1 in Pane Coord
                mov     edx,_W
                dec     edx

				cmp     eax,edx
				jl      around3
				mov     eax,edx
around3:
                mov     _R,eax
    
                mov     eax,[esi+16]	//y1 in Pane Coord
                mov     edx,ecx
                dec     edx
				cmp     eax,edx
				jl      around4
				mov     eax,edx
around4:
                mov     _B,eax

                // exit if pane is malformed or completely off window:
                //  if _B < &vname&_T, return bad pane
                //  if _R < &vname&_L, return bad pane

                mov     eax,_R
                cmp     eax,_L
                jl      ReturnBadPane
    
                mov     eax,_B
                cmp     eax,_T
                jl      ReturnBadPane

                mov     eax,[ebx]		//Buffer in Window
                mov     _A,eax

                jmp     exit1

ReturnBadWindow:
                mov     eax,-1

				jmp		exit1
			}

			__asm
			{

ReturnBadPane:
                mov     eax,-2
			}

exit1:

			__asm
			{
		//Convert Quad Pane to Window

                mov     eax,_CX
                add     x0,eax
                add     x1,eax

                mov     eax,_CY
                add     y0,eax
                add     y1,eax

		// calculate dx, absdx, and sgndx
    
                mov     eax,x1
                sub     eax,x0
                mov     _dx,eax
    
                cdq
                mov     sgndx,edx
    
                xor     eax,edx
                sub     eax,edx
                mov     absdx,eax

        // calculate dy, absdy, and sgndy
    
                mov     eax,y1
                sub     eax,y0
                mov     _dy,eax
    
                cdq
                mov     sgndy,edx
    
                xor     eax,edx
                sub     eax,edx
                mov     absdy,eax

        // make working copies of endpoint coordinates

	            mov     eax,x0
		        mov     x0_,eax

	            mov     eax,x1
		        mov     x1_,eax

	            mov     eax,y0
		        mov     y0_,eax

	            mov     eax,y1
		        mov     y1_,eax

        // handle special cases -- vertical, horizontal
    
                cmp     _dx,0
                je      Vertical
    
                cmp     _dy,0
                je      Horizontal
    
        // calculate sgndxdy
    
                mov     eax,sgndx
                xor     eax,sgndy
                mov     sgndxdy,eax

        // calculate slope
    
                mov     edx,absdx
                mov     ebx,absdy
                mov     eax,0xFFFFFFFF
    
                cmp     edx,ebx
                je      slope2
                jl      slope1
    
                xchg    edx,ebx
slope1:
                xor     eax,eax
                div     ebx

slope2:
                mov     slope,eax

        // clip line to pane

                mov     clip_flags,0

clip_loop:
                xor     edx,edx

        // calculate clip0 (dl)
    
                mov     eax,x0_
                sub     eax,_L
                shl     eax,1
                adc     dl,dl
    
                mov     eax,_R
                sub     eax,x0_
                shl     eax,1
                adc     dl,dl
    
                mov     eax,y0_
                sub     eax,_T
                shl     eax,1
                adc     dl,dl
    
                mov     eax,_B
                sub     eax,y0_
                shl     eax,1
                adc     dl,dl

        // calculate clip1 (dh)

                mov     eax,x1_
                sub     eax,_L
                shl     eax,1
                adc     dh,dh
    
                mov     eax,_R
                sub     eax,x1_
                shl     eax,1
                adc     dh,dh
    
                mov     eax,y1_
                sub     eax,_T
                shl     eax,1
                adc     dh,dh
    
                mov     eax,_B
                sub     eax,y1_
                shl     eax,1
                adc     dh,dh

        // remember clip flags for final return value

                or      clip_flags,edx

        // accept if line is completely in the pane

                or      edx,edx
                jz      Accept

        // reject if line is completely above, below, left of, or right of the pane

                test    dl,dh
                jnz     ReturnReject


        // dispatch to appropriate clipper
    
                mov     ebx,absdx
                cmp     ebx,absdy
                jl      ClipYmajor

//ClipXmajor:

        // clip (x0,y0)

                test    dl,1000B
                jnz     Xmaj_x0_lo
                test    dl,0100B
                jnz     Xmaj_x0_hi
                test    dl,0010B
                jnz     Xmaj_y0_lo
                test    dl,0001B
                jnz     Xmaj_y0_hi

        // clip (x1,y1)

                test    dh,1000B
                jnz     Xmaj_x1_lo
                test    dh,0100B
                jnz     Xmaj_x1_hi
                test    dh,0010B
                jnz     Xmaj_y1_lo
                test    dh,0001B
                jnz     Xmaj_y1_hi
    
                jmp     clip_loop

ClipYmajor:

        // clip (x0,y0)

                test    dl,1000B
                jnz     Ymaj_x0_lo
                test    dl,0100B
                jnz     Ymaj_x0_hi
                test    dl,0010B
                jnz     Ymaj_y0_lo
                test    dl,0001B
                jnz     Ymaj_y0_hi

        // clip (x1,y1)

                test    dh,1000B
                jnz     Ymaj_x1_lo
                test    dh,0100B
                jnz     Ymaj_x1_hi
                test    dh,0010B
                jnz     Ymaj_y1_lo
                test    dh,0001B
                jnz     Ymaj_y1_hi
    
                jmp     clip_loop

Xmaj_x0_lo:

        // x0_ = CP_L;
        // y0_ = y0 + sgndxdy * floor ((x0_-x0)*slope)+1/2);
    
				mov		eax,_L
				mov		x0_,eax

                sub     eax,x0
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y0_,eax
    
                jmp     clip_loop

Ymaj_x0_lo:

        // x0_ = CP_L;
        // y0_ = y0 + sgndxdy * ceil ((x0_-x0-1/2)/slope);
    
				mov		eax, _L
				mov		x0_, eax
                sub     eax,x0
    
                mov     edx,eax
                dec     edx
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,-1
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y0_,eax
    
                jmp     clip_loop
    
Xmaj_x0_hi:

        // x0_ = CP_R;
        // y0_ = y0 - sgndxdy * floor ((x0-CP_R)*slope)+1/2);

				mov		eax,_R
				mov		x0_,eax

                sub     eax,x0
                neg     eax
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y0_,eax
    
                jmp     clip_loop
    
Ymaj_x0_hi:

        // x0_ = CP_R;
        // y0_ = y0 - sgndxdy * ceil ((x0-x0_-1/2)/slope);

				mov		eax,_R
				mov		x0_,eax

                sub     eax,x0
                neg     eax
    
                mov     edx,eax
                dec     edx
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,-1
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y0_,eax
    
                jmp     clip_loop

Ymaj_y0_lo:

        // y0_ = CP_T;
        // x0_ = x0 + sgndxdy * floor ((y0_-y0)*slope)+1/2);

				mov		eax,_T
				mov		y0_,eax

                sub     eax,y0
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x0_,eax
    
                jmp     clip_loop
    
Xmaj_y0_lo:

        // y0_ = CP_T;
        // x0_ = x0 + sgndxdy * ceil ((y0_-y0-1/2)/slope);
    
				mov		eax,_T
				mov		y0_,eax

                sub     eax,y0
    
                mov     edx,eax
                dec     edx
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,-1
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x0_,eax
    
                jmp     clip_loop

Ymaj_y0_hi:

        // y0_ = CP_B;
        // x0_ = x0 - sgndxdy * floor ((y0-y0_)*slope+1/2);

				mov		eax,_B
				mov		y0_,eax

                sub     eax,y0
                neg     eax
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x0_,eax
    
                jmp     clip_loop

Xmaj_y0_hi:

        // y0_ = CP_B;
        // x0_ = x0 - sgndxdy * ceil ((y0-y0_-1/2)/slope);
    
				mov		eax,_B
				mov		y0_,eax

                sub     eax,y0
                neg     eax
    
                mov     edx,eax
                dec     edx
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,-1
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x0_,eax
    
                jmp     clip_loop

Xmaj_x1_lo:

        // x1_ = CP_L;
        // y1_ = y0 - sgndxdy * floor ((x0-x1_)*slope+1/2);

				mov		eax,_L
				mov		x1_,eax

                sub     eax,x0
                neg     eax
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y1_,eax
    
                jmp     clip_loop

Ymaj_x1_lo:

        // x1_ = CP_L;
        // y1_ = y0 - sgndxdy * (ceil ((x0-x1_+1/2)/slope) - 1);

				mov		eax,_L
				mov		x1_,eax

                sub     eax,x0
                neg     eax
    
                mov     edx,eax
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,0
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y1_,eax
    
                jmp     clip_loop

Xmaj_x1_hi:

        // x1_ = CP_R;
        // y1_ = y0 + sgndxdy * floor ((x1_-x0)*slope+1/2);
    
				mov		eax,_R
				mov		x1_,eax

                sub     eax,x0
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y1_,eax
    
                jmp     clip_loop

Ymaj_x1_hi:

        // x1_ = CP_R;
        // y1_ = y0 + sgndxdy * (ceil ((x1_-x0+1/2)/slope) - 1);

				mov		eax,_R
				mov		x1_,eax

                sub     eax,x0
    
                mov     edx,eax
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,0
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,y0
                mov     y1_,eax
    
                jmp     clip_loop

Ymaj_y1_lo:

        // y1_ = CP_T;
        // x1_ = x0 - sgndxdy * floor ((y0-y1_)*slope+1/2);

				mov		eax,_T
				mov		y1_,eax

                sub     eax,y0
                neg     eax
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x1_,eax
    
                jmp     clip_loop

Xmaj_y1_lo:

        // y1_ = CP_T;
        // x1_ = x0 - sgndxdy * (ceil ((y0-y1_+1/2)/slope) - 1);
    
				mov		eax,_T
				mov		y1_,eax

                sub     eax,y0
                neg     eax
    
                mov     edx,eax
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,0
    
                mov     edx,sgndxdy
                not     edx
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x1_,eax
    
                jmp     clip_loop
    
Ymaj_y1_hi:

        // y1_ = CP_B;
        // x1_ = x0 + sgndxdy * floor ((y1_-y0)*slope+1/2);

				mov		eax,_B
				mov		y1_,eax

                sub     eax,y0
    
                mul     slope
                add     eax,0x80000000

                adc     edx,0
                mov     eax,edx
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x1_,eax
    
                jmp     clip_loop

Xmaj_y1_hi:

        // y1_ = CP_B;
        // x1_ = x0 + sgndxdy * (ceil ((y1_-y0+1/2)/slope) - 1);

				mov		eax,_B
				mov		y1_,eax

                sub     eax,y0
    
                mov     edx,eax
                mov     eax,0x80000000

                div     slope
    
                cmp     edx,1
                sbb     eax,0
    
                mov     edx,sgndxdy
    
                xor     eax,edx
                sub     eax,edx
    
                add     eax,x0
                mov     x1_,eax
    
                jmp     clip_loop

#if 0
//----------------------------------------------------------------------------
//
// Macros for inner loops of DRAW forms (Y major, X major, and Straight)
//
//----------------------------------------------------------------------------

YM_DRAW     MACRO   adc_sbb

            mov     [edi],al            ; [adr] = pixel

            add     edx,ebx             ; accum += slope
            adc_sbb edi,esi             ; adr += ystep (+ xstep)
            dec     ecx                 ; count--

            ENDM

//----------------------------------------------------------------------------
XM_DRAW     MACRO   inc_dec

            mov     [edi],al            ; [adr] = pixel

            inc_dec edi                 ; adr += xstep
            add     edx,ebx             ; accum += slope
            jnc     @F                  ; if accum overflowed,
            add     edi,esi             ;   adr += ystep
@@:
            dec     ecx                 ; count--

            ENDM

//----------------------------------------------------------------------------
ST_DRAW     MACRO

            mov     [edi],al            ; [adr] = pixel

            add     edi,esi             ; adr += xystep
            dec     ecx                 ; count--

            ENDM

//----------------------------------------------------------------------------
//
// Macros for inner loops of XLAT forms (Y major, X major, and Straight)
//

//----------------------------------------------------------------------------
YM_XLAT     MACRO   adc_sbb

            mov     al,[edi]            ; pixel = [adr]
            xlat                        ; pixel = parm[pixel]
            mov     [edi],al            ; [adr] = pixel

            add     edx,ebp             ; accum += slope
            adc_sbb edi,esi             ; adr += ystep (+ xstep)
            dec     ecx                 ; count--

            ENDM

//----------------------------------------------------------------------------
XM_XLAT     MACRO   inc_dec

            mov     al,[edi]            ; pixel = [adr]
            xlat                        ; pixel = parm[pixel]
            mov     [edi],al            ; [adr] = pixel

            inc_dec edi                 ; adr += xstep
            add     edx,ebp             ; accum += slope
            jnc     @F                  ; if accum overflowed,
            add     edi,esi             ;   adr += ystep
@@:
            dec     ecx                 ; count--

            ENDM

//----------------------------------------------------------------------------
SW_XLAT     MACRO

            mov     al,[edi]            ; pixel = [adr]
            xlat                        ; pixel = parm[pixel]
            mov     [edi],al            ; [adr] = pixel

            add     edi,esi             ; adr += xystep
            dec     ecx                 ; count--

            ENDM

//----------------------------------------------------------------------------
#endif

Accept:

        // calculate adr (edi),
        // address of first pixel = window_buffer + CP_W*y0 + x0

            //GET_WINDOW_ADDRESS  x0_, y0_

			mov     eax,y0_
			imul    _W
			add     eax,_A
			add     eax,x0_

            mov     edi,eax

        // calculate ystep (esi) = CP_W * sgn (dy)

            mov     esi,_W
            xor     esi,sgndy
            sub     esi,sgndy

        // get slope

            mov     ebx,slope

        // branch to Diagonal, Xmajor or Ymajor depending on absdx & absdy

            mov     eax,absdx
            cmp     eax,absdy
            je      Diagonal
            jg      Xmajor

;----------------------------------------------------------------------------
//Ymajor:     

        // calculate count (ecx) = abs (y1_ - y0_) + 1

            mov     eax,y1_
            sub     eax,y0_
            cdq
            xor     eax,edx
            sub     eax,edx
            inc     eax
            mov     ecx,eax

        // calculate accum (edx) = abs (y0_ - y0) * slope + 1/2

            mov     eax,y0_
            sub     eax,y0
            cdq
            xor     eax,edx
            sub     eax,edx

            mul     ebx
            add     eax,0x80000000

            mov     edx,eax

        // branch to YmajorNegdx or fall through to YmajorPosdx depending on sgndx

            cmp     sgndx,-1
            je      YmajorNegdx

//----------------------------------------------------------------------------
//YmajorPosdx:
            cmp     mode,1
            je      YmPdxXlat
            jg      YmPdxProc

//----------------------------------------------------------------------------
//YmPdxDraw:
            mov     eax,parm            // get line color

YmPdxDrawLoop:
            //REPEAT  LD_COPIES-1         //; repeat this code copies-1 times or 3 times!

            //YM_DRAW <adc>               //;   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edx,ebx             // accum += slope
            adc		edi,esi             // adr += ystep (+ xstep)
            dec     ecx                 // count--

            jz      YmPdxDrawDone

            //YM_DRAW <adc>               //;   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edx,ebx             // accum += slope
            adc		edi,esi             // adr += ystep (+ xstep)
            dec     ecx                 // count--

            jz      YmPdxDrawDone

            //YM_DRAW <adc>               //;   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edx,ebx             // accum += slope
            adc		edi,esi             // adr += ystep (+ xstep)
            dec     ecx                 // count--

            jz      YmPdxDrawDone

            //YM_DRAW <adc>               //;   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edx,ebx             // accum += slope
            adc		edi,esi             // adr += ystep (+ xstep)
            dec     ecx                 // count--

            jnz     YmPdxDrawLoop       //; while (count)

YmPdxDrawDone:
            jmp     ReturnClipFlags     //; done

//----------------------------------------------------------------------------
YmPdxXlat:
            mov     eax,parm            //; get translation table pointer
            push    ebp                 //; preserve ebp
            mov     ebp,ebx             //; use ebp for slope to free up ebx
            mov     ebx,eax             //; table ptr must be in ebx for xlat

YmPdxXlatLoop:
            //REPEAT  LD_COPIES-1         //; repeat this code copies-1 times or 3 Times
            //YM_XLAT <adc>               //;   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            adc		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmPdxXlatDone

            //YM_XLAT <adc>               //;   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            adc		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmPdxXlatDone

            //YM_XLAT <adc>               //;   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            adc		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmPdxXlatDone

            //YM_XLAT <adc>               //;   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            adc		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jnz     YmPdxXlatLoop       //; while (count)

YmPdxXlatDone:
            pop     ebp                 //; restore ebp
            jmp     ReturnClipFlags     //; done

//----------------------------------------------------------------------------
YmPdxProc:
            mov     esi,panep           // get pane pointer

            mov     edi,x0_             // x (edi) = x0_ in pane coordinates
            sub     edi,[esi+4]			//.x0

            mov     eax,y0_             // y (esi) = y0_ in pane coordinates
            sub     eax,[esi+8]			//.y0
            mov     esi,eax

            mov     eax,sgndy           // ybump (eax) = (sgndy=-1) ? -1 : +1
            add     eax,eax
            inc     eax

YmPdxProcLoop:
            pushad                      // callback (x, y)
            call    parm
            popad

            add     edx,ebx             // accum += slope
            jnc     F1                  // if overflow, x++
            inc     edi
F1:
            add     esi,eax             // y += ybump

            dec     ecx                 // count--
            jnz     YmPdxProcLoop       // while (count)

            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
YmajorNegdx:
            neg     esi                 // neg_ystep (esi) = -ystep

            cmp     mode,1
            je      YmNdxXlat
            jg      YmNdxProc

//----------------------------------------------------------------------------
//YmNdxDraw:
            mov     eax,parm            // get line color

YmNdxDrawLoop:
            //REPEAT  LD_COPIES-1         // repeat this code copies-1 times or 3 times
            //YM_DRAW <sbb>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebx             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmNdxDrawDone

            //YM_DRAW <sbb>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebx             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmNdxDrawDone

            //YM_DRAW <sbb>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebx             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmNdxDrawDone

            //YM_DRAW <sbb>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebx             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jnz     YmNdxDrawLoop       // while (count)

YmNdxDrawDone:
            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
YmNdxXlat:
            mov     eax,parm            // get translation table pointer
            push    ebp                 // preserve ebp
            mov     ebp,ebx             // use ebp for slope to free up ebx
            mov     ebx,eax             // table ptr must be in ebx for xlat

YmNdxXlatLoop:
            //REPEAT  LD_COPIES-1         // repeat this code copies-1 times or 3 times
            //YM_XLAT <sbb>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmNdxXlatDone

            //YM_XLAT <sbb>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmNdxXlatDone

            //YM_XLAT <sbb>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jz      YmNdxXlatDone

            //YM_XLAT <sbb>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edx,ebp             //; accum += slope
            sbb		edi,esi             //; adr += ystep (+ xstep)
            dec     ecx                 //; count--

            jnz     YmNdxXlatLoop       // while (count)

YmNdxXlatDone:
            pop     ebp                 // restore ebp
            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
YmNdxProc:
            mov     esi,panep           // get pane pointer

            mov     edi,x0_             // x (edi) = x0_ in pane coordinates
            sub     edi,[esi+4]			//.x0

            mov     eax,y0_             // y (esi) = y0_ in pane coordinates
            sub     eax,[esi+8]			//.y0
            mov     esi,eax

            mov     eax,sgndy           // ybump (eax) = (sgndy=-1) ? -1 : +1
            add     eax,eax
            inc     eax

YmNdxProcLoop:
            pushad                      // callback (x, y)
            call    parm
            popad

            add     edx,ebx             // accum += slope
            jnc     F2                  // if overflow, x--
            dec     edi
F2:
            add     esi,eax             // y += ybump

            dec     ecx                 // count--
            jnz     YmNdxProcLoop       // while (count)

            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
Xmajor:

        // calculate count (ecx) = abs (x1_ - x0_) + 1

            mov     eax,x1_
            sub     eax,x0_
            cdq
            xor     eax,edx
            sub     eax,edx
            inc     eax
            mov     ecx,eax

        // calculate accum (edx) = abs (x0_ - x0) * slope + 1/2

            mov     eax,x0_
            sub     eax,x0
            cdq
            xor     eax,edx
            sub     eax,edx

            mul     ebx
            add     eax,0x80000000

            mov     edx,eax

        // branch to XmajorNegdx or fall through to XmajorPosdx depending on sgndx

            cmp     sgndx,-1
            je      XmajorNegdx

//----------------------------------------------------------------------------
//XmajorPosdx:
            cmp     mode,1
            je      XmPdxXlat
            jg      XmPdxProc

//----------------------------------------------------------------------------
//XmPdxDraw:
            mov     eax,parm            // get line color
            
XmPdxDrawLoop:
            //REPEAT  LD_COPIES-1         // repeat this code copies-1 times or 3 Times
            //XM_DRAW <inc>               //    process a pixel
            mov     [edi],al            // [adr] = pixel

            inc		edi                 // adr += xstep
            add     edx,ebx             // accum += slope
            jnc     Fa1                  // if accum overflowed,
            add     edi,esi             //   adr += ystep
Fa1:
            dec     ecx                 // count--

            jz      XmPdxDrawDone

            //XM_DRAW <inc>               //    process a pixel
            mov     [edi],al            // [adr] = pixel

            inc		edi                 // adr += xstep
            add     edx,ebx             // accum += slope
            jnc     Fa2                  // if accum overflowed,
            add     edi,esi             //   adr += ystep
Fa2:
            dec     ecx                 // count--

            jz      XmPdxDrawDone

            //XM_DRAW <inc>               //    process a pixel
            mov     [edi],al            // [adr] = pixel

            inc		edi                 // adr += xstep
            add     edx,ebx             // accum += slope
            jnc     Fa3                  // if accum overflowed,
            add     edi,esi             //   adr += ystep
Fa3:
            dec     ecx                 // count--

            jz      XmPdxDrawDone

            //XM_DRAW <inc>               //    process a pixel
            mov     [edi],al            // [adr] = pixel

            inc		edi                 // adr += xstep
            add     edx,ebx             // accum += slope
            jnc     Fa4                  // if accum overflowed,
            add     edi,esi             //   adr += ystep
Fa4:
            dec     ecx                 // count--

            jnz     XmPdxDrawLoop       // while (count)

XmPdxDrawDone:
            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
XmPdxXlat:
            mov     eax,parm            // get translation table pointer
            push    ebp                 // preserve ebp
            mov     ebp,ebx             // use ebp for slope to free up ebx
            mov     ebx,eax             // table ptr must be in ebx for xlat

XmPdxXlatLoop:
            //REPEAT  LD_COPIES-1         // repeat this code copies-1 times or 3 times
            //XM_XLAT <inc>               //    process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            inc		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fb1                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fb1:
            dec     ecx                 //; count--

            jz      XmPdxXlatDone

            //XM_XLAT <inc>               //    process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            inc		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fb2                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fb2:
            dec     ecx                 //; count--

            jz      XmPdxXlatDone

            //XM_XLAT <inc>               //    process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            inc		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fb3                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fb3:
            dec     ecx                 //; count--

            jz      XmPdxXlatDone

            //XM_XLAT <inc>               //    process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            inc		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fb4                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fb4:
            dec     ecx                 //; count--

            jnz     XmPdxXlatLoop       // while (count)

XmPdxXlatDone:
            pop     ebp                 // restore ebp
            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
XmPdxProc:
            mov     esi,panep           // get pane pointer

            mov     edi,x0_             // x (edi) = x0_ in pane coordinates
            sub     edi,[esi+4]			//.x0

            mov     eax,y0_             // y (esi) = y0_ in pane coordinates
            sub     eax,[esi+8]			//.y0
            mov     esi,eax

            mov     eax,sgndy           // xbump (eax) = (sgndy=-1) ? -1 : +1
            add     eax,eax
            inc     eax

XmPdxProcLoop:
            pushad                      // callback (x, y)
            call    parm
            popad

            add     edx,ebx             // accum += slope
            jnc     F3                  // if overflow, y++
            inc     esi
F3:         add     edi,eax             // x += xbump

            dec     ecx                 // count--
            jnz     XmPdxProcLoop       // while (count)

            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
XmajorNegdx:
            cmp     mode,1
            je      XmNdxXlat
            jg      XmNdxProc

//----------------------------------------------------------------------------
//XmNdxDraw:
            mov     eax,parm            // get line color
            
XmNdxDrawLoop:
            //REPEAT  LD_COPIES-1         // repeat this code copies-1 times or 3 times
            //XM_DRAW <dec>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebx             //; accum += slope
            jnc     Fc1                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fc1:
            dec     ecx                 //; count--

            jz      XmNdxDrawDone

            //XM_DRAW <dec>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebx             //; accum += slope
            jnc     Fc2                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fc2:
            dec     ecx                 //; count--

            jz      XmNdxDrawDone

            //XM_DRAW <dec>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebx             //; accum += slope
            jnc     Fc3                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fc3:
            dec     ecx                 //; count--

            jz      XmNdxDrawDone

            //XM_DRAW <dec>               //   process a pixel
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebx             //; accum += slope
            jnc     Fc4                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fc4:
            dec     ecx                 //; count--

            jnz     XmNdxDrawLoop       // while (count)

XmNdxDrawDone:
            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
XmNdxXlat:
            mov     eax,parm            // get translation table pointer
            push    ebp                 // preserve ebp
            mov     ebp,ebx             // use ebp for slope to free up ebx
            mov     ebx,eax             // table ptr must be in ebx for xlat

XmNdxXlatLoop:
            //REPEAT  LD_COPIES-1         // repeat this code copies-1 times or 3 Times
            //XM_XLAT <dec>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fd1                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fd1:
            dec     ecx                 //; count--

            jz      XmNdxXlatDone

            //XM_XLAT <dec>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fd2                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fd2:
            dec     ecx                 //; count--

            jz      XmNdxXlatDone

            //XM_XLAT <dec>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fd3                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fd3:
            dec     ecx                 //; count--

            jz      XmNdxXlatDone

            //XM_XLAT <dec>               //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            dec		edi                 //; adr += xstep
            add     edx,ebp             //; accum += slope
            jnc     Fd4                 //; if accum overflowed,
            add     edi,esi             //;   adr += ystep
Fd4:
            dec     ecx                 //; count--

            jnz     XmNdxXlatLoop       // while (count)

XmNdxXlatDone:
            pop     ebp                 // restore ebp
            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
XmNdxProc:
            mov     esi,panep           // get pane pointer

            mov     edi,x0_             // x (edi) = x0_ in pane coordinates
            sub     edi,[esi+4]			//.x0

            mov     eax,y0_             // y (esi) = y0_ in pane coordinates
            sub     eax,[esi+8]			//.y0
            mov     esi,eax

            mov     eax,sgndy           // xbump (eax) = (sgndy=-1) ? -1 : +1
            add     eax,eax
            inc     eax

XmNdxProcLoop:
            pushad                      // callback (x, y)
            call    parm
            popad

            add     edx,ebx             // accum += slope
            jnc     F4                  // if overflow, y--
            dec     esi
F4:         add     edi,eax             // x += xbump

            dec     ecx                 // count--
            jnz     XmNdxProcLoop       // while (count)

            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
Vertical:

        // reject if line is left or right of pane

            mov     eax,x0
            cmp     eax,_L
            jl      ReturnReject
            cmp     eax,_R
            jg      ReturnReject

        // reject if line is above plane

            mov     eax,y0

            cmp     eax,y1
            jg      around1a1
            mov     eax,y1
around1a1:
            cmp     eax,_T
            jl      ReturnReject

        // reject if line is below plane

            mov     eax,y0
            cmp     eax,y1
            jl      around1b1
            mov     eax,y1
around1b1:
            cmp     eax,_B
            jg      ReturnReject

        // clip y0, clip y1

            mov     eax,y0

            cmp     eax,_T
            jg      around2a1
            mov     eax,_T
around2a1:

            cmp     eax,_B
            jg      around2b1
            mov     eax,_B
around2b1:

            mov     y0_,eax
            mov     eax,y1

            cmp     eax,_T
            jg      around3a1
            mov     eax,_T
around3a1:

            cmp     eax,_B
            jg      around3b1
            mov     eax,_B
around3b1:

            mov     y1_,eax

			mov		eax,x0
			mov		x0_,eax

        // calculate ystep (esi)

            mov     esi,_W
            xor     esi,sgndy
            sub     esi,sgndy

        // calculate count (ecx) =  abs(y1-y0) + 1

            mov     eax,y1_
            sub     eax,y0_
            cdq
            xor     eax,edx
            sub     eax,edx
            mov     ecx,eax
            inc     ecx

            jmp     Straight

//---------------------------------------------------------------------------
Horizontal:

        // reject if line is above or below pane

            mov     eax,y0
            cmp     eax,_T
            jl      ReturnReject
            cmp     eax,_B
            jg      ReturnReject

        // reject if line is left of pane

            mov     eax,x0
            cmp     eax,x1
            jg      around1a
            mov     eax,x1
around1a:
            cmp     eax,_L
            jl      ReturnReject

        // reject if line is right of pane

            mov     eax,x0
            cmp     eax,x1
            jl      around1b
            mov     eax,x1
around1b:
            cmp     eax,_R
            jg      ReturnReject

        // clip x0, clip x1

            mov     eax,x0
            cmp     eax,_L
            jg      around2a
            mov     eax,_L
around2a:
            cmp     eax,_R
            jl      around2b
            mov     eax,_R
around2b:

            mov     x0_,eax

            mov     eax,x1
            cmp     eax,_L
            jg      around3a
            mov     eax,_L
around3a:
            cmp     eax,_R
            jl      around3b
            mov     eax,_R
around3b:
            mov     x1_,eax

			mov		eax,y0
			mov		y0_,eax

        // calculate xstep (esi)

            mov     esi,sgndx
            inc     esi
            or      esi,sgndx

        // calculate count (ecx) =  abs(x1-x0) + 1

            mov     eax,x1_
            sub     eax,x0_
            cdq
            xor     eax,edx
            sub     eax,edx
            mov     ecx,eax
            inc     ecx

            jmp     Straight

;----------------------------------------------------------------------------
Diagonal:

        // calculate xystep (esi)

            mov     esi,_W
            xor     esi,sgndy
            sub     esi,sgndy

            mov     eax,sgndx
            inc     eax
            or      eax,sgndx
            add     esi,eax

        // calculate count (ecx) =  abs(x1-x0) + 1

            mov     eax,x1_
            sub     eax,x0_
            cdq
            xor     eax,edx
            sub     eax,edx
            mov     ecx,eax
            inc     ecx

//----------------------------------------------------------------------------
Straight:

        // calculate adr (edi), address of first pixel = window_buffer + CP_W*y0 + x0

            //GET_WINDOW_ADDRESS  x0_, y0_
			mov     eax,y0_
			imul    _W
			add     eax,_A
			add     eax,x0_

            mov     edi,eax

        // draw the line with a color, a translation table, or a callback function

            cmp     mode,1
            je      StraightXlat
            jg      StraightProc

//----------------------------------------------------------------------------
//StraightDraw:
            mov     eax,parm            // get line color

StraightLoop:
            //REPEAT  LD_COPIES-1         // repeat code copies-1 times or 3 Times
            //ST_DRAW                     //   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edi,esi             // adr += xystep
            dec     ecx                 // count--

            jz      StraightDone

            //ST_DRAW                     //   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edi,esi             // adr += xystep
            dec     ecx                 // count--

            jz      StraightDone

            //ST_DRAW                     //   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edi,esi             // adr += xystep
            dec     ecx                 // count--

            jz      StraightDone

            //ST_DRAW                     //   process a pixel
            mov     [edi],al            // [adr] = pixel

            add     edi,esi             // adr += xystep
            dec     ecx                 // count--

            jnz     StraightLoop        // while (count)

StraightDone:
            jmp     ReturnClipFlags     // done

//----------------------------------------------------------------------------
StraightXlat:
            mov     ebx,parm            // get pointer to translation table

StraightXlatLoop:
            //REPEAT  LD_COPIES-1         // repeat code copies-1 times or 3 Times
            //SW_XLAT                     //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edi,esi             //; adr += xystep
            dec     ecx                 //; count--

            jz      StraightXlatDone

            //SW_XLAT                     //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edi,esi             //; adr += xystep
            dec     ecx                 //; count--

            jz      StraightXlatDone

            //SW_XLAT                     //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edi,esi             //; adr += xystep
            dec     ecx                 //; count--

            jz      StraightXlatDone

            //SW_XLAT                     //   process a pixel
            mov     al,[edi]            //; pixel = [adr]
            xlat                        //; pixel = parm[pixel]
            mov     [edi],al            //; [adr] = pixel

            add     edi,esi             //; adr += xystep
            dec     ecx                 //; count--

            jnz     StraightXlatLoop    // while (count)

StraightXlatDone:
            jmp     ReturnClipFlags     // done

//---------------------------------------------------------------------------

StraightProc:
            mov     esi,panep           // get pane pointer

            mov     edi,x0_             // x (edi) = x0_ in pane coordinates
            sub     edi,[esi+4]			//.x0

            mov     eax,y0_             // y (esi) = y0_ in pane coordinates
            sub     eax,[esi+8]			//.y0
            mov     esi,eax

            xor     eax,eax             // ybump (eax) = sgn (_dy)
            test    _dy,-1
            setnz   al
            or      eax,sgndy

            xor     ebx,ebx             // xbump (ebx) = sgn (_dx)
            test    _dx,-1
            setnz   bl
            or      ebx,sgndx

StraightProcLoop:
            pushad                      // callback (x, y)
            call    parm
            popad

            add     esi,eax             // y += ybump
            add     edi,ebx             // x += xbump

            dec     ecx                 // count--
            jnz     StraightProcLoop    // while (count)

            jmp     ReturnClipFlags     // done

        // return error code:
        //
        // -2: pane was malformed (or completely off its window)
        // -1: window was malformed
        //  0: line was accepted
        //  1: line was clipped
        //  2: line was rejected

ReturnClipFlags:
            xor     eax,eax
            cmp     clip_flags,1
            setae   al
            jmp		exit

ReturnReject:       
            mov     eax,2
            jmp		exit

exit:
			mov		lineResult,eax
		}
#endif // LINUX_BUILD
		return lineResult;
}

//----------------------------------------------------------------------------
//
// int cdecl VFX_pixel_write (PANE *panep, int x, int y, UBYTE color)
//
//     This function writes a single pixel.
//
// The panep parameter specifies the pane containing the pixel to be written.
// The x and y parameters specify the pixel coordinates.
// The color parameter specifies the color to write to the pixel.
//                                          
// Return values:
//
//    0..255:
//       Pixel value prior to write.
//
//   -1: Bad window.
//       The height or width of the pane's window is less than one.
//
//   -2: Bad pane.
//       The height or width of the pane is less than one.
//
//   -3: Off pane.
//       The specified pixel is off the pane.
//                                          
//----------------------------------------------------------------------------

LONG VFX_pixel_write (PANE *panep, LONG x, LONG y, ULONG color)
{
		int result = 0;
#ifdef LINUX_BUILD
	assert(0 && "VFX_pixel_write is not implemented");
#else
		long		_L;	//Leftmost pixel in Window coord.
		long		_T;	//Top
		long		_R;	//Right
		long		_B;	//Bottom
	              
		MemoryPtr	_A;	//Base address of Clipped Pane
		long		_W;	//Width of underlying window (bytes)
	
		long		_CX;	//Window x coord. = Pane x coord. + CP_CX
		long		_CY;	//Window y coord. = Pane x coord. + CP_CY

		
		__asm
			{
				cld
			}

	//Clip Pane to Window Routine

		// get panep (esi)
        // windowp (ebx) = panep->win

		//ASSUME  esi:PPANE
		//ASSUME  ebx:PWIN

		__asm
			{
                mov     esi,panep
                mov     ebx,[esi]		//This is the Window Pointer

                // _W = windowp->wnd_x1 + 1
                // if _W <= 0, return bad window
    
                mov     eax,[ebx+4]		//X1 Window Coord
                inc     eax
                mov     _W,eax
                jle     ReturnBadWindow
    
                // ecx = Ysize = windowp->wnd_y1 + 1
                // if <= 0, return bad window

                mov     eax,[ebx+8]		//y1 Window Coord
                inc     eax
                mov     ecx,eax
                jle     ReturnBadWindow

                // clip pane to window:
                //  pane_x0 = max (pane->x0, 0)
                //  pane_y0 = max (pane->y0, 0)
                //  pane_x1 = min (pane->x1, _W - 1)
                //  pane_y1 = min (pane->x1, (Ysize=ecx) - 1)

                mov     eax,[esi+4]		//x0 in Pane Coord
                mov     _CX,eax

	            cmp     eax,0
	            jg      around1
		        mov     eax,0
around1:
                mov     _L,eax
    
                mov     eax,[esi+8]		//y0 in Pane Coord
                mov     _CY,eax

	            cmp     eax,0
	            jg      around2
		        mov     eax,0
around2:
                mov     _T,eax
    
                mov     eax,[esi+12]	//X1 in Pane Coord
                mov     edx,_W
                dec     edx

				cmp     eax,edx
				jl      around3
				mov     eax,edx
around3:
                mov     _R,eax
    
                mov     eax,[esi+16]	//y1 in Pane Coord
                mov     edx,ecx
                dec     edx
				cmp     eax,edx
				jl      around4
				mov     eax,edx
around4:
                mov     _B,eax

                // exit if pane is malformed or completely off window:
                //  if _B < &vname&_T, return bad pane
                //  if _R < &vname&_L, return bad pane

                mov     eax,_R
                cmp     eax,_L
                jl      ReturnBadPane
    
                mov     eax,_B
                cmp     eax,_T
                jl      ReturnBadPane

                mov     eax,[ebx]		//Buffer in Window
                mov     _A,eax

                jmp     exit1

ReturnBadWindow:
                mov     eax,-1

				jmp		exit1
			}

			__asm
			{

ReturnBadPane:
                mov     eax,-2
			}

exit1:

			__asm
			{
				// transform x & y to window coord's

                mov     ecx,x
                mov     ebx,y

                //CONVERT_REG_PAIR_PANE_TO_WINDOW ecx, ebx
				
                add     ecx,_CX
                add     ebx,_CY

        		// clip pixel to pane
    
                cmp     ecx,_L
                jl      ReturnOffPane
                cmp     ecx,_R
                jg      ReturnOffPane
                cmp     ebx,_T
                jl      ReturnOffPane
                cmp     ebx,_B
                jg      ReturnOffPane

		        // adr (ebx) = window->buffer + CP_W * y + x

				//GET_WINDOW_ADDRESS  x0_, y0_

				mov     eax,ebx
				imul    _W
				add     eax,_A
				add     eax,ecx

        	    mov     edi,eax

				mov     ebx,eax

        		// write the pixel

                mov     dl,BYTE PTR color
                mov     [ebx],dl

        		// return prior_value

                ret

        		// error returns

ReturnOffPane:
                mov     eax,-3
                ret
				
			}
#endif // LINUX_BUILD			
			return(result);
}


//----------------------------------------------------------------------------
//
// int cdecl VFX_pixel_read (PANE *panep, int x, int y)
//
//     This function reads a single pixel.
//
// The panep parameter specifies the pane containing the pixel to be written.
// The x and y parameters specify the pixel coordinates.
//                                          
// Return values:
//
//    0..255:
//       Pixel value.
//
//   -1: Bad window.
//       The height or width of the pane's window is less than one.
//
//   -2: Bad pane.
//       The height or width of the pane is less than one.
//
//   -3: Off pane.
//      The specified pixel is off the pane.
//                                         
//---------------------------------------------------------------------------

LONG VFX_pixel_read (PANE *panep, LONG x, LONG y)
{
		int result = 0;
#ifdef LINUX_BUILD
	assert(0 && "VFX_line_draw is not implemented");
#else
		long		_L;	//Leftmost pixel in Window coord.
		long		_T;	//Top
		long		_R;	//Right
		long		_B;	//Bottom
	              
		MemoryPtr	_A;	//Base address of Clipped Pane
		long		_W;	//Width of underlying window (bytes)
	
		long		_CX;	//Window x coord. = Pane x coord. + CP_CX
		long		_CY;	//Window y coord. = Pane x coord. + CP_CY

		
		__asm
			{
				cld
			}

	//Clip Pane to Window Routine

		// get panep (esi)
        // windowp (ebx) = panep->win

		//ASSUME  esi:PPANE
		//ASSUME  ebx:PWIN

		__asm
			{
                mov     esi,panep
                mov     ebx,[esi]		//This is the Window Pointer

                // _W = windowp->wnd_x1 + 1
                // if _W <= 0, return bad window
    
                mov     eax,[ebx+4]		//X1 Window Coord
                inc     eax
                mov     _W,eax
                jle     ReturnBadWindow
    
                // ecx = Ysize = windowp->wnd_y1 + 1
                // if <= 0, return bad window

                mov     eax,[ebx+8]		//y1 Window Coord
                inc     eax
                mov     ecx,eax
                jle     ReturnBadWindow

                // clip pane to window:
                //  pane_x0 = max (pane->x0, 0)
                //  pane_y0 = max (pane->y0, 0)
                //  pane_x1 = min (pane->x1, _W - 1)
                //  pane_y1 = min (pane->x1, (Ysize=ecx) - 1)

                mov     eax,[esi+4]		//x0 in Pane Coord
                mov     _CX,eax

	            cmp     eax,0
	            jg      around1
		        mov     eax,0
around1:
                mov     _L,eax
    
                mov     eax,[esi+8]		//y0 in Pane Coord
                mov     _CY,eax

	            cmp     eax,0
	            jg      around2
		        mov     eax,0
around2:
                mov     _T,eax
    
                mov     eax,[esi+12]	//X1 in Pane Coord
                mov     edx,_W
                dec     edx

				cmp     eax,edx
				jl      around3
				mov     eax,edx
around3:
                mov     _R,eax
    
                mov     eax,[esi+16]	//y1 in Pane Coord
                mov     edx,ecx
                dec     edx
				cmp     eax,edx
				jl      around4
				mov     eax,edx
around4:
                mov     _B,eax

                // exit if pane is malformed or completely off window:
                //  if _B < &vname&_T, return bad pane
                //  if _R < &vname&_L, return bad pane

                mov     eax,_R
                cmp     eax,_L
                jl      ReturnBadPane
    
                mov     eax,_B
                cmp     eax,_T
                jl      ReturnBadPane

                mov     eax,[ebx]		//Buffer in Window
                mov     _A,eax

                jmp     exit1

ReturnBadWindow:
                mov     eax,-1

				jmp		exit1
			}

			__asm
			{

ReturnBadPane:
                mov     eax,-2
			}

exit1:

			__asm
			{
				// transform x & y to window coord's

                mov     ecx,x
                mov     ebx,y

                //CONVERT_REG_PAIR_PANE_TO_WINDOW ecx, ebx
				
                add     ecx,_CX
                add     ebx,_CY

        		// clip pixel to pane
    
                cmp     ecx,_L
                jl      ReturnOffPane
                cmp     ecx,_R
                jg      ReturnOffPane
                cmp     ebx,_T
                jl      ReturnOffPane
                cmp     ebx,_B
                jg      ReturnOffPane

		        // adr (ebx) = window->buffer + CP_W * y + x

				//GET_WINDOW_ADDRESS  x0_, y0_

				mov     eax,ebx
				imul    _W
				add     eax,_A
				add     eax,ecx

        	    mov     edi,eax

				mov     ebx,eax

        		// read and return the pixel
    
                xor     eax,eax
                mov     al,[ebx]
                ret

				// error returns

ReturnOffPane:
                mov     eax,-3
				mov 	result, eax
                ret
			}
#endif // LINUX_BUILD	
	return(result);
}
