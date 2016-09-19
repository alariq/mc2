#include"vfx.h"

//extern enum { CPU_UNKNOWN, CPU_PENTIUM, CPU_MMX, CPU_KATMAI } Processor;

extern char AlphaTable[];
static unsigned int SourceWidth,tWidth,tHeight,DestWidth;			// Used for code optimizing
//static _int64 xmask=-1;



//
//
// Draws a status bar
//
//
void AG_StatusBar( PANE *pane, int X0, int Y0, int X1, int Y1, int Color, int Width )
{
	static long TopY,BottomY;

	DestWidth = pane->window->x_max+1;

	long paneX0 = (pane->x0 < 0) ? 0 : pane->x0;
	long paneY0 = (pane->y0 < 0) ? 0 : pane->y0;
	long paneX1 = (pane->x1 >= (long)DestWidth) ? pane->window->x_max : pane->x1;
	long paneY1 = (pane->y1 >= (pane->window->y_max+1)) ? pane->window->y_max : pane->y1;

	if (X0>X1)
	{
		Width=-Width;
		TopY=X0;
		X0=X1;
		X1=TopY;
	}
	if (Y0>Y1)
	{
		TopY=Y0;
		Y0=Y1;
		Y1=TopY;
	}

	TopY=Y0;
	BottomY=Y1;

	if ( ( X0 >= paneX1 ) ||
		 ( Y0 >= paneY1 ) ||
		 ( X1 <= paneX0) ||
		 ( Y1 <= paneY0))
		return;

	if( X0<paneX0 )
	{
		Width-=paneX0-X0;
		X0=paneX0;
	}

	if( Y0<paneY0 )
		Y0=paneY0;
		
	if( X1>paneX1 )
		X1=paneX1;

	if( Y1>paneY1 )
		Y1=paneY1;

	if( (X0+Width) >= X1 )
		Width=X1-X0-2;

	if( Width<0 )
		Width=0;

	if( X1-X0 < 3)
		return;

	UBYTE* DestPointer = pane->window->buffer + X0 + Y0*DestWidth;
#ifdef LINUX_BUILD
#else
	_asm{

	mov esi,Color
	mov edi,DestPointer

	sal esi,8
	mov ebx,X1

	add esi,offset AlphaTable
	sub ebx,X0

	mov ecx,Y0

lp0:
	mov eax,TopY
	mov edx,BottomY

	cmp ecx,eax
	jz lp1
	cmp ecx,edx
	jz lp1
//
// Middle lines
//
//lp2:
	xor eax,eax
	xor edx,edx

	mov al,[edi]
	mov dl,[edi+ebx]

	mov al,[eax+(AlphaTable+S_BLACK*256)]
	mov dl,[edx+(AlphaTable+S_BLACK*256)]

	mov [edi],al
	mov [edi+ebx],dl

	mov edx,Width
	and edx,edx

	jz lp3
	inc edx
	push edi

lp4:
	mov al,[edi+1]
	inc edi

	mov al,[esi+eax]
	dec edx

	mov [edi],al
	jnz lp4

	pop edi
	jmp lp3
//
// Top or bottom line
//
lp1:
	push edi
	mov edx,ebx

	xor eax,eax
	dec edx

lp5:
	mov al,[edi+1]
	inc edi

	mov al,[eax+(AlphaTable+S_BLACK*256)]
	dec edx

	mov [edi],al
	jnz lp5

	pop edi
	nop

lp3:
	mov eax,DestWidth
	mov edx,Y1

	add edi,eax
	inc ecx

	cmp edx,ecx
	jge lp0
	}
#endif // LINUX_BUILD
}





//
//
// Writes a single pixel
//
//
void AG_pixel_write (PANE *pane, LONG x, LONG y, ULONG color)
{
	int X=x+pane->x0;
	int Y=y+pane->y0;

	if( X>pane->x0 && X<pane->x1 && Y>pane->y0 && Y<pane->y1 )
		*(pane->window->buffer + X + Y*(pane->window->x_max+1))=(unsigned char)color;
}



//
//
// Draws a sprite - any colors marked 255 will be drawn transparent
//
//
//
long DrawTransparent( PANE *pane, WINDOW *texture, int X, int Y, int Width, int Height )
{
	DestWidth = pane->window->x_max+1;

	long paneX0 = (pane->x0 < 0) ? 0 : pane->x0;
	long paneY0 = (pane->y0 < 0) ? 0 : pane->y0;
	long paneX1 = (pane->x1 >= (long)DestWidth) ? pane->window->x_max : pane->x1;
	long paneY1 = (pane->y1 >= (pane->window->y_max+1)) ? pane->window->y_max : pane->y1;

	X+=paneX0;
	Y+=paneY0;
	
	if ( ( X >= paneX1 ) ||
		 ( Y >= paneY1 ) ||
		 ( X <= (paneX0 - Width)) ||
		 ( Y <= (paneY0 - Height)))
		return(1);

	UBYTE* SourcePointer = texture->buffer;
	
	if( X<paneX0 )
	{
		Width-=paneX0-X;
		SourcePointer+=paneX0-X;
		X=paneX0;
	}

	if( Y<paneY0 )
	{
		Height-=paneY0-Y;
		SourcePointer+=(paneY0-Y)*(texture->x_max+1);
		Y=paneY0;
	}
		
	if( X+Width > (paneX1+1) )
		Width= paneX1+1-X;

	if( Y+Height > (paneY1+1) )
		Height= paneY1+1-Y;

	UBYTE* DestPointer = pane->window->buffer + X + Y*DestWidth;

	SourceWidth=texture->x_max+1;
	tWidth=Width;
	tHeight=Height;

#ifdef LINUX_BUILD
#else
	_asm{

		cmp Processor,CPU_MMX
		jnz nonmmx

		push ebp
		mov edi,DestPointer
		mov esi,SourcePointer
		mov ebp,tWidth
		movq mm3,xmask

Drawlpx:
		mov edx,ebp
		sub ebp,edi
		sub ebp,edx
		and ebp,7
		sub edx,ebp
		jle mt5a

		test ebp,ebp
		jz mt0b

mt0:	mov al,[esi]
		inc esi
		cmp al,255			; Transfer enough bytes to align destination
		jz mt0a
		mov [edi],al
mt0a:	inc edi
		dec ebp
		jnz mt0

mt0b:	mov ebp,edx
		and edx,7

		shr ebp,3			; ebp = number of quad words
		mov eax,esi

		mov ebx,8
		jz mt5a

		and eax,7
		and esi,-8

		movd mm5,eax
		nop
		
		sub ebx,eax
		add eax,esi

		xor ecx,ecx
		nop

		movq mm7,[esi+8]
		psllq mm5,3

		movd mm6,ebx
		movq mm4,mm7		;Source 8 bytes

		movq mm0,[esi]
		psllq mm6,3

		movq mm2,mm3
		psrlq mm0,mm5
;
;
; MMX transparent blit inner loop - 1 cycle per byte. (Source+Dest 8 byte aligned, transparency not altered!)
;
;
mt1:	movq mm1,[edi+ecx]		;Desitination background
		psllq mm4,mm6

		por mm0,mm4
		lea ecx,[ecx+8]

		pcmpeqb mm2,mm0			;mm2 = 0xff where transparent pixels in source
		dec ebp

		pand mm1,mm2			;Only visible background left in mm1
		pandn mm2,mm0			;Only visible image left in source

		movq mm4,[esi+ecx+8]
		por mm1,mm2				;Combine visible background and source

		movq mm0,mm7			;Source 8 bytes
		movq mm7,mm4

		movq [edi+ecx-8],mm1
		movq mm2,mm3

		psrlq mm0,mm5
		jnz mt1


		add edi,ecx
		lea esi,[eax+ecx]



mt5a:	add ebp,edx
		jz mt7

mt6:	mov al,[esi]
		inc esi
		cmp al,255			; Finish off any last bytes
		jz mt8
		mov [edi],al
mt8:	inc edi
		dec ebp
		jnz mt6

mt7:	mov ebp,tWidth
		mov eax,tHeight

		sub edi,ebp
		sub esi,ebp

		mov ebx,DestWidth
		mov edx,SourceWidth
		
		add edi,ebx
		dec eax

		lea esi,[esi+edx]
		mov tHeight,eax

		jnz Drawlpx
		pop ebp

		emms
		jmp done

//
//
// Non-mmx version of the transparent blit. - color 255 is transparent
//
//
nonmmx:
		push ebp
		mov edi,DestPointer
		mov esi,SourcePointer
		mov ebp,tWidth

Drawlp:
		mov edx,ebp
		sub ebp,edi
		sub ebp,edx
		and ebp,3
		sub edx,ebp
		jle dt5b

		test ebp,ebp
		jz dt0b

dt0:	mov al,[esi]
		inc esi
		cmp al,255			; Transfer enough bytes to align destination
		jz dt0a
		mov [edi],al
dt0a:	inc edi
		dec ebp
		jnz dt0

dt0b:	mov ebp,edx
		and edx,3
		shr ebp,2
		jz dt5b

dt1:	mov ecx,[esi]
		lea esi,[esi+4]
		cmp ecx,-1			; Quick check when all transparent
		jz dt5a
		mov eax,[edi]
		cmp cl,255
		jz dt2
		mov al,cl
dt2:	cmp ch,255
		jz dt3
		mov ah,ch
dt3:	rol eax,16			; Transfer data in DWORD chunks
		shr ecx,16
		cmp cl,255
		jz dt4
		mov al,cl
dt4:	cmp ch,255
		jz dt5
		mov ah,ch
dt5:	ror eax,16
		mov [edi],eax
dt5a:	lea edi,[edi+4]
		dec ebp
		jnz dt1

dt5b:	add ebp,edx
		jz dt7

dt6:	mov al,[esi]
		inc esi
		cmp al,255			; Finish off any last bytes
		jz dt8
		mov [edi],al
dt8:	inc edi
		dec ebp
		jnz dt6

dt7:	mov ebp,tWidth
		mov eax,tHeight

		sub edi,ebp
		sub esi,ebp

		mov ebx,DestWidth
		mov edx,SourceWidth
		
		add edi,ebx
		dec eax

		lea esi,[esi+edx]
		mov tHeight,eax

		jnz Drawlp
		pop ebp
done:
	}
#endif // LINUX_BUILD
	return 0;
}



