#include"vfx.h"
extern char AlphaTable[];
extern char SpecialColor[];


//extern void AG_ellipse_draw(PANE *pane, LONG xc, LONG yc, LONG width, LONG height, LONG color);
//extern void AG_ellipse_fill(PANE *pane, LONG xc, LONG yc, LONG width, LONG height, LONG color);

#ifndef LINUX_BUILD
static signed int paneY0,paneY1,paneX0,paneX1,DestWidth,DestBuffer,x_top,y_top,Bsquared,TwoBsquared,Asquared,TwoAsquared,var_dx,var_dy,x_vector,line_left,line_right;
static int DrawRoutine;
#endif


void AG_ellipse_draw(PANE *pane, LONG xc, LONG yc, LONG width, LONG height, LONG color)
{
	if( width==0 || height==0 )
	{
		size_t ll = color;
		VFX_line_draw( pane, xc-width, yc-height, xc+width, yc+height, 0, reinterpret_cast<void*>(ll) );
		return;
	}


#ifdef LINUX_BUILD
#else
	long paneX0 = (pane->x0 < 0) ? 0 : pane->x0;
	long paneY0 = (pane->y0 < 0) ? 0 : pane->y0;
	long paneX1 = (pane->x1 >= DestWidth) ? pane->window->x_max : pane->x1;
	long paneY1 = (pane->y1 >= (pane->window->y_max+1)) ? pane->window->y_max : pane->y1;

	UBYTE* DestBuffer = pane->window->buffer;
	DestWidth = pane->window->x_max+1;

	xc+=paneX0;
	yc+=paneY0;

	_asm{

		mov eax,color
		mov ebx,offset ALPHA_ELLIPSE_PIXELS
		cmp byte ptr SpecialColor[eax],1
		jz ow1
		mov ebx,offset ELLIPSE_PIXELS
ow1:	mov DrawRoutine,ebx
		
		mov x_top,0
		mov eax,height
		mov y_top,eax
		mul eax							;compute B squares
		mov Bsquared,eax
		shl eax,1
		mov TwoBsquared,eax

		mov eax,width					;compute A squares
		mul eax
		mov Asquared,eax
		shl eax,1
		mov TwoAsquared,eax

		mov var_dx,0

		mov eax,TwoAsquared				;dy = TwoAsquared * b
		mul height
		mov var_dy,eax

		mov eax,Asquared                
		shr eax,2                       
		add eax,Bsquared				;eax = Asquared/4 + Bsquared
		mov x_vector,eax				;x_vector= a^2/4 + b^2
		mov eax,Asquared
		mul height
		sub x_vector,eax				;x_vector=a^2/4+b^2-a^2*b 

		mov ebx,height

__until_pos:
		mov eax,var_dx                  
		sub eax,var_dy
		jns __dx_ge_dy					;jmp if dx >= dy

//__plot_neg:
		call DrawRoutine

		cmp x_vector,0                  
		js __d_neg						;jmp if d < 0

		dec y_top
		dec ebx

		mov eax,var_dy
		sub eax,TwoAsquared
		mov var_dy,eax					;dy -= 2*a^2

		sub x_vector,eax				;x_vector -= dy

__d_neg:
        inc x_top               

		mov eax,var_dx                  
		add eax,TwoBsquared
		mov var_dx,eax					;dx += 2*b^2

		add eax,Bsquared                
		add x_vector,eax				;x_vector += dx + b^2

		jmp __until_pos

__dx_ge_dy:     
		mov eax,Asquared
		sub eax,Bsquared				;eax=a^2-b^2
		mov edx,eax						;edx=a^2-b^2
		sar eax,1						;eax=(a^2-b^2)/2
		add eax,edx						;eax=3*(a^2-b^2)/2 
		sub eax,var_dx					;eax=3*(a^2-b^2)/2 - dx 
		sub eax,var_dy					;eax=3*(a^2-b^2)/2 - (dx+dy)
		sar eax,1						; /2

		add x_vector,eax

__until_neg:
		call DrawRoutine

		cmp x_vector,0
		jns __d_pos

		inc x_top

		mov eax,var_dx
		add eax,TwoBsquared
		mov var_dx,eax					;dx += 2*b^2
		add x_vector,eax				;x_vector += dx

__d_pos:
        dec y_top

		mov eax,var_dy                  
		sub eax,TwoAsquared
		mov var_dy,eax					;dy -= 2*a^2
		sub eax,Asquared				;eax = dy - a^2
		sub x_vector,eax				;x_vector += (-dy + a^2)

		dec ebx

		js __end_ellipse
		jmp __until_neg


;
;
;
ELLIPSE_PIXELS:
		push ebx
		mov cl,byte ptr color

		mov edi,xc
		add edi,x_top
		mov ebx,yc
		add ebx,y_top
		cmp edi,paneX0
		jl P1
		cmp edi,paneX1
		jg P1
		cmp ebx,paneY0
		jl P1
		cmp ebx,paneY1
		jg P1
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov BYTE PTR [edi+ebx],cl
P1:
		mov edi,xc
		add edi,x_top
		mov ebx,yc
		sub ebx,y_top
		cmp edi,paneX0
		jl P2
		cmp edi,paneX1
		jg P2
		cmp ebx,paneY0
		jl P2
		cmp ebx,paneY1
		jg P2
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov BYTE PTR [edi+ebx],cl
P2:
		mov edi,xc
		sub edi,x_top
		mov ebx,yc
		add ebx,y_top
		cmp edi,paneX0
		jl P3
		cmp edi,paneX1
		jg P3
		cmp ebx,paneY0
		jl P3
		cmp ebx,paneY1
		jg P3
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov BYTE PTR [edi+ebx],cl
P3:
		mov edi,xc
		sub edi,x_top
		mov ebx,yc
		sub ebx,y_top
		cmp edi,paneX0
		jl P4
		cmp edi,paneX1
		jg P4
		cmp ebx,paneY0
		jl P4
		cmp ebx,paneY1
		jg P4
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov BYTE PTR [edi+ebx],cl
P4:
		pop ebx
		ret
;
;
; Same as above, but allow alpha pixels
;
;
ALPHA_ELLIPSE_PIXELS:
		push ebx
		xor ecx,ecx
		mov ch,byte ptr color

		mov edi,xc
		add edi,x_top
		mov ebx,yc
		add ebx,y_top
		cmp edi,paneX0
		jl TP1
		cmp edi,paneX1
		jg TP1
		cmp ebx,paneY0
		jl TP1
		cmp ebx,paneY1
		jg TP1
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov cl,[edi+ebx]
		mov cl,AlphaTable[ecx]
		mov BYTE PTR [edi+ebx],cl
TP1:
		mov edi,xc
		add edi,x_top
		mov ebx,yc
		sub ebx,y_top
		cmp edi,paneX0
		jl TP2
		cmp edi,paneX1
		jg TP2
		cmp ebx,paneY0
		jl TP2
		cmp ebx,paneY1
		jg TP2
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov cl,[edi+ebx]
		mov cl,AlphaTable[ecx]
		mov BYTE PTR [edi+ebx],cl
TP2:
		mov edi,xc
		sub edi,x_top
		mov ebx,yc
		add ebx,y_top
		cmp edi,paneX0
		jl TP3
		cmp edi,paneX1
		jg TP3
		cmp ebx,paneY0
		jl TP3
		cmp ebx,paneY1
		jg TP3
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov cl,[edi+ebx]
		mov cl,AlphaTable[ecx]
		mov BYTE PTR [edi+ebx],cl
TP3:
		mov edi,xc
		sub edi,x_top
		mov ebx,yc
		sub ebx,y_top
		cmp edi,paneX0
		jl TP4
		cmp edi,paneX1
		jg TP4
		cmp ebx,paneY0
		jl TP4
		cmp ebx,paneY1
		jg TP4
		imul ebx,DestWidth
		add ebx,DestBuffer
		mov cl,[edi+ebx]
		mov cl,AlphaTable[ecx]
		mov BYTE PTR [edi+ebx],cl
TP4:
		pop ebx
		ret

__end_ellipse:  
	}
#endif // LINUX_BUILD
}





void AG_ellipse_fill(PANE *pane, LONG xc, LONG yc, LONG width, LONG height, LONG color)
{
	if( width==0 || height==0 )
	{
		size_t ll = color;
		VFX_line_draw( pane, xc-width, yc-height, xc+width, yc+height, 0, reinterpret_cast<void*>(ll) );
		return;
	}

#ifdef LINUX_BUILD
#else

	DestWidth = pane->window->x_max+1;

	long paneX0 = (pane->x0 < 0) ? 0 : pane->x0;
	long paneY0 = (pane->y0 < 0) ? 0 : pane->y0;
	long paneX1 = (pane->x1 >= DestWidth) ? pane->window->x_max : pane->x1;
	long paneY1 = (pane->y1 >= (pane->window->y_max+1)) ? pane->window->y_max : pane->y1;

	UBYTE* DestBuffer = pane->window->buffer;

	xc+=paneX0;
	yc+=paneY0;
	_asm{

		mov eax,color
		mov ebx,offset ALPHA_ELLIPSE_LINES
		cmp byte ptr SpecialColor[eax],1
		jz ow2
		mov ebx,offset ELLIPSE_LINES
ow2:	mov DrawRoutine,ebx

		mov x_top,0
		mov eax,height
		mov y_top,eax
		mul eax                         ;compute B squares
		mov Bsquared,eax
		shl eax,1
		mov TwoBsquared,eax

		mov eax,width                 ;compute A squares
		mul eax
		mov Asquared,eax
		shl eax,1
		mov TwoAsquared,eax

		mov var_dx,0

		mov eax,TwoAsquared             ;dy = TwoAsquared * b
		mul height
		mov var_dy,eax

		mov eax,Asquared                
		shr eax,2                       
		add eax,Bsquared                ;eax = Asquared/4 + Bsquared
		mov x_vector,eax                ;x_vector= a^2/4 + b^2
		mov eax,Asquared
		mul height
		sub x_vector,eax                ;x_vector=a^2/4+b^2-a^2*b 

		mov ebx,height

__until_pos:
		mov eax,var_dx
		sub eax,var_dy
		js __plot_neg
		jmp __dx_ge_dy

__plot_neg:
		call DrawRoutine

		cmp x_vector,0
		js __d_neg
		dec y_top
		dec ebx
		mov eax,var_dy
		sub eax,TwoAsquared
		mov var_dy,eax
		sub x_vector,eax

__d_neg:
		inc x_top
		mov eax,var_dx
		add eax,TwoBsquared
		mov var_dx,eax
		add eax,Bsquared
		add x_vector,eax
		jmp __until_pos

__dx_ge_dy:
		mov eax,Asquared
		sub eax,Bsquared                ;eax=a^2-b^2
		mov edx,eax                     ;edx=a^2-b^2
		sar eax,1                       ;eax=(a^2-b^2)/2
		add eax,edx                     ;eax=3*(a^2-b^2)/2 
		sub eax,var_dx                  ;eax=3*(a^2-b^2)/2 - dx 
		sub eax,var_dy                  ;eax=3*(a^2-b^2)/2 - (dx+dy)
		sar eax,1                       ; /2
		add x_vector,eax

__until_neg:
		call DrawRoutine

		cmp x_vector,0
		jns __d_pos
		inc x_top
		mov eax,var_dx
		add eax,TwoBsquared
		mov var_dx,eax
		add x_vector,eax

__d_pos:
		dec y_top
		mov eax,var_dy
		sub eax,TwoAsquared
		mov var_dy,eax
		sub eax,Asquared
		sub x_vector,eax

		dec ebx
		js __end_ellipse
		jmp __until_neg



ELLIPSE_LINES:
		mov edi,xc
		add edi,x_top
		cmp edi,paneX0
		jl LN2                  ;;right end to left of window

		cmp edi,paneX1
		jl TR1

		mov edi,paneX1

TR1:
		mov line_right,edi
		mov edi,xc
		sub edi,x_top
		cmp edi,paneX1
		jg LN2                  ;;left end to right of window

		cmp edi,paneX0
		jg TR2

		mov edi,paneX0

TR2:
		mov line_left,edi
		mov edx,yc
		add edx,y_top
		cmp edx,paneY0     
		jl LN2                  ;;bottom line above window

		cmp edx,paneY1
		jg LN1

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

		mov al,byte ptr color
lo1:	mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz lo1

LN1:
		mov edi,line_left
		mov edx,yc
		sub edx,y_top
		cmp edx,paneY0     
		jl LN2

		cmp edx,paneY1
		jg LN2                  ;;top line below window

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

		mov al,byte ptr color
lo2:	mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz lo2

LN2:    
		ret

;
;
; Same as above, but with Alpha pixels
;
;
ALPHA_ELLIPSE_LINES:
		mov edi,xc
		add edi,x_top
		cmp edi,paneX0
		jl ALN2                  ;;right end to left of window

		cmp edi,paneX1
		jl ATR1

		mov edi,paneX1

ATR1:
		mov line_right,edi
		mov edi,xc
		sub edi,x_top
		cmp edi,paneX1
		jg ALN2                  ;;left end to right of window

		cmp edi,paneX0
		jg ATR2

		mov edi,paneX0

ATR2:
		mov line_left,edi
		mov edx,yc
		add edx,y_top
		cmp edx,paneY0     
		jl ALN2                  ;;bottom line above window

		cmp edx,paneY1
		jg ALN1

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

		xor eax,eax
		mov ah,byte ptr color
Alo1:	mov al,[edi]
		mov al,AlphaTable[eax]
		mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz Alo1

ALN1:
		mov edi,line_left
		mov edx,yc
		sub edx,y_top
		cmp edx,paneY0     
		jl ALN2

		cmp edx,paneY1
		jg ALN2                  ;;top line below window

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

		xor eax,eax
		mov ah,byte ptr color
Alo2:	mov al,[edi]
		mov al,AlphaTable[eax]
		mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz Alo2

ALN2:    
		ret




__end_ellipse:
	}
#endif // LINUX_BUILD
}

PANE *xorgPane = 0;
unsigned char gColor = 0;

void orLineCallback (int x, int y)
{
	long result = VFX_pixel_read(xorgPane,x,y);
	result |= gColor;
	VFX_pixel_write(xorgPane,x,y,result);
}

void AG_ellipse_fillOr(PANE *pane, LONG xc, LONG yc, LONG width, LONG height, LONG color)
{
	if( width==0 || height==0 )
	{
		xorgPane = pane;
		gColor = (unsigned char)color;
		VFX_line_draw( pane, xc-width, yc-height, xc+width, yc+height,LD_EXECUTE, &orLineCallback);
		return;
	}


#ifdef LINUX_BUILD
#else

	DestWidth = pane->window->x_max+1;

	long paneX0 = (pane->x0 < 0) ? 0 : pane->x0;
	long paneY0 = (pane->y0 < 0) ? 0 : pane->y0;
	long paneX1 = (pane->x1 >= DestWidth) ? pane->window->x_max : pane->x1;
	long paneY1 = (pane->y1 >= (pane->window->y_max+1)) ? pane->window->y_max : pane->y1;

	UBYTE* DestBuffer = pane->window->buffer;

	xc+=paneX0;
	yc+=paneY0;
	_asm{

		mov eax,color
		mov ebx,offset XOR_ELLIPSE_LINES
		mov DrawRoutine,ebx

		mov x_top,0
		mov eax,height
		mov y_top,eax
		mul eax                         ;compute B squares
		mov Bsquared,eax
		shl eax,1
		mov TwoBsquared,eax

		mov eax,width                 ;compute A squares
		mul eax
		mov Asquared,eax
		shl eax,1
		mov TwoAsquared,eax

		mov var_dx,0

		mov eax,TwoAsquared             ;dy = TwoAsquared * b
		mul height
		mov var_dy,eax

		mov eax,Asquared                
		shr eax,2                       
		add eax,Bsquared                ;eax = Asquared/4 + Bsquared
		mov x_vector,eax                ;x_vector= a^2/4 + b^2
		mov eax,Asquared
		mul height
		sub x_vector,eax                ;x_vector=a^2/4+b^2-a^2*b 

		mov ebx,height

__until_pos:
		mov eax,var_dx
		sub eax,var_dy
		js __plot_neg
		jmp __dx_ge_dy

__plot_neg:
		call DrawRoutine

		cmp x_vector,0
		js __d_neg
		dec y_top
		dec ebx
		mov eax,var_dy
		sub eax,TwoAsquared
		mov var_dy,eax
		sub x_vector,eax

__d_neg:
		inc x_top
		mov eax,var_dx
		add eax,TwoBsquared
		mov var_dx,eax
		add eax,Bsquared
		add x_vector,eax
		jmp __until_pos

__dx_ge_dy:
		mov eax,Asquared
		sub eax,Bsquared                ;eax=a^2-b^2
		mov edx,eax                     ;edx=a^2-b^2
		sar eax,1                       ;eax=(a^2-b^2)/2
		add eax,edx                     ;eax=3*(a^2-b^2)/2 
		sub eax,var_dx                  ;eax=3*(a^2-b^2)/2 - dx 
		sub eax,var_dy                  ;eax=3*(a^2-b^2)/2 - (dx+dy)
		sar eax,1                       ; /2
		add x_vector,eax

__until_neg:
		call DrawRoutine

		cmp x_vector,0
		jns __d_pos
		inc x_top
		mov eax,var_dx
		add eax,TwoBsquared
		mov var_dx,eax
		add x_vector,eax

__d_pos:
		dec y_top
		mov eax,var_dy
		sub eax,TwoAsquared
		mov var_dy,eax
		sub eax,Asquared
		sub x_vector,eax

		dec ebx
		js __end_ellipse
		jmp __until_neg



XOR_ELLIPSE_LINES:
		mov edi,xc
		add edi,x_top
		cmp edi,paneX0
		jl LN2                  ;;right end to left of window

		cmp edi,paneX1
		jl TR1

		mov edi,paneX1

TR1:
		mov line_right,edi
		mov edi,xc
		sub edi,x_top
		cmp edi,paneX1
		jg LN2                  ;;left end to right of window

		cmp edi,paneX0
		jg TR2

		mov edi,paneX0

TR2:
		mov line_left,edi
		mov edx,yc
		add edx,y_top
		cmp edx,paneY0     
		jl LN2                  ;;bottom line above window

		cmp edx,paneY1
		jg LN1

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

lo1:	mov al, byte ptr [edi]
		or al, byte ptr color
		mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz lo1

LN1:
		mov edi,line_left
		mov edx,yc
		sub edx,y_top
		cmp edx,paneY0     
		jl LN2

		cmp edx,paneY1
		jg LN2                  ;;top line below window

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

lo2:	mov al, byte ptr [edi]
		or al, byte ptr color
		mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz lo2

LN2:    
		ret

__end_ellipse:
	}
#endif // LINUX_BUILD
}

void andLineCallback (int x, int y)
{
	long result = VFX_pixel_read(xorgPane,x,y);
	result &= gColor;
	VFX_pixel_write(xorgPane,x,y,result);
}	

void AG_ellipse_fillXor(PANE *pane, LONG xc, LONG yc, LONG width, LONG height, LONG color)
{
	
	if( width==0 || height==0 )
	{
		xorgPane = pane;
		gColor = (unsigned char)color;
		VFX_line_draw( pane, xc-width, yc-height, xc+width, yc+height, LD_EXECUTE, &andLineCallback);
		return;
	}


#ifdef LINUX_BUILD
    // sebi !NB (checkk othes _asm in this file)
#else

	long xorResult = color ^ 0xff;

	unsigned char xorColor = (unsigned char)xorResult;

	DestWidth = pane->window->x_max+1;

	long paneX0 = (pane->x0 < 0) ? 0 : pane->x0;
	long paneY0 = (pane->y0 < 0) ? 0 : pane->y0;
	long paneX1 = (pane->x1 >= DestWidth) ? pane->window->x_max : pane->x1;
	long paneY1 = (pane->y1 >= (pane->window->y_max+1)) ? pane->window->y_max : pane->y1;

	UBYTE* DestBuffer = pane->window->buffer;

	xc+=paneX0;
	yc+=paneY0;

	_asm{

		mov eax,color
		mov ebx,offset XOR_ELLIPSE_LINES
		mov DrawRoutine,ebx

		mov x_top,0
		mov eax,height
		mov y_top,eax
		mul eax                         ;compute B squares
		mov Bsquared,eax
		shl eax,1
		mov TwoBsquared,eax

		mov eax,width                 ;compute A squares
		mul eax
		mov Asquared,eax
		shl eax,1
		mov TwoAsquared,eax

		mov var_dx,0

		mov eax,TwoAsquared             ;dy = TwoAsquared * b
		mul height
		mov var_dy,eax

		mov eax,Asquared                
		shr eax,2                       
		add eax,Bsquared                ;eax = Asquared/4 + Bsquared
		mov x_vector,eax                ;x_vector= a^2/4 + b^2
		mov eax,Asquared
		mul height
		sub x_vector,eax                ;x_vector=a^2/4+b^2-a^2*b 

		mov ebx,height

__until_pos:
		mov eax,var_dx
		sub eax,var_dy
		js __plot_neg
		jmp __dx_ge_dy

__plot_neg:
		call DrawRoutine

		cmp x_vector,0
		js __d_neg
		dec y_top
		dec ebx
		mov eax,var_dy
		sub eax,TwoAsquared
		mov var_dy,eax
		sub x_vector,eax

__d_neg:
		inc x_top
		mov eax,var_dx
		add eax,TwoBsquared
		mov var_dx,eax
		add eax,Bsquared
		add x_vector,eax
		jmp __until_pos

__dx_ge_dy:
		mov eax,Asquared
		sub eax,Bsquared                ;eax=a^2-b^2
		mov edx,eax                     ;edx=a^2-b^2
		sar eax,1                       ;eax=(a^2-b^2)/2
		add eax,edx                     ;eax=3*(a^2-b^2)/2 
		sub eax,var_dx                  ;eax=3*(a^2-b^2)/2 - dx 
		sub eax,var_dy                  ;eax=3*(a^2-b^2)/2 - (dx+dy)
		sar eax,1                       ; /2
		add x_vector,eax

__until_neg:
		call DrawRoutine

		cmp x_vector,0
		jns __d_pos
		inc x_top
		mov eax,var_dx
		add eax,TwoBsquared
		mov var_dx,eax
		add x_vector,eax

__d_pos:
		dec y_top
		mov eax,var_dy
		sub eax,TwoAsquared
		mov var_dy,eax
		sub eax,Asquared
		sub x_vector,eax

		dec ebx
		js __end_ellipse
		jmp __until_neg



XOR_ELLIPSE_LINES:
		mov edi,xc
		add edi,x_top
		cmp edi,paneX0
		jl LN2                  ;;right end to left of window

		cmp edi,paneX1
		jl TR1

		mov edi,paneX1

TR1:
		mov line_right,edi
		mov edi,xc
		sub edi,x_top
		cmp edi,paneX1
		jg LN2                  ;;left end to right of window

		cmp edi,paneX0
		jg TR2

		mov edi,paneX0

TR2:
		mov line_left,edi
		mov edx,yc
		add edx,y_top
		cmp edx,paneY0     
		jl LN2                  ;;bottom line above window

		cmp edx,paneY1
		jg LN1

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

lo1:	mov al, byte ptr [edi]
		and al, xorColor
		mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz lo1

LN1:
		mov edi,line_left
		mov edx,yc
		sub edx,y_top
		cmp edx,paneY0     
		jl LN2

		cmp edx,paneY1
		jg LN2                  ;;top line below window

		imul    edx,DestWidth
		add     edx,DestBuffer
		add edi,edx

		mov ecx,line_right
		sub ecx,line_left
		inc ecx

lo2:	mov al, byte ptr [edi]
		and al, xorColor
		mov byte ptr [edi],al
		inc edi
		dec ecx
		jnz lo2

LN2:    
		ret

__end_ellipse:
	}
#endif // LINUX_BUILD
}







