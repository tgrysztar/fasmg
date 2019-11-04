; TetrOS version 1.02
; by Tomasz Grysztar

; Requires VGA and 80386 CPU or higher.
; Version 1.01 was submitted in 2004 for a 512-byte OS contest.

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
DELAY = 4

virtual at 46Ch
  clock dw ?
end virtual

virtual at bp
  current dw ?
  current_column db ?
  current_row dw ?
  next dw ?
  score dw ?
  last_tick dw ?
  random dw ?
end virtual

label well at 9000h
label pics at well-2*64

	xor	ax,ax
	mov	sp,8000h
	mov	ss,ax
	mov	ds,ax
	mov	es,ax
	push	ax
	push	start
	retf

start:
	mov	bp,sp

	mov	al,13h
	int	10h

	mov	di,3*4
	mov	ax,int_3
	stosw
	xor	ax,ax
	stosw
	lea	di,[next]
	stosw	; [next]
	stosw	; [score]
	mov	ax,[clock]
	stosw	; [last_tick]
	stosw	; [random]

	mov	di,pics
	mov	cx,64
	mov	al,1	; remove this instruction to get randomized background
	rep	stosb
	mov	ah,15
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
	mov	ax,11b + (-1) shl (COLUMNS+2)
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
	mov	al,COLUMNS/2
	stosb	; [current_column]
	mov	ax,well + (3+ROWS-4)*2
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
	jz	start
	test	bp,bp
	jnp	process_key
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
	mov	bx,7
	mov	dx,12h
	mov	ah,2
	int	10h
	mov	cl,12
      print_score:
	mov	ax,[score]
	shr	ax,cl
	and	al,0Fh
	cmp	al,10
	sbb	al,69h
	das
	mov	ah,0Eh
	int	10h
	sub	cl,4
	jnc	print_score
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
	shr	bx,2
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
	mov	ax,[clock]
	sub	ax,[last_tick]
	cmp	al,DELAY
	jb	main_loop
	add	[last_tick],ax
	call	do_move_down
	jz	update_screen
	mov	dx,1
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
	mov	ax,clear_piece
	int3
first_move:
	push	dword [current]
	call	si
	xor	ch,ch
	mov	ax,test_piece
	int3
	mov	al,draw_piece and 0FFh
	pop	edx
	or	ch,ch
	jz	@f
	mov	dword [current],edx
      @@:
	int3
      no_move:
	ret
down:
	sub	byte [current_row],2
	ret
left:
	dec	[current_column]
	ret
right:
	inc	[current_column]
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

int_3:
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
	call	ax
	add	bh,4
	scasw
	dec	bl
	jnz	on_piece_row
	iret

clear_piece:
	not	dx
	and	[di],dx
	ret
test_piece:
	test	[di],dx
	jz	@f
	inc	ch
     @@:
	ret
draw_piece:
	or	[di],dx
	ret

pieces dw 0010001000100010b
       dw 0010011000100000b
       dw 0010001001100000b
       dw 0100010001100000b
       dw 0000011001100000b
       dw 0100011000100000b
       dw 0010011001000000b

rb 7C00h+510-$
dw 0AA55h