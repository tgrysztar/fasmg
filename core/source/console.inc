
show_display_data:
	test	[trace_mode],TRACE_DISPLAY
	jnz	display_line_feed
	mov	ecx,[display_data_length]
	jecxz	display_data_shown
	mov	esi,[display_buffer]
	call	display_string
      display_line_feed:
	mov	esi,_new_line
	xor	ecx,ecx
	call	display_string
      display_data_shown:
	retn

show_errors:
	mov	esi,[first_error]
    display_error:
	test	esi,esi
	jz	display_data_shown
	push	esi
	mov	eax,[esi+sizeof.Error+SourceContext.number_of_entries]
	test	eax,eax
	jz	show_error_message
	lea	ebx,[esi+sizeof.Error+sizeof.SourceContext]
	dec	eax
	imul	eax,sizeof.SourceEntry
	lea	eax,[ebx+eax]
	mov	[last_source_entry],eax
	test	[trace_mode],TRACE_ERROR_STACK
	jnz	show_source_context
	and	[last_file_source_entry],0
    find_last_file_entry:
	cmp	[eax+SourceEntry.type],SOURCE_FILE
	je	last_file_entry_found
	cmp	eax,ebx
	je	show_source_context
	sub	eax,sizeof.SourceEntry
	jmp	find_last_file_entry
    last_file_entry_found:
	mov	[last_file_source_entry],eax
    show_source_context:
	push	ebx
	cmp	[ebx+SourceEntry.type],SOURCE_MEMORY
	je	display_memory_source
	cmp	[ebx+SourceEntry.type],SOURCE_MACRO
	jne	display_source_name
	mov	esi,_macro_source
	test	[ebx+SourceEntry.flags],SRCF_PREPROCESSED
	jz	display_source_type
	mov	esi,_preprocessed_source
      display_source_type:
	xor	ecx,ecx
	call	display_error_string
      display_source_name:
	mov	esi,[ebx+SourceEntry.name]
	test	esi,esi
	jz	unnamed_source
	mov	ecx,[ebx+SourceEntry.name_length]
	cmp	ecx,-1
	je	display_source_symbol
	call	display_error_string
	jmp	display_line_number
      display_source_symbol:
	xchg	ebx,esi
	call	show_symbol_name
	mov	ebx,esi
	jmp	display_line_number
      unnamed_source:
	mov	esi,_unnamed_source
	xor	ecx,ecx
	call	display_error_string
	jmp	display_line_number
      display_memory_source:
	mov	esi,_memory_source
	xor	ecx,ecx
	call	display_error_string
      display_line_number:
	mov	esi,_line_number_prefix
	xor	ecx,ecx
	call	display_error_string
	mov	eax,[ebx+SourceEntry.line_number]
	xor	edx,edx
	call	itoa
	call	display_error_string
	mov	esi,_line_number_suffix
	xor	ecx,ecx
	call	display_error_string
	pop	ebx
	mov	esi,[esp]
	push	ebx
	cmp	[ebx+SourceEntry.line_number],0
	je	skip_line_content
	test	[trace_mode],TRACE_ERROR_STACK
	jnz	show_source_line
	cmp	ebx,[last_source_entry]
	je	last_source_entry_line_content
	cmp	ebx,[last_file_source_entry]
	je	show_source_line
      skip_line_content:
	mov	esi,_space
      next_source_entry:
	pop	ebx
      find_next_source_entry:
	cmp	ebx,[last_source_entry]
	je	source_context_shown
	add	ebx,sizeof.SourceEntry
	test	[trace_mode],TRACE_ERROR_STACK
	jnz	show_source_entry
	test	[ebx+SourceEntry.flags],SRCF_PREPROCESSED
	jnz	find_next_source_entry
      show_source_entry:
	xor	ecx,ecx
	call	display_error_string
	jmp	show_source_context
      last_source_entry_line_content:
	test	[esi+Error.flags],ERR_CUSTOM
	jnz	skip_line_content
      show_source_line:
	cmp	[ebx+SourceEntry.type],SOURCE_CALM
	je	show_calm_source
	mov	esi,_line_content_prefix
	xor	ecx,ecx
	call	display_error_string
	call	show_line_content
	mov	esi,_new_line
	jmp	next_source_entry
      show_calm_source:
	mov	esi,_calm_source
	xor	ecx,ecx
	call	display_error_string
	mov	esi,_new_line
	jmp	next_source_entry
      source_context_shown:
	mov	esi,_new_line
	xor	ecx,ecx
	call	display_error_string
	mov	ebx,[esp]
	test	[ebx+Error.flags],ERR_CUSTOM
	jnz	show_error_message
	cmp	[ebx+Error.preprocessed_length],0
	je	show_error_message
	mov	esi,_preprocessed_text_prefix
	xor	ecx,ecx
	call	display_error_string
	mov	esi,[ebx+Error.preprocessed_data]
	mov	ecx,[ebx+Error.preprocessed_length]
	call	show_preprocessed_line
	mov	esi,_new_line
	xor	ecx,ecx
	call	display_error_string
    show_error_message:
	pop	ebx
	mov	esi,_error_prefix
	test	[ebx+Error.flags],ERR_CUSTOM
	jz	display_error_prefix
	mov	esi,_custom_error_prefix
      display_error_prefix:
	xor	ecx,ecx
	call	display_error_string
	mov	esi,[ebx+Error.message]
	test	[ebx+Error.flags],ERR_CUSTOM
	jz	format_error_message
	xor	ecx,ecx
	call	display_error_string
      finish_error_message:
	mov	esi,_message_suffix
	xor	ecx,ecx
	call	display_error_string
	mov	esi,ebx
    next_error:
	mov	esi,[esi+Error.next]
	jmp	display_error
    format_error_message:
	mov	edx,esi
      cut_error_message:
	lodsb
	test	al,al
	jz	show_error_message_segment
	cmp	al,'%'
	jne	cut_error_message
      show_error_message_segment:
	dec	esi
	push	esi
	xchg	ecx,esi
	sub	ecx,edx
	mov	esi,edx
	call	display_error_string
	pop	esi
	lodsb
	test	al,al
	jz	finish_error_message
	lodsb
	cmp	al,'s'
	je	insert_string_into_error_message
	cmp	al,'i'
	jne	format_error_message
	push	ebx
	mov	ebx,[ebx+Error.symbol]
	mov	ebx,[ebx+SymbolTree_Leaf.branch]
	call	show_symbol_name
	pop	ebx
	jmp	format_error_message
      insert_string_into_error_message:
	push	esi
	mov	esi,[ebx+Error.symbol]
	xor	ecx,ecx
	call	display_error_string
	pop	esi
	jmp	format_error_message

show_symbol_name:
; in:
;  ebx - SymbolTree_Foliage, may be null
; preserves: esi
	test	ebx,ebx
	jz	symbol_name_shown
	mov	edi,[identifier_workspace.memory_start]
	cmp	[ebx+SymbolTree_Foliage.name_kind],NAME_NUMERIC
	je	next_name_segment
      compose_symbol_name:
	mov	ecx,[ebx+SymbolTree_Foliage.name_length]
	mov	edx,identifier_workspace
	mov	al,[ebx+SymbolTree_Foliage.name_kind]
	cmp	al,NAME_CASESENSITIVE
	je	name_segment_to_copy
	cmp	al,NAME_ABSTRACT
	je	name_segment_copied
	cmp	al,NAME_NUMERIC
	je	dot_label_name
	mov	al,'?'
	stosb
      name_segment_to_copy:
	push	ecx
	add	ecx,2
	call	reserve_workspace
	pop	ecx
	mov	edx,[ebx+SymbolTree_Foliage.name_data]
      copy_name_segment:
	jecxz	name_segment_copied
	dec	ecx
	mov	al,[edx+ecx]
	stosb
	jmp	copy_name_segment
      dot_label_name:
	push	esi
	mov	esi,[ebx+SymbolTree_Foliage.name_data]
	xor	eax,eax
      read_dot_count:
	jecxz	dot_count_read
	dec	ecx
	shl	eax,8
	mov	al,[esi+ecx]
	jmp	read_dot_count
      dot_count_read:
	pop	esi
	push	eax
	lea	ecx,[eax+2]
	call	reserve_workspace
	pop	ecx
	mov	al,'.'
	rep	stosb
      name_segment_copied:
	mov	edx,[ebx+SymbolTree_Foliage.root]
	mov	ebx,[edx+SymbolTree_Root.parent_branch]
	test	[edx+SymbolTree_Root.flags],NAMESPACE_LOCAL or NAMESPACE_CALM
	jnz	mark_local_symbol_name
	test	ebx,ebx
	jz	symbol_name_ready
      next_name_segment:
	mov	al,'.'
	stosb
	jmp	compose_symbol_name
      mark_local_symbol_name:
	mov	al,':'
	stosb
	test	[edx+SymbolTree_Root.flags],NAMESPACE_CALM
	jz	symbol_name_ready
	mov	eax,[ebx+SymbolTree_Foliage.name_data]
	mov	ebx,[eax+SymbolTree_Leaf.branch]
	test	ebx,ebx
	jnz	compose_symbol_name
      symbol_name_ready:
	mov	ebx,[identifier_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,ebx
	jz	symbol_name_shown
	push	esi
	mov	esi,ebx
      reverse_composed_name:
	dec	edi
	cmp	ebx,edi
	jae	show_composed_name
	mov	al,[ebx]
	xchg	al,[edi]
	mov	[ebx],al
	inc	ebx
	jmp	reverse_composed_name
      show_composed_name:
	call	display_error_string
	pop	esi
      symbol_name_shown:
	retn

show_line_content:
; in:
;  ebx - SourceEntry
	cmp	[ebx+SourceEntry.type],SOURCE_MACRO
	je	show_line_from_macro
	mov	esi,[ebx+SourceEntry.text]
	add	esi,[ebx+SourceEntry.line_offset]
	mov	ecx,[ebx+SourceEntry.number_of_attached_lines]
	inc	ecx
	mov	[number_of_lines],ecx
    show_token:
	mov	al,[esi]
	test	al,al
	jz	line_content_shown
	cmp	al,0Ah
	je	line_content_shown
	cmp	al,1Ah
	je	show_name_token
	cmp	al,22h
	je	show_string_token
	cmp	al,27h
	je	show_string_token
	cmp	al,'\'
	jne	show_basic_token
	cmp	byte [esi+1],0Ah
	jne	show_basic_token
	dec	[number_of_lines]
	jnz	show_attached_line
    show_basic_token:
	mov	ecx,1
	call	display_error_string
	inc	esi
	jmp	show_token
    show_name_token:
	add	esi,1+4
	mov	ecx,[esi-4]
	call	display_error_string
	add	esi,[esi-4]
	add	esi,12
	jmp	show_token
    show_string_token:
	mov	ebx,esi
	inc	esi
	call	show_string_token_content
	lea	esi,[ebx+1]
	lodsd
	add	esi,eax
	jmp	show_token
    show_string_token_content:
	lea	edi,[esi+4]
	mov	ecx,[esi]
    show_string_segment:
	push	ecx edi
	mov	esi,_single_quote
	mov	ecx,1
	call	display_error_string
	pop	edi ecx
	jecxz	show_end_quote
	mov	edx,ecx
	mov	al,27h
	repne	scasb
	sub	edx,ecx
	mov	esi,edi
	sub	esi,edx
	push	ecx edi
	mov	ecx,edx
	call	display_error_string
	pop	edi ecx
	test	ecx,ecx
	jnz	show_string_segment
    show_end_quote:
	cmp	byte [ebx],27h
	je	string_token_shown
	mov	esi,_single_quote
	mov	ecx,1
	call	display_error_string
    string_token_shown:
	retn
    show_attached_line:
	mov	ecx,1
	call	display_error_string
	lea	ebx,[esi+2]
	mov	esi,_line_segment_prefix
	xor	ecx,ecx
	call	display_error_string
	mov	esi,ebx
	jmp	show_token
    show_line_from_macro:
	mov	edx,[ebx+SourceEntry.text]
	mov	esi,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	eax,[ebx+SourceEntry.line_offset]
	add	esi,eax
	sub	ecx,eax
	jbe	line_content_shown
	call	show_preprocessed_line
    line_content_shown:
	retn

show_preprocessed_line:
; in:
;  esi - preprocessed tokens
;  ecx = total length of preprocessed tokens
	lea	eax,[esi+ecx]
	mov	[preprocessed_text_end],eax
    show_preprocessed_token:
	cmp	esi,[preprocessed_text_end]
	jae	preprocessed_line_shown
	mov	al,[esi]
	test	al,al
	jz	preprocessed_line_shown
	cmp	al,1Ah
	je	show_preprocessed_name_token
	cmp	al,22h
	je	show_preprocessed_string_token
	cmp	al,27h
	je	show_preprocessed_string_token
	cmp	al,30h
	je	show_internal_number
	cmp	al,40h
	je	show_context_token
	mov	ecx,1
	call	display_error_string
	inc	esi
	jmp	show_preprocessed_token
    show_preprocessed_name_token:
	inc	esi
	lodsd
	mov	ebx,esi
	mov	esi,eax
	lodsd
	mov	ecx,eax
	call	display_error_string
	mov	esi,ebx
	jmp	show_preprocessed_token
    show_preprocessed_string_token:
	mov	ebx,esi
	mov	esi,[esi+1]
	call	show_string_token_content
	lea	esi,[ebx+1+4]
	jmp	show_preprocessed_token
    show_internal_number:
	inc	esi
	mov	edx,esi
	push	esi
	call	convert_number_back
	lea	esi,[edx+4]
	mov	ecx,[edx]
	call	display_error_string
	pop	esi
	add	esi,[esi]
	add	esi,4
	jmp	show_preprocessed_token
    show_context_token:
	add	esi,1+sizeof.RecognitionContext
	jmp	show_preprocessed_token
    preprocessed_line_shown:
	retn

itoa:
; in:
;  edx:eax = unsigned number
; out:
;  esi - temporary buffer containing decimal digits
;  ecx = length of string (number of digits)
	mov	edi,temporary_value+4
	stosd
	mov	eax,edx
	stosd
	mov	edx,temporary_value
	mov	dword [edx],8
	call	convert_number_back
	lea	esi,[edx+4]
	mov	ecx,[edx]
	retn
