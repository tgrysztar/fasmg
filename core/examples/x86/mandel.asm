
; Mandelbrot Set - fasm example program

include '80186.inc'
include '8087.inc'

	org	100h

	mov	ax,13h			; MCGA/VGA
	int	10h
	push	0A000h
	pop	es

	mov	dx,3C8h
	xor	al,al
	out	dx,al
	inc	dl
	mov	cx,64
    vga_palette:
	out	dx,al
	out	dx,al
	out	dx,al
	inc	al
	loop	vga_palette

	xor	di,di
	xor	dx,dx
	finit
	fld	[y_top]
	fstp	[y]
screen:
	xor	bx,bx
	fld	[x_left]
	fstp	[x]
   row:
	finit
	fldz
	fldz
	mov	cx,63
    iteration:
	fld	st0
	fmul	st0,st0
	fxch	st1
	fmul	st0,st2
	fadd	st0,st0
	fxch	st2
	fmul	st0,st0
	fsubp	st1,st0
	fxch	st1
	fadd	[y]
	fxch	st1
	fadd	[x]
	fld	st1
	fmul	st0,st0
	fld	st1
	fmul	st0,st0
	faddp	st1,st0
	fsqrt
	fistp	[i]
	cmp	[i],2
	ja	over
	loop	iteration
    over:
	mov	al,cl
	stosb
	fld	[x]
	fadd	[x_step]
	fstp	[x]
	inc	bx
	cmp	bx,320
	jb	row
	fld	[y]
	fsub	[y_step]
	fstp	[y]
	inc	dx
	cmp	dx,200
	jb	screen

	xor	ah,ah
	int	16h
	mov	ax,3
	int	10h
	int	20h

x_left dd -2.2
y_top dd 1.25

x_step dd 0.009375
y_step dd 0.0125

x dd ?
y dd ?

i dw ?
