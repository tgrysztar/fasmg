
struct Error
	flags dd ?			 ; ERR_#
	message dd ?
	preprocessed_data dd ?
	preprocessed_length dd ?
	next dd ?			 ; pointer to another Error
      ; source SourceContext
ends

ERR_CUSTOM = 1

register_volatile_error:
; in:
;  edx - 32-bit length followed by string data
; out:
;  edx - Error
	cmp	[next_pass_needed],0
	jne	error_ignored
	push	eax ebx ecx esi edi
	or	[message_volatile],1
	jmp	get_error_line
register_delayed_error:
; in:
;  edx - error message
;  esi - SourceContext
; out:
;  edx - Error
; preserves: eax, ebx, ecx, esi, edi
	cmp	[next_pass_needed],0
	jne	error_ignored
	push	eax ebx ecx esi edi
	and	[message_volatile],0
	and	[error_line_start],0
	and	[error_line_end],0
	jmp	add_error
register_error:
; in:
;  edx - error message
; out:
;  edx - Error, null when error was not registered
; preserves: eax, ebx, ecx, esi, edi
	cmp	[next_pass_needed],0
	jne	error_ignored
	push	eax ebx ecx esi edi
	and	[message_volatile],0
    get_error_line:
	mov	eax,[line_start]
	mov	[error_line_start],eax
	mov	ecx,[line_end]
	cmp	[number_of_line_embeddings],0
	je	error_line_end_ok
	mov	eax,[line_embeddings]
	mov	ecx,[eax+LineEmbedding.previous_end]
    error_line_end_ok:
	mov	[error_line_end],ecx
	xor	esi,esi
    add_error:
	lea	ebx,[first_error]
	xor	ecx,ecx
    find_last_error:
	mov	eax,[ebx]
	test	eax,eax
	jz	last_error_found
	lea	ebx,[eax+Error.next]
	inc	ecx
	cmp	ecx,[maximum_number_of_errors]
	jb	find_last_error
	pop	edi esi ecx ebx eax
	xor	edx,edx
	retn
    last_error_found:
	cmp	[message_volatile],0
	jne	prepare_volatile_error
	test	esi,esi
	jnz	prepare_error
	mov	esi,[source_context]
    prepare_error:
	mov	ecx,[esi+SourceContext.number_of_entries]
	imul	ecx,sizeof.SourceEntry
	add	ecx,sizeof.SourceContext
	add	ecx,sizeof.Error
	add	ecx,[error_line_end]
	sub	ecx,[error_line_start]
	mov	edi,edx
	call	malloc
	jc	out_of_memory
	mov	[ebx],eax
	mov	[eax+Error.message],edi
	xor	ecx,ecx
	mov	[eax+Error.flags],ecx
	mov	[eax+Error.next],ecx
	lea	edi,[eax+sizeof.Error]
	push	eax
	call	clone_source_context
	pop	edx
    store_preprocessed_data:
	mov	[edx+Error.preprocessed_data],edi
	mov	esi,[error_line_start]
	mov	ecx,[error_line_end]
	sub	ecx,esi
	mov	[edx+Error.preprocessed_length],ecx
	rep	movsb
	pop	edi esi ecx ebx eax
	retn
    error_ignored:
	xor	edx,edx
	retn
    prepare_volatile_error:
	mov	esi,edx
	mov	eax,[source_context]
	mov	ecx,[eax+SourceContext.number_of_entries]
	imul	ecx,sizeof.SourceEntry
	add	ecx,sizeof.SourceContext
	add	ecx,sizeof.Error
	add	ecx,[error_line_end]
	sub	ecx,[error_line_start]
	mov	edi,ecx
	add	ecx,[esi]
	inc	ecx
	call	malloc
	jc	out_of_memory
	add	edi,eax
	mov	[ebx],eax
	mov	edx,eax
	mov	[edx+Error.message],edi
	xor	eax,eax
	mov	[edx+Error.next],eax
	mov	[edx+Error.flags],eax
	lodsd
	mov	ecx,eax
	rep	movsb
	xor	al,al
	stosb
	mov	esi,[source_context]
	lea	edi,[edx+sizeof.Error]
	push	edx
	call	clone_source_context
	pop	edx
	jmp	store_preprocessed_data

discard_errors:
	mov	eax,[first_error]
	test	eax,eax
	jnz	discard_error
	retn
    discard_error:
	add	eax,sizeof.Error
	call	release_source_context
	sub	eax,sizeof.Error
	mov	ebx,[eax+Error.next]
	call	mfree
	mov	eax,ebx
	test	eax,eax
	jnz	discard_error
	mov	[first_error],eax
	retn