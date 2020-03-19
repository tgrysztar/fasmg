
struct OutputArea
	cached_offset dq ?
	definition dd ? 		; pointer to ValueDefinition
ends

struct AreaHeader
	flags dd ?			; AREA_#
	base_address_length dd ?
	uninitialized_length dd ?
ends

AREA_VIRTUAL = 1
AREA_VARIABLE = 2
AREA_SHIFT_TRACKING_DISABLED = 4

create_output_area:
; in:
;  esi - base address in format of VALTYPE_NUMERIC value
;  ecx = length of base address value
; out:
;  ebx - AreaHeader
;  edx - ValueDefinition
	mov	eax,[current_output_area_entry]
	test	eax,eax
	jz	create_first_output_area
	push	ecx
	lea	ecx,[eax+sizeof.OutputArea*2]
	cmp	ecx,[output_areas_list_end]
	jbe	get_next_output_entry
	mov	eax,[output_areas_list]
	sub	ecx,eax
	sub	[current_output_area_entry],eax
	sub	[initial_output_area_entry],eax
	sub	[output_areas_list_end],eax
	call	grow_stack
	mov	[output_areas_list],eax
	add	[current_output_area_entry],eax
	add	[initial_output_area_entry],eax
	add	eax,ecx
	mov	edi,eax
	xchg	[output_areas_list_end],eax
	sub	ecx,eax
	sub	edi,ecx
	shr	ecx,2
	xor	eax,eax
	rep	stosd
	mov	eax,[current_output_area_entry]
    get_next_output_entry:
	add	eax,sizeof.OutputArea
	cmp	[initial_output_area_entry],0
	je	another_initial_output_area
	mov	edi,[eax-sizeof.OutputArea+OutputArea.definition]
	mov	ecx,[edi+ValueDefinition.value_length]
	mov	edi,[edi+ValueDefinition.value]
	mov	ebx,[edi+AreaHeader.uninitialized_length]
	sub	ecx,sizeof.AreaHeader
	sub	ecx,[edi+AreaHeader.base_address_length]
	jz	prior_uninitialized_length_ok
	and	dword [prior_uninitialized_length],0
	and	dword [prior_uninitialized_length+4],0
    prior_uninitialized_length_ok:
	add	dword [prior_uninitialized_length],ebx
	adc	dword [prior_uninitialized_length+4],0
	xor	edx,edx
	add	ecx,ebx
	adc	edx,0
	add	ecx,dword [eax-sizeof.OutputArea+OutputArea.cached_offset]
	adc	edx,dword [eax-sizeof.OutputArea+OutputArea.cached_offset+4]
	mov	dword [eax+OutputArea.cached_offset],ecx
	mov	dword [eax+OutputArea.cached_offset+4],edx
	pop	ecx
	jmp	new_output_entry_ready
    another_initial_output_area:
	pop	ecx
	jmp	create_initial_output_area
    create_first_output_area:
	mov	eax,[output_areas_list]
    create_initial_output_area:
	mov	[initial_output_area_entry],eax
	and	dword [eax+OutputArea.cached_offset],0
	and	dword [eax+OutputArea.cached_offset+4],0
	and	dword [prior_uninitialized_length],0
	and	dword [prior_uninitialized_length+4],0
    new_output_entry_ready:
	mov	[current_output_area_entry],eax
	lea	ebx,[eax+OutputArea.definition]
	call	create_area
	retn

create_area:
; in:
;  ebx - where pointer to ValueDefinition should be stored, may already hold a previously used one (should contain null otherwise)
;  esi - base address in format of VALTYPE_NUMERIC value
;  ecx = length of base address value
; out:
;  ebx - AreaHeader
;  edx - ValueDefinition
	mov	[address_length],ecx
	mov	edx,[ebx]
	test	edx,edx
	jz	current_area_definition_unusable
	cmp	[edx+ValueDefinition.reference_count],1
	je	area_definition_ready
	dec	[edx+ValueDefinition.reference_count]
    current_area_definition_unusable:
	mov	ecx,retired_definition
      retrieve_retired_detached_value:
	mov	edx,[ecx]
	test	edx,edx
	jz	create_area_definition
	cmp	[edx+ValueDefinition.reference_count],0
	jne	retired_detached_value_immutable
	xor	eax,eax
	xchg	eax,[edx+ValueDefinition.previous]
	mov	[ecx],eax
	jmp	adopt_area_definition
      retired_detached_value_immutable:
	lea	ecx,[edx+ValueDefinition.previous]
	jmp	retrieve_retired_detached_value
    create_area_definition:
	mov	ecx,sizeof.ValueDefinition
	call	create_tree_element
	mov	ecx,eax
	xchg	ecx,[value_definition_chain]
	mov	[eax+ValueDefinition.interlink],ecx
	mov	edx,eax
    adopt_area_definition:
	mov	[ebx],edx
	or	[edx+ValueDefinition.flags],VAL_DETACHED
	inc	[edx+ValueDefinition.reference_count]
    area_definition_ready:
	mov	ecx,[address_length]
	add	ecx,sizeof.AreaHeader
	mov	eax,[edx+ValueDefinition.block_length]
	test	eax,eax
	jz	allocate_area_block
	cmp	ecx,eax
	jbe	initialize_area_block
	push	ecx edx
	xor	eax,eax
	xchg	eax,[edx+ValueDefinition.value]
	call	mfree
	pop	edx ecx
    allocate_area_block:
	push	edx
	call	malloc_growable
	pop	edx
	mov	[edx+ValueDefinition.value],eax
	mov	[edx+ValueDefinition.block_length],ecx
    initialize_area_block:
	mov	ebx,[edx+ValueDefinition.value]
	lea	edi,[ebx+sizeof.AreaHeader]
	mov	ecx,[address_length]
	mov	[ebx+AreaHeader.base_address_length],ecx
	rep	movsb
	mov	[ebx+AreaHeader.uninitialized_length],ecx
	mov	[ebx+AreaHeader.flags],ecx
	sub	edi,ebx
	mov	[edx+ValueDefinition.value_length],edi
	mov	[edx+ValueDefinition.type],VALTYPE_AREA
	mov	ecx,[current_pass]
	mov	[edx+ValueDefinition.pass],ecx
	retn

initialize_output:
; in: ecx = number of bytes that should be added to output
; out: edi - output buffer to be filled with data
; preserves: esi
	mov	edx,[current_area]
	mov	ebx,[edx+ValueDefinition.value]
	add	ecx,[ebx+AreaHeader.uninitialized_length]
	jc	output_out_of_memory
	mov	eax,[edx+ValueDefinition.value_length]
	lea	edi,[ebx+eax]
	add	ecx,eax
	jc	output_out_of_memory
	mov	[edx+ValueDefinition.value_length],ecx
	cmp	ecx,[edx+ValueDefinition.block_length]
	jbe	area_reserve_sufficient
	mov	eax,[edx+ValueDefinition.value]
	sub	edi,eax
	push	edx
	push	ecx
	bsr	edx,ecx
	xchg	ecx,edx
	dec	cl
	shr	edx,cl
	inc	edx
	shl	edx,cl
	pop	ecx
	cmp	edx,ecx
	jbe	output_out_of_memory
	xchg	ecx,edx
	call	realloc
	pop	edx
	mov	ebx,eax
	add	edi,eax
	mov	[edx+ValueDefinition.value],ebx
	mov	[edx+ValueDefinition.block_length],ecx
    area_reserve_sufficient:
	mov	ecx,[ebx+AreaHeader.uninitialized_length]
	jecxz	output_buffer_ready
	xor	eax,eax
	mov	dl,cl
	shr	ecx,2
	rep	stosd
	mov	cl,dl
	and	cl,11b
	rep	stosb
	mov	[ebx+AreaHeader.uninitialized_length],eax
    output_buffer_ready:
	retn
    output_out_of_memory:
	jmp	out_of_memory

uninitialized_output:
; in: ecx = number of uninitialized bytes to be added to output
; preserves: ebx, ecx, esi, edi
	mov	edx,[current_area]
	mov	eax,[edx+ValueDefinition.value]
	add	[eax+AreaHeader.uninitialized_length],ecx
	jc	area_overflow
	mov	edx,[edx+ValueDefinition.value_length]
	sub	edx,sizeof.AreaHeader
	sub	edx,[eax+AreaHeader.base_address_length]
	add	edx,[eax+AreaHeader.uninitialized_length]
	jc	area_overflow
	retn
    area_overflow:
	mov	edx,_area_overflow
	call	register_error
	mov	edx,[current_area]
	or	ecx,-1
	mov	eax,[edx+ValueDefinition.value]
	sub	ecx,[edx+ValueDefinition.value_length]
	add	ecx,sizeof.AreaHeader
	add	ecx,[eax+AreaHeader.base_address_length]
	mov	[eax+AreaHeader.uninitialized_length],ecx
	retn

trim_output:
; preserves: ecx, esi
	xor	eax,eax
	mov	dword [prior_uninitialized_length],eax
	mov	dword [prior_uninitialized_length+4],eax
	mov	edi,[current_output_area_entry]
    trim_current_output_area:
	mov	edx,[edi+OutputArea.definition]
	mov	eax,[edx+ValueDefinition.value]
	and	[eax+AreaHeader.uninitialized_length],0
	mov	ebx,[edx+ValueDefinition.value_length]
	sub	ebx,sizeof.AreaHeader
	sub	ebx,[eax+AreaHeader.base_address_length]
	jnz	output_trimmed
	cmp	edi,[initial_output_area_entry]
	je	output_trimmed
	sub	edi,sizeof.OutputArea
	jmp	trim_current_output_area
    output_trimmed:
	mov	[current_output_area_entry],edi
	retn

find_output_area:
; in:
;  [file_offset] = offset within the output
; out:
;  cf set when not found an area that would contain a byte at specified offset
;  when cf = 0:
;   ebx - OutputArea
;   [file_offset] = offset relative to the beginning of the found area (upper 32 bits are zero)
	mov	esi,[initial_output_area_entry]
	mov	edi,[current_output_area_entry]
	add	edi,sizeof.OutputArea
    search_areas:
	mov	ebx,edi
	sub	ebx,esi
	jz	output_area_not_found
	test	ebx,1 shl bsf sizeof.OutputArea
	jz	bisect_areas_list
	sub	ebx,sizeof.OutputArea
    bisect_areas_list:
	shr	ebx,1
	add	ebx,esi
	mov	eax,dword [file_offset]
	mov	edx,dword [file_offset+4]
	sub	eax,dword [ebx+OutputArea.cached_offset]
	sbb	edx,dword [ebx+OutputArea.cached_offset+4]
	jc	search_earlier_areas
	jnz	search_later_areas
	mov	edx,[ebx+OutputArea.definition]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	edx,[edx+ValueDefinition.value]
	sub	ecx,sizeof.AreaHeader
	sub	ecx,[edx+AreaHeader.base_address_length]
	add	ecx,[edx+AreaHeader.uninitialized_length]
	cmp	eax,ecx
	jae	search_later_areas
    output_area_found:
	mov	dword [file_offset],eax
	and	dword [file_offset+4],0
       ; clc
	retn
    output_area_not_found:
	stc
	retn
    search_later_areas:
	lea	esi,[ebx+sizeof.OutputArea]
	jmp	search_areas
    search_earlier_areas:
	mov	edi,ebx
	jmp	search_areas

read_from_output:
; in:
;  edi - buffer for read data
;  [value_length] = length of data to read
;  [file_offset] = offset within the output
; out:
;  [value_length] = number of bytes that were not in the existing output and could not be read
	push	edi
	call	find_output_area
	pop	edi
	jc	output_reading_done
    read_output_areas:
	cmp	[value_length],0
	je	output_reading_done
	mov	edx,[ebx+OutputArea.definition]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	eax,ecx
	mov	edx,[edx+ValueDefinition.value]
	sub	ecx,sizeof.AreaHeader
	sub	ecx,[edx+AreaHeader.base_address_length]
	sub	dword [file_offset],ecx
	jnc	initialized_load_done
	lea	esi,[edx+eax]
	mov	ecx,dword [file_offset]
	add	esi,ecx
	neg	ecx
	cmp	ecx,[value_length]
	jbe	initialized_load_length_ok
	mov	ecx,[value_length]
      initialized_load_length_ok:
	sub	[value_length],ecx
	rep	movsb
	mov	dword [file_offset],ecx
      initialized_load_done:
	mov	ecx,[edx+AreaHeader.uninitialized_length]
	sub	dword [file_offset],ecx
	jnc	uninitialized_load_done
	mov	ecx,dword [file_offset]
	neg	ecx
	cmp	ecx,[value_length]
	jbe	uninitialized_load_length_ok
	mov	ecx,[value_length]
      uninitialized_load_length_ok:
	sub	[value_length],ecx
	xor	al,al
	rep	stosb
	mov	dword [file_offset],ecx
      uninitialized_load_done:
	cmp	ebx,[current_output_area_entry]
	je	output_reading_done
	add	ebx,sizeof.OutputArea
	jmp	read_output_areas
    output_reading_done:
	retn

rewrite_output:
; in:
;  esi - data to write
;  [value_length] = length of data to write
;  [file_offset] = offset within the output
; out:
;  [value_length] = number of bytes that were not in the existing output and could not be rewritten
	push	esi
	call	find_output_area
	pop	esi
	jc	output_rewriting_done
    rewrite_output_areas:
	cmp	[value_length],0
	je	output_rewriting_done
	mov	edx,[ebx+OutputArea.definition]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	edx,[edx+ValueDefinition.value]
	sub	ecx,sizeof.AreaHeader
	sub	ecx,[edx+AreaHeader.base_address_length]
	mov	edi,[edx+AreaHeader.uninitialized_length]
	add	ecx,edi
	sub	dword [file_offset],ecx
	jnc	rewrite_next_area
	mov	eax,[value_length]
	xor	ecx,ecx
	add	eax,edi
	adc	ecx,ecx
	add	eax,dword [file_offset]
	jnc	rewrite_initialized_data
	cmp	eax,edi
	jbe	rewrite_uninitialized_data
	mov	eax,edi
      rewrite_uninitialized_data:
	test	eax,eax
	jz	rewrite_initialized_data
	push	ebx
	sub	[edx+AreaHeader.uninitialized_length],eax
	mov	edx,[ebx+OutputArea.definition]
	mov	ebx,eax
	call	expand_value
	call	update_output_offsets
	pop	ebx
      rewrite_initialized_data:
	mov	edx,[ebx+OutputArea.definition]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	edi,[edx+ValueDefinition.value]
	or	[edi+AreaHeader.flags],AREA_VARIABLE
	add	edi,[edi+AreaHeader.uninitialized_length]
	add	edi,ecx
	mov	ecx,dword [file_offset]
	add	edi,ecx
	neg	ecx
	cmp	ecx,[value_length]
	jbe	rewrite_length_ok
	mov	ecx,[value_length]
      rewrite_length_ok:
	sub	[value_length],ecx
	rep	movsb
	mov	dword [file_offset],ecx
      rewrite_next_area:
	cmp	ebx,[current_output_area_entry]
	je	output_rewriting_done
	add	ebx,sizeof.OutputArea
	jmp	rewrite_output_areas
    output_rewriting_done:
	retn

update_output_offsets:
; in:
;  edx - ValueDefinition of output area that had some of uninitialized data made initialized
; preserves: esi
	mov	eax,[current_output_area_entry]
	cmp	edx,[eax+OutputArea.definition]
	je	output_offsets_ok
	and	dword [prior_uninitialized_length],0
	and	dword [prior_uninitialized_length+4],0
    recount_prior_uninitialized_length:
	cmp	eax,[initial_output_area_entry]
	je	output_offsets_ok
	sub	eax,sizeof.OutputArea
	mov	edi,[eax+OutputArea.definition]
	mov	ebx,[edi+ValueDefinition.value]
	mov	ecx,[ebx+AreaHeader.uninitialized_length]
	add	dword [prior_uninitialized_length],ecx
	adc	dword [prior_uninitialized_length+4],0
	cmp	edx,edi
	je	output_offsets_ok
	mov	ecx,[edi+ValueDefinition.value_length]
	sub	ecx,sizeof.AreaHeader
	sub	ecx,[ebx+AreaHeader.base_address_length]
	jz	recount_prior_uninitialized_length
    output_offsets_ok:
	retn

get_current_address_value:
; out:
;  esi - address in format of VALTYPE_NUMERIC value
;  ecx = length of address value
; note: the returned value is placed in assembly workspace
	mov	eax,[current_area]
	mov	esi,[eax+ValueDefinition.value]
	mov	ebx,[eax+ValueDefinition.value_length]
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	mov	ecx,[esi+AreaHeader.base_address_length]
	add	ecx,4
	call	reserve_workspace
	mov	ecx,[esi+AreaHeader.base_address_length]
	sub	ebx,ecx
	sub	ebx,sizeof.AreaHeader
	add	ebx,[esi+AreaHeader.uninitialized_length]
       ; jc	 internal_error
	add	esi,sizeof.AreaHeader
	xor	eax,eax
	stosd
	lodsd
	mov	ecx,eax
	xor	edx,edx
	jecxz	offset_added_to_base_address
    add_offset_to_base_address:
	lodsb
	add	al,bl
	setc	dl
	stosb
	shr	ebx,8
	add	ebx,edx
	loop	add_offset_to_base_address
    offset_added_to_base_address:
	mov	edx,[assembly_workspace.memory_start]
	add	edx,4
	mov	eax,ebx
	cmp	byte [esi-1],80h
	cmc
	sbb	eax,0
	stosd
    optimize_base_address:
	movsx	eax,byte [edi-2]
	cmp	ah,[edi-1]
	jne	base_address_ready
	dec	edi
	cmp	edi,edx
	jne	optimize_base_address
    base_address_ready:
	mov	ecx,edi
	sub	ecx,edx
	mov	[edx-4],ecx
	mov	ecx,esi
    measure_variable_terms:
	lodsd
	test	eax,eax
	jz	variable_terms_measured
	lodsd
	add	esi,eax
	jmp	measure_variable_terms
    variable_terms_measured:
	xchg	ecx,esi
	sub	ecx,esi
	mov	al,cl
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	retn

get_output_length:
; out:
;  edx:eax = length of current output (not counting uninitialized data)
; preserves: esi
	mov	ebx,[current_output_area_entry]
	mov	eax,dword [ebx+OutputArea.cached_offset]
	mov	edx,dword [ebx+OutputArea.cached_offset+4]
	mov	edi,[ebx+OutputArea.definition]
	mov	ecx,[edi+ValueDefinition.value_length]
	mov	edi,[edi+ValueDefinition.value]
	sub	ecx,sizeof.AreaHeader
	sub	ecx,[edi+AreaHeader.base_address_length]
	jz	current_area_entirely_uninitialized
	add	eax,ecx
	adc	edx,0
	retn
    current_area_entirely_uninitialized:
	sub	eax,dword [prior_uninitialized_length]
	sbb	edx,dword [prior_uninitialized_length+4]
	retn

get_output_position:
; out:
;  edx:eax = current position in the output (including uninitialized data)
; preserves: esi
	mov	ebx,[current_output_area_entry]
	mov	eax,dword [ebx+OutputArea.cached_offset]
	mov	edx,dword [ebx+OutputArea.cached_offset+4]
	mov	edi,[ebx+OutputArea.definition]
	mov	ecx,[edi+ValueDefinition.value_length]
	mov	edi,[edi+ValueDefinition.value]
	sub	ecx,sizeof.AreaHeader
	sub	ecx,[edi+AreaHeader.base_address_length]
	add	ecx,[edi+AreaHeader.uninitialized_length]
	add	eax,ecx
	adc	edx,0
	retn

create_output_path:
; in:
;  ebx - base path and name
;  esi - file extension
;  ecx = length of the extension
; out:
;  edx - output path (generated in temporary storage)
	push	ecx
	mov	edi,ebx
	xor	al,al
	or	ecx,-1
	repne	scasb
	dec	edi
	mov	ecx,edi
    locate_extension:
	cmp	edi,ebx
	je	copy_path_name
	dec	edi
	mov	al,[edi]
	cmp	al,'\'
	je	copy_path_name
	cmp	al,'/'
	je	copy_path_name
	cmp	al,'.'
	jne	locate_extension
	mov	ecx,edi
    copy_path_name:
	sub	ecx,ebx
	push	ecx
	mov	edi,[preprocessing_workspace.memory_start]
	inc	ecx
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	pop	ecx
	xchg	esi,ebx
	rep	movsb
	mov	esi,ebx
	pop	ecx
	mov	ebx,ecx
	add	ecx,2
	call	reserve_workspace
	mov	ecx,ebx
	jecxz	extension_attached
	mov	al,'.'
	stosb
	rep	movsb
    extension_attached:
	xor	al,al
	stosb
	mov	edx,[preprocessing_workspace.memory_start]
	retn

write_output_file:
; in:
;  ebx - source path
;  edi - output path
; out:
;  cf set when write failed
; note:
;  when output path is null, source path is used with replaced or attached extension
	mov	[base_path],edi
	xor	eax,eax
	mov	[output_failures],eax
	mov	dword [uninitialized_length],eax
	mov	dword [uninitialized_length+4],eax
	mov	edx,edi
	test	edx,edx
	jnz	create_output_file
	mov	[base_path],ebx
	mov	esi,[output_extension]
	mov	ecx,[output_extension_length]
	call	create_output_path
    create_output_file:
	call	create
	jc	output_write_failed
	mov	esi,[initial_output_area_entry]
    write_area:
	mov	edx,[esi+OutputArea.definition]
	mov	eax,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	sub	ecx,[eax+AreaHeader.base_address_length]
	sub	ecx,sizeof.AreaHeader
	jz	write_next_area
	mov	eax,dword [uninitialized_length]
	or	eax,dword [uninitialized_length+4]
	jz	write_initialized_data
    write_uninitialized_data:
	mov	edi,[assembly_workspace.memory_start]
	mov	ecx,1000h shr 2
	xor	eax,eax
	rep	stosd
	mov	ecx,1000h
	cmp	dword [uninitialized_length+4],0
	jne	portion_length_ok
	cmp	ecx,dword [uninitialized_length]
	jbe	portion_length_ok
	mov	ecx,dword [uninitialized_length]
    portion_length_ok:
	sub	dword [uninitialized_length],ecx
	sbb	dword [uninitialized_length+4],0
	mov	edx,[assembly_workspace.memory_start]
	call	write
	jc	file_write_failed
	mov	eax,dword [uninitialized_length]
	or	eax,dword [uninitialized_length+4]
	jnz	write_uninitialized_data
    write_initialized_data:
	mov	edx,[esi+OutputArea.definition]
	mov	eax,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	edx,[eax+AreaHeader.base_address_length]
	add	edx,sizeof.AreaHeader
	sub	ecx,edx
	add	edx,eax
	call	write
	jc	file_write_failed
    write_next_area:
	mov	edx,[esi+OutputArea.definition]
	mov	eax,[edx+ValueDefinition.value]
	mov	eax,[eax+AreaHeader.uninitialized_length]
	add	dword [uninitialized_length],eax
	adc	dword [uninitialized_length+4],0
	cmp	esi,[current_output_area_entry]
	je	close_output_file
	add	esi,sizeof.OutputArea
	jmp	write_area
    close_output_file:
	call	close
	mov	ebx,[auxiliary_output_areas]
	mov	edi,write_auxiliary_output_area
	call	iterate_through_map
	cmp	[output_failures],0
	jne	output_write_failed
	clc
	retn
    file_write_failed:
	call	close
    output_write_failed:
	stc
	retn

write_auxiliary_output_area:
; in:
;  eax = ValueDefinition, null for cached extension not used for auxiliary output
;  esi - file extension
;  ecx = length of the extension
	test	eax,eax
	jz	auxiliary_output_processed
	mov	ebx,[base_path]
	test	ebx,ebx
	jz	auxiliary_output_processed
	push	eax
	call	create_output_path
	call	create
	pop	edx
	jc	auxiliary_file_creation_failed
	mov	eax,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	edx,[eax+AreaHeader.base_address_length]
	add	edx,sizeof.AreaHeader
	sub	ecx,edx
	add	edx,eax
	call	write
	jc	auxiliary_file_write_failed
	call	close
    auxiliary_output_processed:
	retn
    auxiliary_file_write_failed:
	call	close
    auxiliary_file_creation_failed:
	inc	[output_failures]
	retn
