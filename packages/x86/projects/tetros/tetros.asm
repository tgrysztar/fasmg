; TetrOS version 1.05
; by Tomasz Grysztar

; Requires VGA and 80386 CPU or higher.
; Version 1.01 was submitted in 2004 for a 512-byte OS contest
;   (https://web.archive.org/web/20040930044834/http://512.decard.net/)

; For your playing pleasure, it's a boot-sector Tetris game.
; Keys:
;   Left - move left
;   Right - move right
;   Up - rotate
;   Down - drop
;   Esc - new game at any time

include 'cpu/80386.inc'
include 'macro/@@.inc'

format binary as 'img'
org 7C00h

ROWS = 23
COLUMNS = 12
BACKGROUND = 0 ; background color, 0 for randomized

virtual at 46Ch
  clock dw ?
end virtual

virtual at bp
  current dw ?
  current_column db ?
  current_row dw ?
  filler db ?
  next dw ?
  score dw ?
  last_tick dw ?
  random dw ?
  pics_wrt_bp:
end virtual

label well at 8000h
label pics at well-2*64
label origin at pics-(pics_wrt_bp-bp)

assert origin and 1 = 0
ORIGIN_PARITY = origin and 0FFh
while ORIGIN_PARITY and not 1
	ORIGIN_PARITY = (ORIGIN_PARITY shr 1) xor (ORIGIN_PARITY and 1)
end while

	xor	ax,ax
	mov	ss,ax
	mov	sp,origin
	mov	ds,ax
	mov	es,ax
	push	ax
	push	start
	retf

start:
	mov	al,13h
	int	10h

restart:
	mov	bp,sp

	xor	ax,ax
	cld
	lea	di,[next]
	stosw	; [next]
	stosw	; [score]
	mov	ax,[clock]
	stosw	; [last_tick]
	stosw	; [random]

	mov	cx,64
if BACKGROUND
	mov	ax,0F00h + BACKGROUND
else
	mov	ah,15
end if
	rep	stosb
	mov	dx,7
      @@:
	mov	al,15
	stosb
	mov	al,ah
	mov	cl,6
	rep	stosb
	mov	ax,0708h
	stosb
	dec	dx
	jnz	@b
	mov	cl,8
	rep	stosb

	or	ax,-1
	stosw
	stosw
	stosw
	mov	cl,ROWS+4
	mov	ax,not ( (1 shl COLUMNS - 1) shl ((16-COLUMNS)/2) )
	rep	stosw

new_piece:
	mov	bx,[random]
	mov	ax,257
	mul	bx
	inc	ax
	mov	cx,43243
	div	cx
	mov	[random],dx
	and	bx,7
	jz	new_piece
	shl	bx,1
	mov	ax,[pieces+bx-2]
	xchg	ax,[next]
	or	ax,ax
	jz	new_piece
	lea	di,[current]
	stosw	; [current]
	mov	al,6
	stosb	; [current_column]
	mov	ax,well+(3+ROWS-4)*2
	stosw	; [current_row]
	mov	si,no_move
	call	first_move
	jz	update_screen
	inc	bp	; game over

process_key:
	xor	ah,ah
	int	16h
	mov	al,ah
	dec	ah
	jz	restart
	test	bp,bp
if ORIGIN_PARITY
	jp	process_key
else
	jnp	process_key
end if
	mov	si,rotate
	cmp	al,48h
	je	action
	mov	si,left
	cmp	al,4Bh
	je	action
	mov	si,right
	cmp	al,4Dh
	je	action
	cmp	al,50h
	jne	main_loop

drop_down:
	call	do_move_down
	jz	drop_down

action:
	call	do_move

update_screen:
	mov	bx,15
	mov	dx,12h
	mov	ah,2
	int	10h
	mov	cl,84h
      print_score:
	mov	ax,[score]
	rol	ax,cl
	and	al,0Fh
	cmp	al,10
	sbb	al,69h
	das
	mov	ah,0Eh
	int	10h
	add	cl,84h
	jns	print_score
	xor	bl,15 xor 7
	jnp	print_score
	push	es
	push	0A000h
	pop	es
	mov	si,well+3*2
	mov	di,320*184+160-(COLUMNS*8)/2
      draw_well:
	lodsw
	push	si
	mov	dl,COLUMNS
	xchg	bx,ax
	shr	bx,(16-COLUMNS)/2
	call	draw_row
	pop	si
	cmp	si,well+(3+ROWS)*2
	jb	draw_well
	mov	di,320*100+250
	mov	bx,[next]
      draw_preview:
	mov	dl,4
	call	draw_row
	cmp	di,320*68
	ja	draw_preview
	pop	es

main_loop:
	mov	ah,1
	int	16h
	jnz	process_key
	mov	al,-1
	xor	al,byte [score+1]
	and	al,11b
	mov	cx,[clock]
	sub	cx,[last_tick]
	cmp	cl,al
	jbe	main_loop
	add	[last_tick],cx
	call	do_move_down
	jz	update_screen
	movzx	dx,byte [current_row]
	shr	dx,2
	mov	si,well+3*2
	mov	di,si
      check_row:
	lodsw
	inc	ax
	jz	remove_row
	dec	ax
	stosw
	jmp	check_next_row
      remove_row:
	shl	dx,1
      check_next_row:
	cmp	di,well+(3+ROWS)*2
	jb	check_row
	add	[score],dx
	jmp	new_piece

draw_row:
	push	di
      blit_row:
	shr	bx,1
	mov	si,pics
	jnc	blit_block
	add	si,64
      blit_block:
	mov	cx,8
	rep	movsb
	add	di,320-8
	test	si,111111b
	jnz	blit_block
	sub	di,320*8-8
	dec	dx
	jnz	blit_row
	pop	di
	sub	di,320*8
	ret

do_move_down:
	mov	si,down
do_move:
	mov	al,1
	call	on_piece
first_move:
	push	dword [current]
	call	si
	xor	ax,ax
	call	on_piece
	inc	ax
	pop	edx
	test	ah,ah
	jz	@f
	mov	dword [current],edx
      @@:
	jmp	on_piece
down:
	sub	byte [current_row],2
	ret
left:
	dec	[current_column]
	ret
right:
	inc	[current_column]
no_move:
	ret
rotate:
	mov	cx,3
     @@:
	bt	[current],cx
	rcl	dx,1
	add	cl,4
	cmp	cl,16
	jb	@b
	sub	cl,17
	jnc	@b
	mov	[current],dx
	ret

on_piece:
	pushf
	mov	di,[current_row]
	mov	bx,4
      on_piece_row:
	mov	dx,[current]
	mov	cl,bh
	shr	dx,cl
	and	dx,1111b
	mov	cl,[current_column]
	add	cl,4
	shl	edx,cl
	shr	edx,4
	test	al,al
	jz	@f
	xor	[di],dx
      @@:
	test	[di],dx
	jz	@f
	inc	ah
      @@:
	add	bh,4
	scasw
	dec	bl
	jnz	on_piece_row
	popf
	ret

rb 510 - ($% + signature - pieces)

pieces dw 0010001000100010b
       dw 0010011000100000b
       dw 0010001001100000b
       dw 0100010001100000b
       dw 0000011001100000b
       dw 0100011000100000b
       dw 0010011001000000b

signature dw 0AA55h
