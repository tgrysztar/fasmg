
; Adapted and tested by Jacob Young (jacobly.alt@gmail.com)

LINE_FEED = 0Ah

system_init:
	ccall	libc.time,timestamp
	or	[local_heap_available],1
	retn

system_shutdown:
	retn

open:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	push	rsi rdi
	call	adapt_path
	ccall	libc.fopen,rbx,_open_mode
    put_file_entry:
	pop	rdi rsi
	test	rax,rax
	jz	interface_error
	push	rax
	mov	eax,[files]
	mov	ecx,[files_count]
	mov	ebx,ecx
	inc	ecx
	mov	[files_count],ecx
	cmp	ecx,[files_maximum_count]
	ja	grow_files_buffer
      store_file_entry:
	pop	rdx
	mov	[eax+ebx*8],rdx
	clc
	retn
      interface_error:
	stc
	retn
      grow_files_buffer:
	shl	ecx,4
	test	eax,eax
	jz	allocate_files_buffer
	call	realloc
	jmp	allocated_files_buffer
      allocate_files_buffer:
	call	malloc
      allocated_files_buffer:
	mov	[files],eax
	shr	ecx,3
	mov	[files_maximum_count],ecx
	jmp	store_file_entry
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
	push	rsi rdi
	call	adapt_path
	ccall	libc.fopen,rbx,_create_mode
	jmp	put_file_entry
write:
; in: ebx = file handle, edx - data, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	rbx rcx rsi rdi
	mov	eax,[files]
	mov	rax,[eax+ebx*8]
	ccall	libc.fwrite,rdx,1,rcx,rax
	pop	rdi rsi rcx rbx
	cmp	eax,ecx
	jne	interface_error
	clc
	ret
read:
; in: ebx = file handle, edx - buffer, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	rbx rcx rsi rdi
	mov	eax,[files]
	mov	rax,[eax+ebx*8]
	ccall	libc.fread,rdx,1,rcx,rax
	pop	rdi rsi rcx rbx
	cmp	eax,ecx
	jne	interface_error
	clc
	ret
close:
; in: ebx = file handle
; preserves: ebx, esi, edi
	push	rsi rdi
	mov	edi,[files]
	mov	rdi,[edi+ebx*8]
	ccall	libc.fclose,rdi
	pop	rdi rsi
	ret
lseek:
; in: ebx = file handle, cl = method, edx:eax = offset
; out: edx:eax = new offset from the beginning of file, cf set on error
; preserves: ebx, esi, edi
	push	rsi rdi rbx
	shl	rdx,32
	or	rax,rdx
	movzx	ecx,cl
	mov	edi,[files]
	mov	rdi,[edi+ebx*8]
	push	rdi
	ccall	libc.fseek,rdi,rax,rcx
	test	eax,eax
	jnz	lseek_error
	pop	rdi
	ccall	libc.ftell,rdi
	cmp	rax,-1
	je	lseek_error
	mov	rdx,rax
	shr	rdx,32
	mov	eax,eax
	pop	rbx rdi rsi
	clc
	ret
    lseek_error:
	pop	rbx rdi rsi
	stc
	ret

get_timestamp:
; out: edx:eax = timestamp
; preserves: ebx, ecx, esi, edi
; note: during the passes of a single assembly this function should always return the same value
	mov	eax,dword [timestamp]
	mov	edx,dword [timestamp+4]
	retn

display_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	rbx rsi
	test	ecx,ecx
	jnz	write_string_to_stdout
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
   write_string_to_stdout:
	ccall	libc.write,1,rsi,rcx
	pop	rsi rbx
	retn

display_error_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	rbx rsi
	test	ecx,ecx
	jnz	write_string_to_stderr
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
   write_string_to_stderr:
	ccall	libc.write,2,rsi,rcx
	pop	rsi rbx
	retn

get_environment_variable:
; in:
;  esi - name
;  edi - buffer for value
;  ecx = size of buffer
; out:
;  eax = length of value
; preserves: ebx, esi, edi
	push	rbx rcx rsi rdi
	ccall	getenv,rsi
	pop	rdi rsi rcx rbx
	test	rax,rax
	jz	no_environment_variable
	push	rsi
	mov	rsi,rax
	xor	eax,eax
      copy_environment_variable:
	mov	dl,[rsi+rax]
	cmp	eax,ecx
	jae	next_environment_variable_character
	mov	[edi+eax],dl
      next_environment_variable_character:
	inc	eax
	test	dl,dl
	jnz	copy_environment_variable
	pop	rsi
      environment_variable_ok:
	ret
      no_environment_variable:
	mov	eax,1
	jecxz	environment_variable_ok
	and	byte [edi],0
	ret

VALLOC_MINIMUM_SIZE = 100000h

valloc:
; in: ecx = requested minimum size
; out: eax - allocated block, ecx = allocated size, zero if failed
; preserves: ebx, esi, edi
	cmp	ecx,VALLOC_MINIMUM_SIZE
	jbe	valloc_size_minimum
	dec	ecx
	and	ecx,(-1) shl 12
	add	ecx,1 shl 12
	jmp	valloc_size_ready
    valloc_size_minimum:
	mov	ecx,VALLOC_MINIMUM_SIZE
    valloc_size_ready:
	push	rbx rsi rdi
	cmp	[local_heap_available],0
	je	valloc_mmap
	cmp	ecx,LOCAL_HEAP_SIZE
	ja	valloc_mmap
	and	[local_heap_available],0
	mov	eax,local_heap
	mov	ecx,LOCAL_HEAP_SIZE
	jmp	valloc_ok
    valloc_mmap:
	push	rcx
	ccall	mmap,0,rcx, \
			3, \		; PROT_READ + PROT_WRITE
			9002h, \	; MAP_PRIVATE + MAP_ANONYMOUS + MAP_32BIT
			-1,0
	pop	rsi
	cmp	eax,-1
	je	valloc_mmap_with_hint
	mov	ecx,eax
	cmp	rcx,rax
	jne	valloc_mmap_unusable
	add	ecx,esi
	jnc	mmap_ok
    valloc_mmap_unusable:
	ccall	munmap,rax,rsi
    valloc_mmap_with_hint:
	push	rsi
	mov	edi,[mmap_hint]
	ccall	mmap,rdi,rsi, \
			3, \		; PROT_READ + PROT_WRITE
			1002h, \	; MAP_PRIVATE + MAP_ANONYMOUS
			-1,0
	pop	rsi
	cmp	eax,-1
	je	valloc_failed
	mov	ecx,eax
	cmp	rcx,rax
	jne	valloc_failed
	add	ecx,esi
	jc	valloc_failed  
    mmap_ok:
	sub	ecx,eax
    valloc_ok:
	lea	edx,[eax+ecx]
	mov	[mmap_hint],edx
	pop	rdi rsi rbx
	retn
    valloc_failed:
	xor	ecx,ecx
	pop	rdi rsi rbx
	retn