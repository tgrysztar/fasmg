
LINE_FEED equ 13,10

system_init:
	cld
	mov	[malloc_freelist],0
	mov	ah,2Ah
	int	21h
	push	dx cx
	movzx	ecx,cx
	mov	eax,ecx
	sub	eax,1970
	mov	ebx,365
	mul	ebx
	mov	ebp,eax
	mov	eax,ecx
	sub	eax,1969
	shr	eax,2
	add	ebp,eax
	mov	eax,ecx
	sub	eax,1901
	mov	ebx,100
	div	ebx
	sub	ebp,eax
	mov	eax,ecx
	xor	edx,edx
	sub	eax,1601
	mov	ebx,400
	div	ebx
	add	ebp,eax
	movzx	ecx,byte [esp+3]
	mov	eax,ecx
	dec	eax
	mov	ebx,30
	mul	ebx
	add	ebp,eax
	cmp	ecx,8
	jbe	months_correction
	mov	eax,ecx
	sub	eax,7
	shr	eax,1
	add	ebp,eax
	mov	ecx,8
    months_correction:
	mov	eax,ecx
	shr	eax,1
	add	ebp,eax
	cmp	ecx,2
	pop	cx
	jbe	day_correction_ok
	sub	ebp,2
	test	ecx,11b
	jnz	day_correction_ok
	xor	edx,edx
	mov	eax,ecx
	mov	ebx,100
	div	ebx
	or	edx,edx
	jnz	day_correction
	mov	eax,ecx
	mov	ebx,400
	div	ebx
	or	edx,edx
	jnz	day_correction_ok
    day_correction:
	inc	ebp
    day_correction_ok:
	pop	dx
	movzx	eax,dl
	dec	eax
	add	eax,ebp
	mov	ebx,24
	mul	ebx
	push	eax
	mov	ah,2Ch
	int	21h
	pop	eax
	push	dx
	movzx	ebx,ch
	add	eax,ebx
	mov	ebx,60
	mul	ebx
	movzx	ebx,cl
	add	eax,ebx
	mov	ebx,60
	mul	ebx
	pop	bx
	movzx	ebx,bh
	add	eax,ebx
	adc	edx,0
	mov	dword [timestamp],eax
	mov	dword [timestamp+4],edx
	retn

system_shutdown:
      ;  call	 mcheck
	retn

dos_int:
	push	0
	push	0
	push	0
	pushw	buffer_segment
	pushw	buffer_segment
	stc
	pushfw
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	0
	push	ebp
	push	esi
	push	edi
	mov	ax,300h
	mov	bx,21h
	xor	cx,cx
	mov	edi,esp
	push	es ss
	pop	es
	int	31h
	pop	es
	mov	edi,[esp]
	mov	esi,[esp+4]
	mov	ebp,[esp+8]
	mov	ebx,[esp+10h]
	mov	edx,[esp+14h]
	mov	ecx,[esp+18h]
	mov	ah,[esp+20h]
	sahf
	mov	eax,[esp+1Ch]
	lea	esp,[esp+32h]
	retn

open:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	push	esi edi
	call	adapt_path
	mov	ax,716Ch
	mov	bx,100000b
	mov	dx,1
	xor	cx,cx
	xor	si,si
	call	dos_int
	jnc	open_done
	cmp	ax,7100h
	je	old_open
	stc
	jmp	open_done
    old_open:
	mov	ax,3D00h
	xor	dx,dx
	call	dos_int
    open_done:
	mov	bx,ax
	pop	edi esi
	retn
    adapt_path:
	mov	esi,edx
	mov	edi,buffer
      copy_path:
	lodsb
	cmp	al,'/'
	jne	path_char_ok
	mov	al,'\'
      path_char_ok:
	stosb
	cmp	edi,buffer+BUFFER_SIZE
	jae	out_of_memory
	test	al,al
	jnz	copy_path
	retn

create:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	push	esi edi
	call	adapt_path
	mov	ax,716Ch
	mov	bx,100001b
	mov	dx,10010b
	xor	cx,cx
	xor	si,si
	xor	di,di
	call	dos_int
	jnc	create_done
	cmp	ax,7100h
	je	old_create
	stc
	jmp	create_done
    old_create:
	mov	ah,3Ch
	xor	cx,cx
	xor	dx,dx
	call	dos_int
    create_done:
	mov	bx,ax
	pop	edi esi
	retn

write:
; in: ebx = file handle, edx - data, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	esi edi ebp
	mov	ebp,ecx
	mov	esi,edx
    write_loop:
	mov	ecx,BUFFER_SIZE
	sub	ebp,BUFFER_SIZE
	jnc	do_write
	add	ebp,BUFFER_SIZE
	mov	ecx,ebp
	xor	ebp,ebp
    do_write:
	push	ecx
	mov	edi,buffer
	shr	ecx,2
	rep	movsd
	mov	ecx,[esp]
	and	ecx,11b
	rep	movsb
	pop	ecx
	mov	ah,40h
	xor	dx,dx
	call	dos_int
	or	ebp,ebp
	jnz	write_loop
	pop	ebp edi esi
	ret

read:
; in: ebx = file handle, edx - buffer, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	esi edi ebp
	mov	ebp,ecx
	mov	edi,edx
    read_loop:
	mov	ecx,BUFFER_SIZE
	sub	ebp,BUFFER_SIZE
	jnc	do_read
	add	ebp,BUFFER_SIZE
	mov	ecx,ebp
	xor	ebp,ebp
    do_read:
	push	ecx
	mov	ah,3Fh
	xor	dx,dx
	call	dos_int
	cmp	ax,cx
	jne	read_eof
	mov	esi,buffer
	mov	ecx,[esp]
	shr	ecx,2
	rep	movsd
	pop	ecx
	and	ecx,11b
	rep	movsb
	or	ebp,ebp
	jnz	read_loop
    read_done:
	pop	ebp edi esi
	ret
    read_eof:
	pop	ecx
	stc
	jmp	read_done

close:
; in: ebx = file handle
; preserves: ebx, esi, edi
	mov	ah,3Eh
	int	21h
	ret

lseek:
; in: ebx = file handle, cl = method, edx:eax = offset
; out: edx:eax = new offset from the beginning of file, cf set on error
; preserves: ebx, esi, edi
	mov	dx,ax
	xchg	ecx,eax
	shr	ecx,16
	mov	ah,42h
	int	21h
	pushf
	shl	edx,16
	popf
	mov	dx,ax
	mov	eax,edx
	xor	edx,edx
	ret

get_timestamp:
; out: edx:eax = timestamp
; preserves: ebx, ecx, esi, edi
	mov	eax,dword [timestamp]
	mov	edx,dword [timestamp+4]
	retn

display_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	ebx
	mov	ebx,1
	jmp	write_string
display_error_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	ebx
	mov	ebx,2
   write_string:
	test	ecx,ecx
	jnz	string_length_ok
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
     string_length_ok:
	mov	edx,esi
	call	write
	pop	ebx
	retn

get_environment_variable:
; in:
;  esi - name
;  edi - buffer for value
;  ecx = size of buffer
; out:
;  eax = length of value
; preserves: ebx, esi, edi
	push	ebx esi edi
	mov	ebx,esi
	xor	esi,esi
    compare_variable_names:
	mov	edx,ebx
    compare_name_character:
	lods	byte [gs:esi]
	mov	ah,[edx]
	inc	edx
	cmp	al,'='
	je	end_of_variable_name
	test	ah,ah
	jz	next_variable
	sub	ah,al
	jz	compare_name_character
	cmp	ah,20h
	jne	next_variable
	cmp	al,41h
	jb	next_variable
	cmp	al,5Ah
	jna	compare_name_character
    next_variable:
	lods	byte [gs:esi]
	test	al,al
	jnz	next_variable
	cmp	byte [gs:esi],0
	jne	compare_variable_names
	mov	ah,al
    end_of_variable_name:
	test	ah,ah
	jnz	next_variable
	add	ecx,edi
	mov	edx,esi
    copy_variable_value:
	lods	byte [gs:esi]
	cmp	edi,ecx
	jae	variable_value_next_character
	stosb
    variable_value_next_character:
	or	al,al
	jnz	copy_variable_value
	lea	eax,[esi-1]
	sub	eax,edx
	pop	edi esi ebx
	ret

include 'malloc.inc'
