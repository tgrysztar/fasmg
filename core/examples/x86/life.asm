
; Life - fasm example program

; Controls:
;   arrow keys - move cursor
;   Space - switch cell
;   Enter - move to next generation
;   Esc - exit program

	include '80186.inc'

	org	100h
	jumps

	mov	di,screen_data
	xor	al,al
	mov	cx,80*50*2
	rep	stosb

	mov	ax,3
	int	10h			; set text mode
	mov	ah,1
	mov	ch,20h
	int	10h			; hide cursor
	mov	ax,1003h
	xor	bx,bx
	int	10h			; enable background intensity

	mov	ax,1100h
	mov	bp,DCh_pattern
	mov	cx,1
	mov	dx,0DCh
	mov	bx,1000h
	int	10h

	mov	ax,0B800h
	mov	es,ax
	xor	di,di
	mov	ax,0DCh
	mov	cx,80*25
	rep	stosw

    redraw_screen:
	mov	si,[cursor_y]
	imul	si,80
	add	si,[cursor_x]
	and	byte [screen_data+si],8
	or	byte [screen_data+si],2

	mov	si,screen_data
	xor	di,di
	mov	cx,50
      draw_screen:
	push	cx
	mov	cx,80
      draw_line:
	mov	ah,[si+80]
	lodsb
	shl	al,4
	and	ah,0Fh
	or	al,ah
	inc	di
	stosb
	loop	draw_line
	pop	cx
	add	si,80
	loop	draw_screen

    wait_for_key:
	xor	ah,ah
	int	16h
	cmp	ah,1
	je	exit
	cmp	ah,1Ch
	je	next_generation
	cmp	ah,39h
	je	switch_cell
	cmp	ah,4Bh
	je	cursor_left
	cmp	ah,4Dh
	je	cursor_right
	cmp	ah,48h
	je	cursor_up
	cmp	ah,50h
	je	cursor_down
	jmp	wait_for_key

    switch_cell:
	mov	si,[cursor_y]
	imul	si,80
	add	si,[cursor_x]
	xor	byte [screen_data+si],8
	jmp	redraw_screen

    cursor_left:
	cmp	[cursor_x],1
	jbe	wait_for_key
	call	clear_cursor
	dec	[cursor_x]
	jmp	redraw_screen
    cursor_right:
	cmp	[cursor_x],78
	jae	wait_for_key
	call	clear_cursor
	inc	[cursor_x]
	jmp	redraw_screen
    cursor_up:
	cmp	[cursor_y],1
	jbe	wait_for_key
	call	clear_cursor
	dec	[cursor_y]
	jmp	redraw_screen
    cursor_down:
	cmp	[cursor_y],48
	jae	wait_for_key
	call	clear_cursor
	inc	[cursor_y]
	jmp	redraw_screen

    next_generation:
	call	clear_cursor
	mov	si,screen_data+81
	mov	di,screen_data+80*50+81
	mov	cx,48
      process_screen:
	push	cx
	mov	cx,78
      process_line:
	xor	bl,bl
	mov	al,[si+1]
	and	al,1
	add	bl,al
	mov	al,[si-1]
	and	al,1
	add	bl,al
	mov	al,[si+80]
	and	al,1
	add	bl,al
	mov	al,[si-80]
	and	al,1
	add	bl,al
	mov	al,[si+80+1]
	and	al,1
	add	bl,al
	mov	al,[si+80-1]
	and	al,1
	add	bl,al
	mov	al,[si-80+1]
	and	al,1
	add	bl,al
	mov	al,[si-80-1]
	and	al,1
	add	bl,al
	mov	al,byte [si]
	mov	byte [di],al
	cmp	bl,1
	jbe	clear_cell
	cmp	bl,4
	jae	clear_cell
	cmp	bl,2
	je	cell_ok
	mov	byte [di],0Fh
	jmp	cell_ok
      clear_cell:
	mov	byte [di],0
      cell_ok:
	inc	si
	inc	di
	loop	process_line
	pop	cx
	add	si,2
	add	di,2
	loop	process_screen
	push	es
	push	ds
	pop	es
	mov	si,screen_data+80*50
	mov	di,screen_data
	mov	cx,80*50
	rep	movsb
	pop	es
	jmp	redraw_screen

    exit:
	mov	ax,3
	int	10h
	int	20h

    clear_cursor:
	mov	si,[cursor_y]
	imul	si,80
	add	si,[cursor_x]
	mov	al,byte [screen_data+si]
	cmp	al,2
	je	empty_cell
	mov	byte [screen_data+si],0Fh
	ret
      empty_cell:
	mov	byte [screen_data+si],0
	ret

cursor_x dw 40
cursor_y dw 25

DCh_pattern:
 db 8 dup 0
 db 8 dup 0FFh

screen_data rb 80*50*2
