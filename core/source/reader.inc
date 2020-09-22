
struct FileData
	length dq ?
	cache dd ?	; pointer to FileCache
ends

struct FileCache
	offset dq ?
	length dd ?
	next dd ?	; pointer to another FileCache
ends

read_source:
; in:
;  esi - source path
; out:
;  eax - tokenized source, null when file not found
;  esi - source path in persistent storage
	mov	ebx,[file_source_cache]
	xor	ecx,ecx
	call	get_from_map
	jc	read_source_file
	cmp	eax,-1
	je	get_erroneous_source
	retn
    read_source_file:
	mov	edx,esi
	call	open
	jc	source_file_not_found
	xor	eax,eax
	mov	edx,eax
	mov	cl,2
	call	lseek
	jc	error_reading_file
	test	edx,edx
	jnz	out_of_memory
	push	eax
	xor	eax,eax
	mov	edx,eax
	mov	cl,al
	call	lseek
	jc	error_reading_file
	pop	ecx
	inc	ecx
	mov	[source_text_length],ecx
	call	malloc
	mov	[source_text],eax
	mov	edx,eax
	mov	ecx,[source_text_length]
	dec	ecx
	mov	byte [edx+ecx],0
	call	read
	jc	error_reading_file
	call	close
	push	esi
	call	tokenize_source
	mov	eax,[source_text]
	call	mfree
	pop	esi
	mov	eax,[tokenization_buffer]
	xor	ecx,ecx
	mov	ebx,[file_source_cache]
	call	put_into_map
	mov	eax,[tokenization_buffer]
	retn
    source_file_not_found:
	xor	eax,eax
	xor	ecx,ecx
	mov	ebx,[file_source_cache]
	call	put_into_map
	xor	eax,eax
	retn
    error_reading_file:
	or	eax,-1
	xor	ecx,ecx
	mov	ebx,[file_source_cache]
	call	put_into_map
    get_erroneous_source:
	mov	ebx,esi
	mov	edx,_error_reading_file
	call	register_error
	mov	eax,zero_value
	retn

use_source:
; in:
;  esi - ASCIIZ source string
; out:
;  eax - tokenized source
;  esi - source text in persistent storage
	mov	edi,esi
	xor	al,al
	or	ecx,-1
	repne	scasb
	not	ecx
	mov	[source_text_length],ecx
	mov	ebx,[memory_source_cache]
	xor	eax,eax
	call	get_from_map
	jc	adapt_memory_source
	retn
    adapt_memory_source:
	mov	[source_text],esi
	call	tokenize_source
	mov	eax,[tokenization_buffer]
	mov	esi,[source_text]
	mov	ecx,[source_text_length]
	mov	ebx,[memory_source_cache]
	call	put_into_map
	mov	eax,[tokenization_buffer]
	retn

tokenize_source:
; in:
;  [source_text] - ASCIIZ text
;  [source_text_length] = length of text (including terminating character)
; out:
;  [tokenization_buffer] - tokenized source
;  [tokenization_buffer_length] = length of tokenized source
	mov	ecx,[source_text_length]
	shl	ecx,1
	add	ecx,18
	call	malloc_growable
	mov	[tokenization_buffer],eax
	mov	[tokenization_buffer_length],ecx
	add	eax,ecx
	sub	eax,[source_text]
	sub	eax,[source_text_length]
	mov	[buffer_end_offset],eax
	mov	esi,[source_text]
	mov	edi,[tokenization_buffer]
	mov	[last_token],0Ah
    tokenize:
	mov	eax,[buffer_end_offset]
	add	eax,esi
	sub	eax,edi
	cmp	eax,18
	jae	tokenization_buffer_reserve_ok
	mov	ecx,esi
	sub	ecx,[source_text]
	mov	eax,[source_text_length]
	mul	[tokenization_buffer_length]
	div	ecx
	mov	ecx,eax
	add	ecx,18
	mov	eax,[tokenization_buffer]
	call	realloc
	sub	edi,[tokenization_buffer]
	add	edi,eax
	mov	[tokenization_buffer],eax
	mov	[tokenization_buffer_length],ecx
	add	eax,ecx
	sub	eax,[source_text]
	sub	eax,[source_text_length]
	mov	[buffer_end_offset],eax
    tokenization_buffer_reserve_ok:
	movzx	eax,byte [esi]
	inc	esi
	mov	ah,[characters+eax]
	cmp	ah,20h
	je	control_character
	test	ah,ah
	jnz	make_name_token
    character_token:
	stosb
	mov	[last_token],al
	jmp	tokenize
    make_string_token:
	mov	dl,al
	mov	byte [edi],22h
	mov	[last_token],22h
	add	edi,5
	xor	ecx,ecx
    copy_string:
	mov	al,[esi]
	cmp	al,0Dh
	je	broken_string
	cmp	al,0Ah
	je	broken_string
	cmp	al,1Ah
	je	broken_string
	test	al,al
	jz	broken_string
	inc	esi
	cmp	al,dl
	jne	copy_string_character
	cmp	byte [esi],al
	jne	finish_string_token
	inc	esi
    copy_string_character:
	mov	[edi+ecx],al
	inc	ecx
	jmp	copy_string
    broken_string:
	mov	byte [edi-5],27h
    finish_string_token:
	mov	al,[edi-5]
	mov	[edi-4],ecx
	add	edi,ecx
	jmp	tokenize
    make_name_token:
	cmp	al,22h
	je	make_string_token
	cmp	al,27h
	je	make_string_token
	mov	byte [edi],1Ah
	mov	[last_token],1Ah
	add	edi,5
	xor	ebx,ebx
	mov	ecx,FNV_OFFSET
	mov	edx,ecx
    hash_name:
	mov	[edi+ebx],al
	inc	ebx
	xor	cl,al
	xor	dl,ah
	imul	ecx,FNV_PRIME
	imul	edx,FNV_PRIME
	movzx	eax,byte [esi]
	inc	esi
	mov	ah,[characters+eax]
	cmp	ah,20h
	je	finish_name_token
	test	ah,ah
	jnz	hash_name
    finish_name_token:
	mov	[edi-4],ebx
	add	edi,ebx
	mov	[edi],ecx
	mov	[edi+4],edx
	xor	ecx,ecx
	mov	[edi+8],ecx
	add	edi,12
	cmp	ah,20h
	jne	character_token
    control_character:
	cmp	al,20h
	je	whitespace
	cmp	al,9
	je	whitespace
	cmp	[last_token],20h
	je	mark_end_of_line
	inc	edi
    mark_end_of_line:
	mov	byte [edi-1],0Ah
	mov	[last_token],0Ah
	cmp	al,0Dh
	je	cr
	cmp	al,0Ah
	je	lf
	xor	al,al
	stosb
	mov	ecx,edi
	mov	eax,[tokenization_buffer]
	sub	ecx,eax
	call	realloc
	mov	[tokenization_buffer],eax
	mov	[tokenization_buffer_length],ecx
	retn
    cr:
	cmp	byte [esi],0Ah
	jne	tokenize
	inc	esi
	jmp	tokenize
    lf:
	cmp	byte [esi],0Dh
	jne	tokenize
	inc	esi
	jmp	tokenize
    whitespace:
	cmp	[last_token],0Ah
	je	tokenize
	cmp	[last_token],20h
	je	tokenize
	mov	al,20h
	stosb
	mov	[last_token],al
	jmp	tokenize

get_file_data:
; in:
;  esi - file path
; out:
;  ebx - FileData, null when file not found
;  esi - file path in persistent storage
; preserves: edi
	mov	ebx,[file_data_cache]
	xor	ecx,ecx
	call	get_from_map
	jc	initialize_file_data
	mov	ebx,eax
	retn
    initialize_file_data:
	mov	edx,esi
	call	open
	jc	remember_file_not_found
	push	edi
	mov	ecx,sizeof.FileData
	call	malloc_fixed
	mov	edi,eax
	xor	eax,eax
	mov	edx,eax
	mov	cl,2
	call	lseek
	jc	file_not_seekable
	mov	dword [edi+FileData.length],eax
	mov	dword [edi+FileData.length+4],edx
	call	close
	mov	eax,edi
	xor	ecx,ecx
	mov	[eax+FileData.cache],ecx
	mov	ebx,[file_data_cache]
	call	put_into_map
	mov	ebx,edi
	pop	edi
	retn
    file_not_seekable:
	pop	edi
    remember_file_not_found:
	xor	eax,eax
	mov	ecx,eax
	mov	ebx,[file_data_cache]
	call	put_into_map
	xor	ebx,ebx
	retn

read_file_data:
; in:
;  esi - file path
;  ebx - FileData
;  edi - buffer for data
;  [file_offset] = offset of data
;  [data_length] = length of data
; out:
;  cf set when read failed
; preserves: esi
	mov	[file_data],ebx
	lea	eax,[ebx+FileData.cache]
	mov	[file_cache_pointer],eax
	mov	ebx,[eax]
    read_from_file_cache:
	mov	ecx,[data_length]
	test	ecx,ecx
	jz	file_data_read
	test	ebx,ebx
	jz	new_trailing_file_cache_entry
	mov	eax,dword [file_offset]
	mov	edx,dword [file_offset+4]
	sub	eax,dword [ebx+FileCache.offset]
	sbb	edx,dword [ebx+FileCache.offset+4]
	jc	new_file_cache_entry
	jnz	next_entry
	mov	edx,[ebx+FileCache.length]
	sub	edx,eax
	jbe	next_entry
	cmp	ecx,edx
	jbe	length_to_read_ok
	mov	ecx,edx
      length_to_read_ok:
	sub	[data_length],ecx
	add	dword [file_offset],ecx
	adc	dword [file_offset+4],0
	mov	edx,esi
	lea	esi,[ebx+sizeof.FileCache+eax]
	mov	al,cl
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	mov	esi,edx
    next_entry:
	lea	eax,[ebx+FileCache.next]
	mov	[file_cache_pointer],eax
	mov	ebx,[eax]
	jmp	read_from_file_cache
    file_data_read:
	clc
	retn
    new_trailing_file_cache_entry:
	mov	ebx,[file_data]
	mov	ecx,dword [ebx+FileData.length]
	mov	edx,dword [ebx+FileData.length+4]
	cmp	ecx,dword [file_offset]
	jne	measure_cache_gap
	cmp	edx,dword [file_offset+4]
	jne	measure_cache_gap
	stc
	retn
    new_file_cache_entry:
	mov	ecx,dword [ebx+FileCache.offset]
	mov	edx,dword [ebx+FileCache.offset+4]
      measure_cache_gap:
	mov	eax,dword [file_offset]
	and	eax,not 0FFFh
	sub	ecx,eax
	sbb	edx,dword [file_offset+4]
	jnz	compute_aligned_length
	cmp	ecx,[data_length]
	jbe	read_into_cache
      compute_aligned_length:
	mov	eax,dword [file_offset]
	and	eax,0FFFh
	add	eax,[data_length]
	dec	eax
	shr	eax,12
	inc	eax
	shl	eax,12
	test	edx,edx
	jnz	use_aligned_length
	cmp	eax,ecx
	jae	read_into_cache
      use_aligned_length:
	mov	ecx,eax
    read_into_cache:
	push	ecx
	add	ecx,sizeof.FileCache
	call	malloc_fixed
	mov	ebx,eax
	mov	eax,[file_cache_pointer]
	mov	edx,ebx
	xchg	edx,[eax]
	mov	[ebx+FileCache.next],edx
	pop	[ebx+FileCache.length]
	mov	eax,dword [file_offset]
	and	eax,not 0FFFh
	mov	edx,dword [file_offset+4]
	mov	dword [ebx+FileCache.offset],eax
	mov	dword [ebx+FileCache.offset+4],edx
	push	ebx edi
	mov	edi,ebx
	mov	edx,esi
	call	open
	jc	file_access_error
	mov	eax,dword [edi+FileCache.offset]
	mov	edx,dword [edi+FileCache.offset+4]
	xor	cl,cl
	call	lseek
	jc	file_access_error
	lea	edx,[edi+sizeof.FileCache]
	mov	ecx,[edi+FileCache.length]
	call	read
	jc	file_access_error
	call	close
	pop	edi ebx
	jmp	read_from_file_cache
     file_access_error:
	pop	edi ebx
	stc
	retn
	    