
LINE_FEED = 0Ah

system_init:
	ccall	libc.time,timestamp
	retn

system_shutdown:
	retn

malloc:
malloc_fixed:
malloc_growable:
; in: ecx = requested size
; out: eax - allocated block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
; note:
;  use of malloc_fixed hints that block will be kept as is until the end of assembly
;  use of malloc_growable hints that block is likely to be resized
	push	ebx ecx esi edi
	ccall	libc.malloc,ecx
	pop	edi esi ecx ebx
	test	eax,eax
	jz	out_of_memory
	retn
realloc:
; in: eax - memory block, ecx = requested size
; out: eax - resized block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
	push	ebx ecx esi edi
	ccall	libc.realloc,eax,ecx
	pop	edi esi ecx ebx
	test	eax,eax
	jz	out_of_memory
	retn
mfree:
; in: eax - memory block
; out: cf set on error
; preserves: ebx, esi, edi
; note: eax may have value 0 or -1, it should be treated as invalid input then
	test	eax,eax
	jz	interface_error
	cmp	eax,-1
	je	interface_error
	push	ebx esi edi
	ccall	libc.free,eax
	pop	edi esi ebx
	clc
	retn
    interface_error:
	stc
	retn

open:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	push	esi edi
	call	adapt_path
	ccall	libc.fopen,ebx,_open_mode
	pop	edi esi
	test	eax,eax
	jz	interface_error
	mov	ebx,eax
	clc
	retn
    adapt_path:
	xor	ecx,ecx
	mov	ebx,path_buffer
      copy_path:
	mov	al,[edx+ecx]
	cmp	al,'\'
	jne	path_char_ok
	mov	al,'/'
      path_char_ok:
	cmp	ecx,1000h
	jae	out_of_memory
	mov	[ebx+ecx],al
	inc	ecx
	test	al,al
	jnz	copy_path
	retn
create:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	push	esi edi
	call	adapt_path
	ccall	libc.fopen,ebx,_create_mode
	pop	edi esi
	test	eax,eax
	jz	interface_error
	mov	ebx,eax
	clc
	retn
write:
; in: ebx = file handle, edx - data, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	ebx ecx esi edi
	ccall	libc.fwrite,edx,1,ecx,ebx
	pop	edi esi ecx ebx
	cmp	eax,ecx
	jne	interface_error
	clc
	ret
read:
; in: ebx = file handle, edx - buffer, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	ebx ecx esi edi
	ccall	libc.fread,edx,1,ecx,ebx
	pop	edi esi ecx ebx
	cmp	eax,ecx
	jne	interface_error
	clc
	ret
close:
; in: ebx = file handle
; preserves: ebx, esi, edi
	ccall	libc.fclose,ebx
	ret
lseek:
; in: ebx = file handle, cl = method, edx:eax = offset
; out: edx:eax = new offset from the beginning of file, cf set on error
; preserves: ebx, esi, edi
	test	edx,edx
	jnz	interface_error
	push	esi edi ebx
	movzx	ecx,cl
	ccall	libc.fseek,ebx,eax,ecx
	test	eax,eax
	jnz	lseek_error
	mov	ebx,[esp]
	ccall	libc.ftell,ebx
	cmp	eax,-1
	je	lseek_error
	xor	edx,edx
	pop	ebx edi esi
	clc
	ret
    lseek_error:
	pop	ebx edi esi
	stc
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
	push	ebx esi
	test	ecx,ecx
	jnz	write_string_to_stdout
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
   write_string_to_stdout:
	ccall	libc.write,1,esi,ecx
	pop	esi ebx
	retn

display_error_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	ebx esi
	test	ecx,ecx
	jnz	write_string_to_stderr
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
   write_string_to_stderr:
	ccall	libc.write,2,esi,ecx
	pop	esi ebx
	retn

get_environment_variable:
; in:
;  esi - name
;  edi - buffer for value
;  ecx = size of buffer
; out:
;  eax = length of value
; preserves: ebx, esi, edi
	push	ebx ecx esi edi
	ccall	getenv,esi
	pop	edi esi ecx ebx
	test	eax,eax
	jz	no_environment_variable
	push	esi
	mov	esi,eax
	xor	eax,eax
      copy_environment_variable:
	mov	dl,[esi+eax]
	cmp	eax,ecx
	jae	next_environment_variable_character
	mov	[edi+eax],dl
      next_environment_variable_character:
	inc	eax
	test	dl,dl
	jnz	copy_environment_variable
	pop	esi
      environment_variable_ok:
	ret
      no_environment_variable:
	mov	eax,1
	jecxz	environment_variable_ok
	and	byte [edi],0
	ret
