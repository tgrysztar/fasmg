
; This is an example of setting up 32-bit program with flat memory model
; using DPMI. It requires 32-bit DPMI host in order to start.

include '80386.inc'
include 'format/mz.inc'

heap 0					; no additional memory

segment loader use16

	push	cs
	pop	ds

	mov	ax,1687h
	int	2Fh
	or	ax,ax			; DPMI installed?
	jnz	error
	test	bl,1			; 32-bit programs supported?
	jz	error
	mov	word [mode_switch],di
	mov	word [mode_switch+2],es
	mov	bx,si			; allocate memory for DPMI data
	mov	ah,48h
	int	21h
	jc	error
	mov	es,ax
	mov	ax,1
	call	far [mode_switch]	; switch to protected mode
	jc	error

	mov	cx,1
	xor	ax,ax
	int	31h			; allocate descriptor for code
	mov	si,ax
	xor	ax,ax
	int	31h			; allocate descriptor for data
	mov	di,ax
	mov	dx,cs
	lar	cx,dx
	shr	cx,8
	or	cx,0C000h
	mov	bx,si
	mov	ax,9
	int	31h			; set code descriptor access rights
	mov	dx,ds
	lar	cx,dx
	shr	cx,8
	or	cx,0C000h
	mov	bx,di
	int	31h			; set data descriptor access rights
	mov	ecx,main
	shl	ecx,4
	mov	dx,cx
	shr	ecx,16
	mov	ax,7			; set descriptor base address
	int	31h
	mov	bx,si
	int	31h
	mov	cx,0FFFFh
	mov	dx,0FFFFh
	mov	ax,8			; set segment limit to 4 GB
	int	31h
	mov	bx,di
	int	31h

	mov	ds,di
	mov	es,di
	mov	fs,di
	mov	gs,di
	push	0
	push	si
	push	dword start
	retfd

    error:
	mov	ax,4CFFh
	int	21h

  mode_switch dd ?

segment main use32

  start:
	mov	esi,hello
    .loop:
	lodsb
	or	al,al
	jz	.done
	mov	dl,al
	mov	ah,2
	int	21h
	jmp	.loop
    .done:

	mov	ax,4C00h
	int	21h

  hello db 'Hello from protected mode!',0Dh,0Ah,0
