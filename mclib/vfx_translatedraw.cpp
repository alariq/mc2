#include"dstd.h"
#include"vfx.h"
#include<stdio.h>

extern char AlphaTable[256*256];

typedef struct
{

	DWORD	bounds;
	DWORD	origin;
	DWORD	xmin;
	DWORD	ymin;
	DWORD	xmax;
	DWORD	ymax;

} SHAPEHEADER;

//
// Sprite format
//
//
// Marker.
// 0			End of line
// 1			Skip next bytes
// Bit 0 = 0	Repeat next byte [7654321] times
// Bit 0 = 1	String packet [7654321] bytes
//

unsigned int lookaside;
static unsigned int tempXmax,tempXmin;
static unsigned int minX,minY,maxY,SkipLeft,NewWidth,StartofLine,StartofClip,EndofClip;
static unsigned int lines,DestWidth,paneX0,paneX1,paneY0,paneY1;




/*
;
; int cdecl VFX_shape_draw (PANE *panep, void *shape_table,
;                           long shape_number,int hotX, int hotY)
;
; This function clips and draws a shape to a pane.
; 
; The panep parameter specifies the pane.
;
; The shape parameter specifies the shape, which must be in VFX Shape format.
;
; The hotX and hotY parameters specify the location where the shape is to be
; drawn.  The shape's hot spot will end up at the specified location.
;
*/
void AG_shape_draw (PANE *pane, void *shape_table,LONG shape_number, LONG hotX, LONG hotY)
{
#ifdef LINUX_BUILD
#else
	_asm{
	mov esi,shape_table
	mov eax,shape_number

	mov ecx,[esi+eax*8+8]
	cmp word ptr [esi+ecx+SIZE SHAPEHEADER],3		; (String packet, 0)
	jz AlphaDraw

	mov edi,pane
	nop
;
; Clip left and right of clipping window
;
	mov ecx,[edi+PANE.x0]
	mov edx,[edi+PANE.y0]

	mov ebx,ecx
	mov eax,edx

	sar ebx,31
	xor eax,-1

	sar eax,31				; If less than 0, make 0
	xor ebx,-1

	and ecx,ebx
	and edx,eax

	mov paneX0,ecx
	mov paneY0,edx

;
; Clip top and bottom of clipping window
;
	mov ecx,[edi+PANE.x1]
	mov edx,[edi+PANE.y1]

	mov edi,[edi+PANE.window]
	mov esi,shape_table

	mov eax,[edi+WINDOW.x_max]
	xor ebx,ebx

	inc eax
	nop

	sub ecx,eax
	mov DestWidth,eax

	setge bl

	dec ebx					; if ecx is less than eax, load ecx with eax
	nop

	and ecx,ebx
	mov ebx,[edi+WINDOW.y_max]

	add ecx,eax
	inc ebx

	xor eax,eax
	sub edx,ebx

	setge al

	dec eax
	mov paneX1,ecx
	
	and edx,eax
	mov eax,shape_number

	add edx,ebx
	mov ebx,paneY0

;
; paneX0,Y0 to PaneX1,Y1 are 0,0 -> 639,479 or window size to render too
;
	mov ecx,[esi+eax*8+8]	; ESI now points to start of sprite data
	mov paneY1,edx

	lea esi,[esi+ecx+SIZE SHAPEHEADER]
	mov edx,hotY

	mov ecx,[esi+SHAPEHEADER.xmax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.xmin-SIZE SHAPEHEADER]

	mov tempXmax,ecx					; Store Xmax and Xmin
	mov tempXmin,eax

	mov ecx,[esi+SHAPEHEADER.ymax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.ymin-SIZE SHAPEHEADER]

	sub ecx,eax							; ecx = Height of sprite
	add eax,edx							; eax = top line

	inc ecx								; Add one line
	nop
;
; Now check for lines off the top of the clipping window
;
	cmp eax,ebx
	jl ClippedTop

;
; Now check for lines off the bottom of the clipping window
;
rw5:
	lea ebx,[eax+ecx-1]					; ebx=Last Line
	nop

	sub ebx,paneY1						; Check to see if off bottom
	ja ClippedBottom
rw6:
	mov lines,ecx						; eax still equals top line

;
; Now check clipping in X
;
	mov ebx,tempXmax

	mov ecx,tempXmin
	mov edx,paneX0

	sub ebx,ecx							; ebx = Width of sprite
	add ecx,hotX						; ecx = offset to left edge

	cmp ecx,edx							; Is sprite off left edge of screen?
	jl ClippedLeft

	lea edx,[ebx+ecx]
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer

	sub edx,paneX1
	jnbe ClippedRight

;
; Work out screen position
;
//NowDraw:
	imul DestWidth

	add eax,ecx
	xor ecx,ecx

	add edi,eax
	mov StartofLine,edi

;
;
; Main drawing loop
;
;
lineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja RunPacket
	jnz StringPacket
	jnc EndPacket

//SkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx
	jmp lineLoop

RunPacket:
	mov cl,[esi]
	inc esi
rp1:
	mov [edi],cl
	inc edi
	dec al
	jnz rp1
	jmp lineLoop

StringPacket:
//
// 17 cycles / 8 bytes   - 2.125 per byte
//
	sub al,8
	jc sp2
sp1:
	mov ecx,[esi]
	mov ebx,[esi+4]

	mov [edi],ecx
	mov [edi+4],ebx

	add esi,8
	add edi,8

	sub al,8
	jnc sp1

sp2:
	add al,8
	jz lineLoop

sp3:
	mov cl,[esi]
	inc esi

	mov [edi],cl
	inc edi

	dec al
	jnz sp3
	jmp lineLoop

EndPacket:
	mov edx,DestWidth
	mov edi,StartofLine

	add edi,edx
	mov edx,lines

	dec edx
	mov StartofLine,edi

	mov lines,edx
	jnz lineLoop
	jmp Exit




;
;
; Lines are clipped off the bottom of the clip window
;
;
ClippedBottom:
	sub ecx,ebx							; Remove lines that go off bottom
	jbe Exit
	jmp rw6
;
;
; Lines are off the top of the clip window
;
;
ClippedTop:
	sub ebx,eax							; ebx=Lines to skip
	xor eax,eax
	sub ecx,ebx							; ecx=Lines in sprite
	jbe Exit							; Off top of screen
rw8:
	mov al,[esi]
	add esi,2
	shr al,1
	ja rw8
	jnz rw8StringPacket					; Skip over lines in sprite data
	jc rw8
	dec esi
	dec ebx
	jnz rw8
	mov eax,paneY0						; eax=top line
	jmp rw5
rw8StringPacket:
	lea esi,[esi+eax-1]
	jmp rw8
;
;
; Sprite is clipped on either left or right
;
;
ClippedLeft:
	sub edx,ecx
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer
	cmp ebx,edx
	jbe Exit							; Completly off left of screen?
	jmp ClippedX

ClippedRight:
	cmp ecx,edx							; Completly off right of screen?
	jbe Exit

ClippedX:
	imul DestWidth

	add edi,eax
	mov eax,paneX0

	mov edx,paneX1
	add eax,edi

	add edx,edi							; eax=Last pixel on right edge of clip window
	mov ebx,eax

	add edi,ecx

	xor ecx,ecx
	mov StartofLine,edi

;
; Clipped left and/or right drawing loop
;
clineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja cRunPacket
	jnz cStringPacket
	jnc cEndPacket


//cSkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx
	jmp clineLoop


cRunPacket:
	mov cl,[esi]
	inc esi
crp1:
	cmp edi,ebx
	jc crp2
	cmp edi,edx
	jnbe clineLoop
	mov [edi],cl
crp2:
	inc edi
	dec al
	jnz crp1
	jmp clineLoop


cStringPacket:
	mov cl,[esi]
	inc esi
	cmp edi,ebx
	jc crp3
	cmp edi,edx
	jnbe crp3a
	mov [edi],cl
crp3:
	inc edi
	dec al
	jnz cStringPacket
	jmp clineLoop

crp3a:
	and eax,255
	lea esi,[esi+eax-1]
	jmp clineLoop


cEndPacket:
	mov eax,DestWidth
	mov edi,StartofLine

	add edx,eax
	add edi,eax

	add ebx,eax
	mov eax,lines

	mov StartofLine,edi
	dec eax

	mov lines,eax
	jnz clineLoop

	jmp Exit










//
//
// Same routine as above, but using Alpha table
//
//
AlphaDraw:
	mov edi,pane
	nop
;
; Clip left and right of clipping window
;
	mov ecx,[edi+PANE.x0]
	mov edx,[edi+PANE.y0]

	mov ebx,ecx
	mov eax,edx

	sar ebx,31
	xor eax,-1

	sar eax,31				; If less than 0, make 0
	xor ebx,-1

	and ecx,ebx
	and edx,eax

	mov paneX0,ecx
	mov paneY0,edx

;
; Clip top and bottom of clipping window
;
	mov ecx,[edi+PANE.x1]
	mov edx,[edi+PANE.y1]

	mov edi,[edi+PANE.window]
	mov esi,shape_table

	mov eax,[edi+WINDOW.x_max]
	xor ebx,ebx

	inc eax
	nop

	sub ecx,eax
	mov DestWidth,eax

	setge bl

	dec ebx					; if ecx is less than eax, load ecx with eax
	nop

	and ecx,ebx
	mov ebx,[edi+WINDOW.y_max]

	add ecx,eax
	inc ebx

	xor eax,eax
	sub edx,ebx

	setge al

	dec eax
	mov paneX1,ecx
	
	and edx,eax
	mov eax,shape_number

	add edx,ebx
	mov ebx,paneY0

;
; paneX0,Y0 to PaneX1,Y1 are 0,0 -> 639,479 or window size to render too
;
	mov ecx,[esi+eax*8+8]	; ESI now points to start of sprite data
	mov paneY1,edx

	lea esi,[esi+ecx+SIZE SHAPEHEADER]
	mov edx,hotY

	mov ecx,[esi+SHAPEHEADER.xmax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.xmin-SIZE SHAPEHEADER]

	mov tempXmax,ecx					; Store Xmax and Xmin
	mov tempXmin,eax

	mov ecx,[esi+SHAPEHEADER.ymax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.ymin-SIZE SHAPEHEADER]

	sub ecx,eax							; ecx = Height of sprite
	add eax,edx							; eax = top line

	inc ecx								; Add one line
	nop
;
; Now check for lines off the top of the clipping window
;
	cmp eax,ebx
	jl aClippedTop

;
; Now check for lines off the bottom of the clipping window
;
arw5:
	lea ebx,[eax+ecx]					; ebx=Last Line
	nop

	sub ebx,paneY1						; Check to see if off bottom
	ja aClippedBottom
arw6:
	mov lines,ecx						; eax still equals top line

;
; Now check clipping in X
;
	mov ebx,tempXmax

	mov ecx,tempXmin
	mov edx,paneX0

	sub ebx,ecx							; ebx = Width of sprite
	add ecx,hotX						; ecx = offset to left edge

	inc ecx
	nop
	
	cmp ecx,edx							; Is sprite off left edge of screen?
	jl aClippedLeft

	lea edx,[ebx+ecx]
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer

	sub edx,paneX1
	ja aClippedRight

;
; Work out screen position
;
//aNowDraw:
	imul DestWidth

	add eax,ecx
	xor ecx,ecx

	add edi,eax
	mov StartofLine,edi

;
;
; Main drawing loop
;
;
alineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja aRunPacket
	jnz aStringPacket
	jnc aEndPacket

//aSkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx
	jmp alineLoop

aRunPacket:
	mov ch,[esi]
	inc esi
arp1:
	mov cl,[edi]
	inc edi
	mov cl,AlphaTable[ecx]
	dec al
	mov [edi-1],cl
	jnz arp1

	jmp alineLoop

aStringPacket:
	mov ch,[esi]
	inc esi

	mov cl,[edi]
	inc edi

	mov cl,AlphaTable[ecx]
	dec al

	mov [edi-1],cl
	jnz aStringPacket

	jmp alineLoop

aEndPacket:
	mov edx,DestWidth
	mov edi,StartofLine

	add edi,edx
	mov edx,lines

	dec edx
	mov StartofLine,edi

	mov lines,edx
	jnz alineLoop
	jmp Exit




;
;
; Lines are clipped off the bottom of the clip window
;
;
aClippedBottom:
	sub ecx,ebx							; Remove lines that go off bottom
	jbe Exit
	jmp arw6
;
;
; Lines are off the top of the clip window
;
;
aClippedTop:
	sub ebx,eax							; ebx=Lines to skip
	xor eax,eax
	sub ecx,ebx							; ecx=Lines in sprite
	jbe Exit							; Off top of screen
arw8:
	mov al,[esi]
	add esi,2
	shr al,1
	ja arw8
	jnz arw8StringPacket					; Skip over lines in sprite data
	jc arw8
	dec esi
	dec ebx
	jnz arw8
	mov eax,paneY0						; eax=top line
	jmp arw5
arw8StringPacket:
	lea esi,[esi+eax-1]
	jmp arw8
;
;
; Sprite is clipped on either left or right
;
;
aClippedLeft:
	sub edx,ecx
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer
	cmp ebx,edx
	jbe Exit							; Completly off left of screen?
	jmp aClippedX

aClippedRight:
	cmp ecx,edx							; Completly off right of screen?
	jbe Exit

aClippedX:
	imul DestWidth

	add edi,eax
	mov eax,paneX0

	mov edx,paneX1
	add eax,edi

	add edx,edi							; eax=Last pixel on right edge of clip window
	mov ebx,eax

	add edi,ecx
	mov StartofLine,edi

	xor ecx,ecx
;
; Clipped left and/or right drawing loop
;
aclineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja acRunPacket
	jnz acStringPacket
	jnc acEndPacket

//acSkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx
	jmp aclineLoop

acRunPacket:
	mov ch,[esi]
	inc esi
acrp1:
	cmp edi,ebx
	jc acrp2
	cmp edi,edx
	jnbe aclineLoop
	mov cl,[edi]
	mov cl,AlphaTable[ecx]
	mov [edi],cl
acrp2:
	inc edi
	dec al
	jnz acrp1
	jmp aclineLoop


acStringPacket:
	mov ch,[esi]
	inc esi
	cmp edi,ebx
	jc acrp3
	cmp edi,edx
	jnbe acrp3a
	mov cl,[edi]
	mov cl,AlphaTable[ecx]
	mov [edi],cl
acrp3:
	inc edi
	dec al
	jnz acStringPacket
	jmp aclineLoop

acrp3a:
	and eax,255
	lea esi,[esi+eax-1]
	jmp aclineLoop


acEndPacket:
	mov eax,DestWidth
	mov edi,StartofLine

	add edx,eax
	add edi,eax

	add ebx,eax
	mov eax,lines

	mov StartofLine,edi
	dec eax

	mov lines,eax
	jnz aclineLoop


Exit:       
	}
#endif //LINUX_BUILD
}






































/*
;----------------------------------------------------------------------------
;
; void cdecl VFX_shape_lookaside (unsigned char *table)
;
; Establishes a color translation lookaside table for use by future calls
; to VFX_shape_translate_draw().
; 
; table points to a 256-byte table specifying remap values for each of
; the 256 possible palette indices.  The table is copied to static local 
; memory, and need not remain valid after the call.
;
;----------------------------------------------------------------------------
*/
void AG_shape_lookaside( UBYTE *table )
{
#ifdef LINUX_BUILD
#else

	_asm{
		mov eax,table
		mov lookaside,eax
		}
#endif 
}

/*
;----------------------------------------------------------------------------
;
; int cdecl VFX_shape_translate_draw (PANE *panep, void *shape_table,
;                           long shape_number,int hotX, int hotY)
;
; This function clips and draws a shape to a pane.  It is identical to 
; VFX_shape_draw(), except that each pixel written is translated through a
; 256-byte table which was specified by a prior call to VFX_shape_lookaside().
; 
; The panep parameter specifies the pane.
;
; The shape parameter specifies the shape, which must be in VFX Shape format.
;
; The hotX and hotY parameters specify the location where the shape is to be
; drawn.  The shape's hot spot will end up at the specified location.
;
; For more information, see the "VFX Shape Format Description".
;
; Return values:
;
;    0: OK
;   -1: Bad window
;   -2: Bad pane
;   -3: Shape off pane
;   -4: Null shape
;
;----------------------------------------------------------------------------
*/
void AG_shape_translate_draw (PANE *pane, void *shape_table,LONG shape_number, LONG hotX, LONG hotY)
{
#ifdef LINUX_BUILD
#else
	_asm{
	mov esi,shape_table
	mov eax,shape_number

	mov ecx,[esi+eax*8+8]
	cmp word ptr [esi+ecx+SIZE SHAPEHEADER],3		; (String packet, 0)
	jz AlphaDraw

	mov edi,pane
	nop
;
; Clip left and right of clipping window
;
	mov ecx,[edi+PANE.x0]
	mov edx,[edi+PANE.y0]

	mov ebx,ecx
	mov eax,edx

	sar ebx,31
	xor eax,-1

	sar eax,31				; If less than 0, make 0
	xor ebx,-1

	and ecx,ebx
	and edx,eax

	mov paneX0,ecx
	mov paneY0,edx

;
; Clip top and bottom of clipping window
;
	mov ecx,[edi+PANE.x1]
	mov edx,[edi+PANE.y1]

	mov edi,[edi+PANE.window]
	mov esi,shape_table

	mov eax,[edi+WINDOW.x_max]
	xor ebx,ebx

	inc eax
	nop

	sub ecx,eax
	mov DestWidth,eax

	setge bl

	dec ebx					; if ecx is less than eax, load ecx with eax
	nop

	and ecx,ebx
	mov ebx,[edi+WINDOW.y_max]

	add ecx,eax
	inc ebx

	xor eax,eax
	sub edx,ebx

	setge al

	dec eax
	mov paneX1,ecx
	
	and edx,eax
	mov eax,shape_number

	add edx,ebx
	mov ebx,paneY0

;
; paneX0,Y0 to PaneX1,Y1 are 0,0 -> 639,479 or window size to render too
;
	mov ecx,[esi+eax*8+8]	; ESI now points to start of sprite data
	mov paneY1,edx

	lea esi,[esi+ecx+SIZE SHAPEHEADER]
	mov edx,hotY

	mov ecx,[esi+SHAPEHEADER.xmax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.xmin-SIZE SHAPEHEADER]

	mov tempXmax,ecx					; Store Xmax and Xmin
	mov tempXmin,eax

	mov ecx,[esi+SHAPEHEADER.ymax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.ymin-SIZE SHAPEHEADER]

	sub ecx,eax							; ecx = Height of sprite
	add eax,edx							; eax = top line

	inc ecx								; Add one line
	nop
;
; Now check for lines off the top of the clipping window
;
	cmp eax,ebx
	jl ClippedTop

;
; Now check for lines off the bottom of the clipping window
;
rw5:
	lea ebx,[eax+ecx-1]					; ebx=Last Line
	nop

	sub ebx,paneY1						; Check to see if off bottom
	ja ClippedBottom
rw6:
	mov lines,ecx						; eax still equals top line

;
; Now check clipping in X
;
	mov ebx,tempXmax

	mov ecx,tempXmin
	mov edx,paneX0

	sub ebx,ecx							; ebx = Width of sprite
	add ecx,hotX						; ecx = offset to left edge

	cmp ecx,edx							; Is sprite off left edge of screen?
	jl ClippedLeft

	lea edx,[ebx+ecx-1]
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer

	sub edx,paneX1
	jnb ClippedRight

;
; Work out screen position
;
//NowDraw:
	imul DestWidth

	add eax,ecx
	xor ecx,ecx

	add edi,eax
	mov StartofLine,edi

	push ebp
	mov ebp,lookaside

	xor ebx,ebx
	nop
;
; Main drawing loop
;
;
lineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja RunPacket
	jnz StringPacket
	jnc EndPacket

//SkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx
	jmp lineLoop

RunPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	mov cl,[ecx+ebp]
rp1:
	mov [edi],cl
	inc edi
	dec al
	jnz rp1
	jmp lineLoop

StringPacket:
//
// 17 cycles / 8 bytes   - 2.125 per byte
//
	sub al,8
	jc sp2
sp1:
	mov cl,[esi]
	mov bl,[esi+4]

	mov cl,[ecx+ebp]
	mov bl,[ebx+ebp]

	mov [edi],cl
	mov [edi+4],bl

	mov cl,[esi+1]
	mov bl,[esi+5]

	mov cl,[ecx+ebp]
	mov bl,[ebx+ebp]

	mov [edi+1],cl
	mov [edi+5],bl

	mov cl,[esi+2]
	mov bl,[esi+6]

	mov cl,[ecx+ebp]
	mov bl,[ebx+ebp]

	mov [edi+2],cl
	mov [edi+6],bl

	mov cl,[esi+3]
	mov bl,[esi+7]

	add esi,8
	add edi,8

	mov cl,[ecx+ebp]
	mov bl,[ebx+ebp]

	mov [edi+3-8],cl
	mov [edi+7-8],bl

	sub al,8
	jnc sp1

sp2:
	add al,8
	jz lineLoop

sp3:
	xor ecx,ecx
	mov cl,[esi]

	mov cl,[ecx+ebp]
	inc esi

	mov [edi],cl
	inc edi

	dec al
	jnz sp3
	jmp lineLoop

EndPacket:
	mov edx,DestWidth
	mov edi,StartofLine

	add edi,edx
	mov edx,lines

	dec edx
	mov StartofLine,edi

	mov lines,edx
	jnz lineLoop

	pop ebp
	jmp Exit




;
;
; Lines are clipped off the bottom of the clip window
;
;
ClippedBottom:
	sub ecx,ebx							; Remove lines that go off bottom
	jbe Exit
	jmp rw6
;
;
; Lines are off the top of the clip window
;
;
ClippedTop:
	sub ebx,eax							; ebx=Lines to skip
	xor eax,eax
	sub ecx,ebx							; ecx=Lines in sprite
	jbe Exit							; Off top of screen
rw8:
	mov al,[esi]
	add esi,2
	shr al,1
	ja rw8
	jnz rw8StringPacket					; Skip over lines in sprite data
	jc rw8
	dec esi
	dec ebx
	jnz rw8
	mov eax,paneY0						; eax=top line
	jmp rw5
rw8StringPacket:
	lea esi,[esi+eax-1]
	jmp rw8
;
;
; Sprite is clipped on either left or right
;
;
ClippedLeft:
	sub edx,ecx
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer
	cmp ebx,edx
	jbe Exit							; Completly off left of screen?
	jmp ClippedX

ClippedRight:
	cmp ecx,edx							; Completly off right of screen?
	jbe Exit

ClippedX:
	imul DestWidth

	add edi,eax
	mov eax,paneX0

	mov edx,paneX1
	add eax,edi

	add edx,edi							; eax=Last pixel on right edge of clip window
	mov StartofClip,eax

	mov EndofClip,edx
	add edi,ecx

	xor ecx,ecx
	mov StartofLine,edi

	push ebp
	mov ebp,lookaside
;
; Clipped left and/or right drawing loop
;
clineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja cRunPacket
	jnz cStringPacket
	jnc cEndPacket


//cSkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx

//DEBUG
	push edi
	shr	edi,28
	cmp edi,8
	pop edi
	je clineLoop
//END DEBUG
	jmp	assertError	//clineLoop


cRunPacket:
	mov cl,[esi]
	inc esi
crp1:
	cmp edi,StartofClip
	jc crp2
	cmp edi,EndofClip
	jnbe clineLoop
	mov cl,[ecx+ebp]
	mov [edi],cl
crp2:
	inc edi
	dec al
	jnz crp1
	jmp clineLoop


cStringPacket:
	mov cl,[esi]
	inc esi
	cmp edi,StartofClip
	jc crp3
	cmp edi,EndofClip
	jnbe crp3a
	mov cl,[ecx+ebp]
	mov [edi],cl
crp3:
	inc edi
	dec al
	jnz cStringPacket
	jmp clineLoop

crp3a:
	and eax,255
	lea esi,[esi+eax-1]
	jmp clineLoop


cEndPacket:
	mov edx,DestWidth
	mov eax,EndofClip

	add eax,edx
	mov edi,StartofLine

	mov EndofClip,eax
	nop

	mov eax,StartofClip
	add edi,edx

	add eax,edx
	mov edx,lines

	mov StartofLine,edi
	dec edx

	mov StartofClip,eax
	mov lines,edx

	jnz clineLoop
	pop ebp
	jmp Exit


//
//
// Same routine as above, but with Alpha Table effects
//
//
AlphaDraw:
	mov edi,pane
	nop
;
; Clip left and right of clipping window
;
	mov ecx,[edi+PANE.x0]
	mov edx,[edi+PANE.y0]

	mov ebx,ecx
	mov eax,edx

	sar ebx,31
	xor eax,-1

	sar eax,31				; If less than 0, make 0
	xor ebx,-1

	and ecx,ebx
	and edx,eax

	mov paneX0,ecx
	mov paneY0,edx

;
; Clip top and bottom of clipping window
;
	mov ecx,[edi+PANE.x1]
	mov edx,[edi+PANE.y1]

	mov edi,[edi+PANE.window]
	mov esi,shape_table

	mov eax,[edi+WINDOW.x_max]
	xor ebx,ebx

	inc eax
	nop

	sub ecx,eax
	mov DestWidth,eax

	setge bl

	dec ebx					; if ecx is less than eax, load ecx with eax
	nop

	and ecx,ebx
	mov ebx,[edi+WINDOW.y_max]

	add ecx,eax
	inc ebx

	xor eax,eax
	sub edx,ebx

	setge al

	dec eax
	mov paneX1,ecx
	
	and edx,eax
	mov eax,shape_number

	add edx,ebx
	mov ebx,paneY0

;
; paneX0,Y0 to PaneX1,Y1 are 0,0 -> 639,479 or window size to render too
;
	mov ecx,[esi+eax*8+8]	; ESI now points to start of sprite data
	mov paneY1,edx

	lea esi,[esi+ecx+SIZE SHAPEHEADER]
	mov edx,hotY

	mov ecx,[esi+SHAPEHEADER.xmax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.xmin-SIZE SHAPEHEADER]

	mov tempXmax,ecx					; Store Xmax and Xmin
	mov tempXmin,eax

	mov ecx,[esi+SHAPEHEADER.ymax-SIZE SHAPEHEADER]
	mov eax,[esi+SHAPEHEADER.ymin-SIZE SHAPEHEADER]

	sub ecx,eax							; ecx = Height of sprite
	add eax,edx							; eax = top line

	inc ecx								; Add one line
	nop
;
; Now check for lines off the top of the clipping window
;
	cmp eax,ebx
	jl aClippedTop

;
; Now check for lines off the bottom of the clipping window
;
arw5:
	lea ebx,[eax+ecx-1]					; ebx=Last Line
	nop

	sub ebx,paneY1						; Check to see if off bottom
	ja aClippedBottom
//arw6:
	mov lines,ecx						; eax still equals top line

;
; Now check clipping in X
;
	mov ebx,tempXmax

	mov ecx,tempXmin
	mov edx,paneX0

	sub ebx,ecx							; ebx = Width of sprite
	add ecx,hotX						; ecx = offset to left edge

	cmp ecx,edx							; Is sprite off left edge of screen?
	jl aClippedLeft

	lea edx,[ebx+ecx-1]
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer

	sub edx,paneX1
	jnbe aClippedRight

;
; Work out screen position
;
//aNowDraw:
	imul DestWidth

	add eax,ecx
	xor ecx,ecx

	add edi,eax
	mov StartofLine,edi

	push ebp
	mov ebp,lookaside

	xor ebx,ebx
	nop
;
; Main drawing loop
;
;
alineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja aRunPacket
	jnz aStringPacket
	jnc aEndPacket

//aSkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx
	jmp alineLoop

aRunPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	mov ch,[ecx+ebp]
arp1:
	mov cl,[edi]
	inc edi
	mov cl,AlphaTable[ecx]
	dec al
	mov [edi-1],cl
	jnz arp1
	jmp alineLoop

aStringPacket:
	xor ecx,ecx
	mov cl,[esi]

	mov ch,[ecx+ebp]
	inc esi

	mov cl,[edi]
	inc edi

	mov cl,AlphaTable[ecx]
	dec al

	mov [edi-1],cl
	jnz aStringPacket
	jmp alineLoop

aEndPacket:
	mov edx,DestWidth
	mov edi,StartofLine

	add edi,edx
	mov edx,lines

	dec edx
	mov StartofLine,edi

	mov lines,edx
	jnz alineLoop

	pop ebp
	jmp Exit




;
;
; Lines are clipped off the bottom of the clip window
;
;
aClippedBottom:
	sub ecx,ebx							; Remove lines that go off bottom
	jbe Exit
	jmp rw6
;
;
; Lines are off the top of the clip window
;
;
aClippedTop:
	sub ebx,eax							; ebx=Lines to skip
	xor eax,eax
	sub ecx,ebx							; ecx=Lines in sprite
	jbe Exit							; Off top of screen
arw8:
	mov al,[esi]
	add esi,2
	shr al,1
	ja arw8
	jnz arw8StringPacket					; Skip over lines in sprite data
	jc arw8
	dec esi
	dec ebx
	jnz arw8
	mov eax,paneY0						; eax=top line
	jmp arw5
arw8StringPacket:
	lea esi,[esi+eax-1]
	jmp arw8
;
;
; Sprite is clipped on either left or right
;
;
aClippedLeft:
	sub edx,ecx
	mov edi,[edi+WINDOW.buffer]			; edi points to top left of buffer
	cmp ebx,edx
	jbe Exit							; Completly off left of screen?
	jmp aClippedX

aClippedRight:
	cmp ecx,edx							; Completly off right of screen?
	jbe Exit

aClippedX:
	imul DestWidth

	add edi,eax
	mov eax,paneX0

	mov edx,paneX1
	add eax,edi

	add edx,edi							; eax=Last pixel on right edge of clip window
	mov StartofClip,eax

	mov EndofClip,edx
	add edi,ecx

	xor ecx,ecx
	mov StartofLine,edi

	push ebp
	mov ebp,lookaside
;
; Clipped left and/or right drawing loop
;
aclineLoop:
	mov al,[esi]
	inc esi
	shr al,1
	ja acRunPacket
	jnz acStringPacket
	jnc acEndPacket

//acSkipPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	add edi,ecx
	jmp aclineLoop

acRunPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	mov ch,[ecx+ebp]
acrp1:
	cmp edi,StartofClip
	jc acrp2
	cmp edi,EndofClip
	jnbe aclineLoop
	mov cl,[edi]
	mov cl,AlphaTable[ecx]
	mov [edi],cl
acrp2:
	inc edi
	dec al
	jnz acrp1
	jmp aclineLoop


acStringPacket:
	xor ecx,ecx
	mov cl,[esi]
	inc esi
	cmp edi,StartofClip
	jc acrp3
	cmp edi,EndofClip
	jnbe acrp3a
	mov ch,[ecx+ebp]
	mov cl,[edi]
	mov cl,AlphaTable[ecx]
	mov [edi],cl
acrp3:
	inc edi
	dec al
	jnz acStringPacket
	jmp aclineLoop

acrp3a:
	and eax,255
	lea esi,[esi+eax-1]
	jmp aclineLoop


acEndPacket:
	mov edx,DestWidth
	mov eax,EndofClip

	add eax,edx
	mov edi,StartofLine

	mov EndofClip,eax
	nop

	mov eax,StartofClip
	add edi,edx

	add eax,edx
	mov edx,lines

	mov StartofLine,edi
	dec edx

	mov StartofClip,eax
	mov lines,edx

	jnz aclineLoop
	pop ebp
	jmp Exit


assertError:
	pop ebp
	
#ifdef CATCH_VFX_BUG
	mov saveEdi,edi
	}

	//void *shape_table,LONG shape_number, LONG hotX, LONG hotY
	
	char msg[1024];
	
	//-------------------------------------
	// Save off the shape data table start
	char *shapeTable = (char *)shape_table;
	char version[5];
	for (long i=0;i<4;i++)
	{
		version[i] = *shapeTable;
		shapeTable++;
	}
	version[4] = 0;
	
	long numShapes = (long)*shapeTable;
	shapeTable += 4;
	shapeTable += (shape_number * 8);
	long shapeNumOffset = (long) *shapeTable;
	
	sprintf(msg,"SP: %08x  SF: %d  X: %d  Y:%d  V: %s  SN: %d  SOf: %d",shape_table,shape_number,hotX,hotY,version,numShapes,shapeNumOffset);
	Fatal(saveEdi,msg);
#else
	}
#endif
	__asm
	{
Exit:       
	}
#endif // LINUX_BUILD
}





