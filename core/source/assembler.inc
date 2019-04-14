
; general note:
;  with the ability to run in various environments in mind, the entire assembler core
;  does not use ebp register and never directly touches esp; the stack is managed
;  with push and pop instructions, occasionally [esp] may be used to access the top value,
;  but no other assumptions about the stack layout are made

struct Workspace
	memory_start dd ?
	memory_end dd ?
ends

struct SourceContext
	number_of_entries dd ?
      ; entries SourceEntry[]
ends

struct SourceEntry
	type db ?			 ; SOURCE_#
	flags db ?
	reserved dw ?
	name dd ?
	name_length dd ?
	text dd ?
	offset dd ?
	line_number dd ?
	number_of_attached_lines dd ?
	line_offset dd ?
	local_namespace dd ?
ends

struct RecognitionContext
	base_namespace dd ?
	current_label dd ?
ends

struct LineEmbedding
	previous_pointer dd ?
	previous_end dd ?
	recognition_context dd ?
	definition dd ?
	whitespace dd ?
ends

struct LineExcerpt
	data_start dd ?
	data_end dd ?
	recognition_context dd ?
	leftover_context dd ?
ends

SOURCE_FILE = 0
SOURCE_MEMORY = 1
SOURCE_MACRO = 2

PMODE_RETAIN_COMMENTS = 1
PMODE_ISOLATE_LINES = 2

AMODE_SKIP = 1
AMODE_DEFINITION = 2
AMODE_POSTPONED = 4

TRACE_ERROR_STACK = 1
TRACE_DISPLAY = 2

assembly_init:
; in:
;  al = any combination of TRACE_# flags

	mov	[trace_mode],al

	xor	eax,eax
	mov	[current_pass],eax
	mov	[first_error],eax

	mov	[file_source_cache],eax
	mov	[memory_source_cache],eax
	mov	[file_data_cache],eax

	mov	[tree_stack_base],eax
	mov	[source_context],eax
	mov	[line_embeddings],eax
	mov	[directives_stack_base],eax
	mov	[counters_stack_base],eax
	mov	[operator_stack_base],eax
	mov	[condition_stack_base],eax
	mov	[assembly_stack_base],eax

	mov	[preprocessing_workspace.memory_start],eax
	mov	[assembly_workspace.memory_start],eax
	mov	[identifier_workspace.memory_start],eax
	mov	[value_workspace.memory_start],eax
	mov	[expression_workspace.memory_start],eax
	mov	[calculation_workspace.memory_start],eax

	mov	[display_buffer],eax
	mov	[macro_buffer],eax
	mov	[temporary_terms],eax
	mov	[temporary_floats],eax
	mov	[operator_table],eax

	mov	edi,characters
	xor	eax,eax
      prepare_characters:
	stosb
	inc	al
	jnz	prepare_characters
	mov	esi,characters+'a'
	mov	edi,characters+'A'
	mov	ecx,'z'+1-'a'
	rep	movsb
	mov	edi,characters
	mov	esi,control_characters
	mov	cl,control_characters.count
      mark_control_characters:
	lodsb
	mov	byte [edi+eax],20h
	loop	mark_control_characters
	mov	esi,syntactical_characters
	mov	cl,syntactical_characters.count
      mark_syntactical_characters:
	lodsb
	mov	byte [edi+eax],0
	loop	mark_syntactical_characters

	mov	esi,include_variable
	xor	ecx,ecx
	call	get_environment_variable
	mov	ecx,eax
	call	malloc_fixed
	mov	[include_paths],eax
	mov	edi,eax
	call	get_environment_variable

	xor	eax,eax
	mov	[storage_blocks],eax
	mov	[storage_free_space_length],eax
	mov	[tree_blocks],eax
	mov	[tree_reserve_length],eax
	mov	[value_definition_chain],eax
	mov	[retired_definition],eax

	mov	cl,10
	call	create_string_map
	mov	[file_source_cache],ebx

	mov	cl,12
	call	create_string_map
	mov	[memory_source_cache],ebx

	mov	cl,10
	call	create_string_map
	mov	[file_data_cache],ebx

	mov	cl,7
	call	create_string_map
	mov	[auxiliary_output_areas],ebx

	mov	ecx,sizeof.SymbolTree_Root + sizeof.SymbolTree_Node
	call	create_tree_element
	mov	[root_namespace],eax

	call	create_parameter_namespace
	mov	[root_parameter_namespace],eax

	mov	ecx,sizeof.SymbolTree_Leaf
	call	create_tree_element
	mov	[eax+SymbolTree_Leaf.class],SYMCLASS_INSTRUCTION
	or	[eax+SymbolTree_Leaf.flags],SYM_VARIABLE
	mov	[interceptor_symbol],eax
	mov	ecx,sizeof.SymbolTree_Leaf
	call	create_tree_element
	mov	[eax+SymbolTree_Leaf.class],SYMCLASS_STRUCTURE
	or	[eax+SymbolTree_Leaf.flags],SYM_VARIABLE
	mov	[label_interceptor_symbol],eax

	mov	ecx,100h
	call	malloc_growable
	mov	[tree_stack_base],eax
	add	eax,ecx
	mov	[tree_stack_end],eax

	mov	ecx,1000h
	call	malloc_growable
	mov	[source_context],eax
	mov	[source_context_maximum_length],ecx

	mov	ecx,1000h
	call	malloc_growable
	mov	[line_embeddings],eax
	mov	[line_embeddings_maximum_length],ecx

	xor	eax,eax
	mov	[number_of_line_embeddings],eax
	mov	[first_output_area_entry],eax

	mov	ecx,sizeof.AreaEntry
	call	create_tree_element
	mov	[virtual_area_entry],eax

	mov	ecx,1000h
	call	malloc_growable
	mov	[directives_stack_base],eax
	add	eax,ecx
	mov	[directives_stack_end],eax

	mov	ecx,1000h
	call	malloc_growable
	mov	[counters_stack_base],eax
	add	eax,ecx
	mov	[counters_stack_end],eax

	mov	ecx,100h
	call	malloc_growable
	mov	[operator_stack_base],eax
	add	eax,ecx
	mov	[operator_stack_end],eax

	mov	ecx,10h
	call	malloc_growable
	mov	[condition_stack_base],eax
	add	eax,ecx
	mov	[condition_stack_end],eax

	mov	ecx,100h
	call	malloc_growable
	mov	[assembly_stack_base],eax
	add	eax,ecx
	mov	[assembly_stack_end],eax

	mov	edx,preprocessing_workspace
	mov	ecx,1000h
	call	allocate_workspace

	mov	edx,assembly_workspace
	mov	ecx,1000h
	call	allocate_workspace

	mov	edx,identifier_workspace
	mov	ecx,100h
	call	allocate_workspace

	mov	edx,value_workspace
	mov	ecx,1000h
	call	allocate_workspace

	mov	edx,expression_workspace
	mov	ecx,1000h
	call	allocate_workspace

	mov	edx,calculation_workspace
	mov	ecx,1000h
	call	allocate_workspace

	mov	ecx,100h
	call	malloc_growable
	mov	[display_buffer],eax
	mov	[display_buffer_length],ecx

	mov	ecx,100h
	call	malloc_growable
	mov	[macro_buffer],eax
	mov	[macro_buffer_length],ecx

	mov	ecx,32*6*sizeof.ExpressionTerm
	call	malloc_fixed
	mov	[temporary_terms],eax
	mov	[free_temporary_terms],eax

	mov	ecx,4*sizeof.FloatData
	call	malloc_fixed
	mov	[temporary_floats],eax

	mov	ecx,256*4
	call	malloc_fixed
	mov	[operator_table],eax
	mov	edi,eax
	mov	ecx,256
	xor	eax,eax
	rep	stosd
	mov	esi,separating_operators
    register_operators:
	lodsb
	test	al,al
	jz	operators_registered
	movzx	ebx,al
	shl	ebx,2
	add	ebx,[operator_table]
	mov	ecx,sizeof.ValueDefinition
	call	create_tree_element
	mov	edx,eax
	xchg	[ebx],eax
	mov	[edx+ValueDefinition.previous],eax
	inc	[edx+ValueDefinition.reference_count]
	lodsb
	mov	[edx+ValueDefinition.type],al
	lodsb
	mov	[edx+ValueDefinition.flags],al
	lodsb
	mov	[edx+ValueDefinition.attribute],al
	lodsd
	mov	[edx+ValueDefinition.value],eax
	jmp	register_operators
    operators_registered:

	xor	eax,eax
	mov	[name_volatile],al
	mov	[name_token],eax
	mov	[name_kind],NAME_CASEINSENSITIVE
	or	[symbol_required],1
	or	[symbol_expected],1
	mov	eax,[root_namespace]
	mov	[current_namespace],eax
	mov	esi,symbols
    register_internal_symbols:
	lodsb
	test	al,al
	jnz	prepare_internal_symbol
	lodsb
	test	al,al
	jz	internal_symbols_registered
	and	[current_namespace],0
    prepare_internal_symbol:
	movzx	ecx,al

	xor	ebx,ebx
	mov	edx,FNV_OFFSET
    hash_internal_symbol:
	movzx	eax,byte [esi+ebx]
	xor	dl,[characters+eax]
	imul	edx,FNV_PRIME
	inc	ebx
	cmp	ebx,ecx
	jb	hash_internal_symbol

	mov	ebx,[current_namespace]
	test	ebx,ebx
	jz	register_internal_namespace
	mov	al,[esi+ecx]
	mov	[symbol_class],al
	push	esi
	call	scan_namespace
	pop	esi
	add	esi,ecx
	mov	ecx,sizeof.ValueDefinition
	call	create_tree_element
	mov	edx,eax
	xchg	[ebx+SymbolTree_Leaf.definition],eax
	mov	[edx+ValueDefinition.previous],eax
	inc	[edx+ValueDefinition.reference_count]
	lodsb
	lodsb
	mov	[edx+ValueDefinition.type],al
	lodsb
	mov	[edx+ValueDefinition.flags],al
	lodsb
	mov	[edx+ValueDefinition.attribute],al
	lodsd
	mov	[edx+ValueDefinition.value],eax
	jmp	register_internal_symbols
    register_internal_namespace:
	mov	[symbol_class],SYMCLASS_EXPRESSION
	mov	ebx,[root_namespace]
	push	esi
	call	scan_namespace
	pop	esi
	add	esi,ecx
	mov	ecx,sizeof.ValueDefinition
	call	create_tree_element
	mov	[ebx+SymbolTree_Leaf.definition],eax
	inc	[eax+ValueDefinition.reference_count]
	mov	[eax+ValueDefinition.type],VALTYPE_RESERVED
	mov	[eax+ValueDefinition.flags],VAL_INTERNAL
	call	get_symbol_namespace
	mov	[current_namespace],ebx
	jmp	register_internal_symbols
    internal_symbols_registered:

	retn

assembly_shutdown:

	call	discard_errors

	mov	ebx,[auxiliary_output_areas]
	test	ebx,ebx
	jz	auxiliary_output_areas_released
	call	destroy_string_map
    auxiliary_output_areas_released:

	mov	ebx,[value_definition_chain]
    release_values_from_chain:
	test	ebx,ebx
	jz	values_released
	cmp	[ebx+ValueDefinition.block_length],0
	je	release_next_value
	mov	eax,[ebx+ValueDefinition.value]
	call	mfree
    release_next_value:
	mov	ebx,[ebx+ValueDefinition.interlink]
	jmp	release_values_from_chain
    values_released:

	mov	eax,[include_paths]
	call	mfree

	mov	ebx,[file_source_cache]
	test	ebx,ebx
	jz	file_source_cache_released
	mov	edi,mfree
	call	iterate_through_map
	mov	ebx,[file_source_cache]
	call	destroy_string_map
    file_source_cache_released:

	mov	ebx,[file_data_cache]
	test	ebx,ebx
	jz	file_data_cache_released
	mov	edi,release_file_data
	call	iterate_through_map
	mov	ebx,[file_data_cache]
	call	destroy_string_map
    file_data_cache_released:

	mov	ebx,[memory_source_cache]
	test	ebx,ebx
	jz	memory_source_cache_released
	mov	edi,mfree
	call	iterate_through_map
	mov	ebx,[memory_source_cache]
	call	destroy_string_map
    memory_source_cache_released:

	mov	eax,[tree_stack_base]
	call	mfree
	mov	eax,[source_context]
	call	mfree
	mov	eax,[line_embeddings]
	call	mfree
	mov	eax,[directives_stack_base]
	call	mfree
	mov	eax,[counters_stack_base]
	call	mfree
	mov	eax,[operator_stack_base]
	call	mfree
	mov	eax,[condition_stack_base]
	call	mfree
	mov	eax,[assembly_stack_base]
	call	mfree

	mov	eax,[preprocessing_workspace.memory_start]
	call	mfree
	mov	eax,[assembly_workspace.memory_start]
	call	mfree
	mov	eax,[identifier_workspace.memory_start]
	call	mfree
	mov	eax,[value_workspace.memory_start]
	call	mfree
	mov	eax,[expression_workspace.memory_start]
	call	mfree
	mov	eax,[calculation_workspace.memory_start]
	call	mfree

	mov	eax,[display_buffer]
	call	mfree
	mov	eax,[macro_buffer]
	call	mfree
	mov	eax,[temporary_terms]
	call	mfree
	mov	eax,[temporary_floats]
	call	mfree
	mov	eax,[operator_table]
	call	mfree

	mov	eax,[tree_blocks]
	call	release_chained_blocks

	mov	eax,[storage_blocks]
	call	release_chained_blocks

	retn

    release_chained_blocks:
	test	eax,eax
	jz	blocks_released
	mov	ebx,[eax]
	call	mfree
	mov	eax,ebx
	jmp	release_chained_blocks
      blocks_released:
	retn

    release_file_data:
	test	eax,eax
	jz	file_data_released
	mov	ebx,[eax+FileData.cache]
	call	mfree
      release_file_cache:
	mov	eax,ebx
	test	eax,eax
	jz	file_data_released
	mov	ebx,[ebx+FileCache.next]
	call	mfree
	jmp	release_file_cache
      file_data_released:
	retn

    release_auxiliary_output:
	test	eax,eax
	jz	auxiliary_output_released
	dec	[eax+ValueDefinition.reference_count]
	xor	eax,eax
	mov	[edx+MapEntry.value],eax
      auxiliary_output_released:
	retn

assembly_pass:
; in:
;  esi - ASCIIZ string containing source text
;  edx - path to source file
; out:
;  cf clear if another pass is needed
; note:
;  if both string and file sources are present, they are assembled as combined text
	mov	[source_file],edx
	inc	[current_pass]
	call	discard_errors
	mov	eax,[directives_stack_base]
	mov	[directives_stack],eax
	mov	eax,[root_namespace]
	mov	[current_namespace],eax
	mov	edx,[counters_stack_base]
	mov	[current_counter],edx
	xor	eax,eax
	mov	[edx],al
	mov	[preprocessing_mode],al
	mov	[next_pass_needed],al
	mov	[assembly_mode],al
	mov	[shift_tracking],al
	mov	[current_area],eax
	mov	[current_output_area_entry],eax
	mov	[initial_output_area_entry],eax
	mov	[predicted_shift],eax
	mov	[display_data_length],eax
	mov	[macro_position],eax
	mov	[macro_end_position],eax
	mov	[proxy_number],eax
	mov	[output_extension],eax
	mov	[output_extension_length],eax
	mov	ebx,[source_context]
	mov	[ebx+SourceContext.number_of_entries],eax
	add	ebx,sizeof.SourceContext
	mov	edi,ebx
	mov	ecx,sizeof.SourceEntry shr 2
	assert	sizeof.SourceEntry and 11b = 0
	rep	stosd
	mov	[line_start],eax
	mov	[line_end],eax
	test	esi,esi
	jz	read_main_source_file
	cmp	byte [esi],0
	je	read_main_source_file
	push	ebx
	call	use_source
	pop	ebx
	mov	[ebx+SourceEntry.type],SOURCE_MEMORY
	jmp	fill_main_source_entry
      read_main_source_file:
	mov	esi,[source_file]
	test	esi,esi
	jz	no_source_to_assemble
	cmp	byte [esi],0
	je	no_source_to_assemble
	push	ebx
	call	read_source
	pop	ebx
	test	eax,eax
	jz	main_source_file_not_found
	mov	[ebx+SourceEntry.type],SOURCE_FILE
      fill_main_source_entry:
	mov	[ebx+SourceEntry.name],esi
	mov	[ebx+SourceEntry.text],eax
	mov	eax,[root_parameter_namespace]
	mov	[local_parameter_namespace],eax
	mov	[ebx+SourceEntry.local_namespace],eax
	mov	ebx,[source_context]
	inc	[ebx+SourceContext.number_of_entries]
	mov	esi,zero_value
	mov	ecx,4+4
	call	create_output_area
	mov	[current_area],edx
	inc	[edx+ValueDefinition.reference_count]
	mov	ebx,[auxiliary_output_areas]
	mov	edi,release_auxiliary_output
	call	iterate_through_map
    assembly_line:
	xor	eax,eax
	mov	[value_position],eax
	mov	[current_constituent],al
	call	clear_line_embeddings
	call	get_line
	jc	pass_done
    assemble_instruction:
	mov	ebx,[interceptor_symbol]
	call	get_available_value
	jc	no_interceptor
	test	[edx+ValueDefinition.flags],VAL_UNCONDITIONAL
	jz	weak_interceptor
	jmp	use_macro
      no_interceptor:
	xor	edx,edx
      weak_interceptor:
	mov	[interceptor],edx
	xor	eax,eax
	mov	[label_interceptor],eax
	mov	[symbol_definition],al
	mov	[instruction_branch],eax
	mov	dl,SYMCLASS_INSTRUCTION
	call	identify_symbol
	jc	empty_line
	mov	[label_branch],edx
	mov	al,[symbol_solid]
	mov	[label_solid],al
	test	ebx,ebx
	jz	unrecognized_instruction
	assert	SYMCLASS_EXPRESSION < SYMCLASS_INSTRUCTION
	assert	SYMCLASS_STRUCTURE > SYMCLASS_INSTRUCTION
	cmp	[ebx+SymbolTree_Leaf.class],SYMCLASS_INSTRUCTION
	jb	labeled_instruction
	ja	unrecognized_instruction
	call	get_available_value
	jc	unrecognized_instruction
	test	[edx+ValueDefinition.flags],VAL_UNCONDITIONAL
	jnz	execute_instruction
	cmp	[interceptor],0
	jne	unrecognized_instruction
	test	[assembly_mode],AMODE_SKIP or AMODE_DEFINITION
	jnz	unrecognized_instruction
    execute_instruction:
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	je	use_macro
	cmp	al,VALTYPE_RESERVED
	je	macro_self_reference
	cmp	al,VALTYPE_NATIVE_COMMAND
	jne	unrecognized_instruction
	jmp	[edx+ValueDefinition.value]
    macro_self_reference:
	mov	edx,_symbolic_self_reference
	call	register_error
    unrecognized_instruction:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
    illegal_instruction:
	cmp	[interceptor],0
	jne	execute_interceptor
	cmp	[label_interceptor],0
	jne	execute_label_interceptor
	mov	edx,_illegal_instruction
	call	register_error
	mov	ebx,[label_branch]
	test	ebx,ebx
	jz	assembly_line
	mov	[symbol_class],SYMCLASS_INSTRUCTION
	or	[symbol_required],1
	or	[symbol_expected],1
	call	scan_symbol_branch
	xor	edx,edx
	call	mark_symbol_as_used
	mov	ebx,[instruction_branch]
	test	ebx,ebx
	jz	assembly_line
	mov	[symbol_class],SYMCLASS_STRUCTURE
	or	[symbol_required],1
	or	[symbol_expected],1
	call	scan_symbol_branch
	xor	edx,edx
	call	mark_symbol_as_used
	jmp	assembly_line
    labeled_instruction:
	cmp	[interceptor],0
	jne	unrecognized_instruction
	mov	[label_leaf],ebx
	mov	[label_instruction_start],esi
	mov	ebx,[label_interceptor_symbol]
	call	get_available_value
	jc	no_label_interceptor
	test	[edx+ValueDefinition.flags],VAL_UNCONDITIONAL
	jnz	use_struc
	mov	[label_interceptor],edx
	mov	eax,[embedded_context]
	mov	[label_instruction_context],eax
	jmp	identify_structure_instruction
    no_label_interceptor:
	call	move_to_next_symbol
	jc	unrecognized_instruction	; orphan label
	cmp	al,':'
	je	define_label
	cmp	al,'='
	je	define_numeric_symbol
    identify_structure_instruction:
	mov	dl,SYMCLASS_STRUCTURE
	call	identify_symbol
	mov	[instruction_branch],edx
	test	ebx,ebx
	jz	unrecognized_instruction
	cmp	[ebx+SymbolTree_Leaf.class],SYMCLASS_STRUCTURE
	jne	unrecognized_instruction
	call	get_available_value
	jc	unrecognized_instruction
	test	[edx+ValueDefinition.flags],VAL_UNCONDITIONAL
	jnz	execute_labeled_instruction
	test	[assembly_mode],AMODE_SKIP or AMODE_DEFINITION
	jnz	unrecognized_instruction
	cmp	[label_interceptor],0
	jne	unrecognized_instruction
    execute_labeled_instruction:
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	je	use_struc
	cmp	al,VALTYPE_NATIVE_COMMAND
	jne	unrecognized_instruction
	jmp	[edx+ValueDefinition.value]
    empty_line:
	mov	al,[assembly_mode]
	and	al,AMODE_SKIP or AMODE_DEFINITION
	cmp	al,AMODE_DEFINITION
	je	add_line_to_macro
	jmp	assembly_line
    execute_interceptor:
	mov	ebx,[interceptor_symbol]
	mov	edx,[interceptor]
	xor	eax,eax
	mov	[embedded_context],eax
	mov	esi,[line_start]
	jmp	use_macro
    execute_label_interceptor:
	mov	ebx,[label_interceptor_symbol]
	mov	edx,[label_interceptor]
	mov	eax,[label_instruction_context]
	mov	[embedded_context],eax
	mov	esi,[label_instruction_start]
	jmp	use_struc
    instruction_assembled:
	cmp	[current_constituent],0
	jne	extra_characters_on_line
	call	warp_to_next_symbol
	jc	assembly_line
    extra_characters_on_line:
	mov	edx,_extra_characters_on_line
	call	register_error
	jmp	assembly_line
    pass_done:
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	assemble_postponed_block
	mov	esi,edi
	sub	esi,[edi+DirectiveBlock.length_of_data]
	mov	edx,_missing_end_directive
	call	register_delayed_error
	call	close_control_directive_block
	jmp	pass_done
    assemble_postponed_block:
	mov	dl,DBLOCK_POSTPONED
	call	find_directive_block
	jc	no_postponed_blocks
	mov	ebx,edi
	mov	esi,edi
	sub	esi,[edi+DirectiveBlock.length_of_data]
	mov	edi,[source_context]
	call	clone_source_context
	mov	edi,ebx
	call	close_directive_block
	or	[assembly_mode],AMODE_POSTPONED
	jmp	assembly_line
    no_postponed_blocks:
	mov	ebx,[root_namespace]
	call	detect_mispredictions
    assemble_suspended_block:
	mov	dl,DBLOCK_SUSPENDED
	call	find_directive_block
	jc	no_suspended_blocks
	cmp	[next_pass_needed],0
	jne	ignore_suspended_block
	mov	ebx,edi
	mov	esi,edi
	sub	esi,[edi+DirectiveBlock.length_of_data]
	mov	edi,[source_context]
	call	clone_source_context
	mov	edi,ebx
	call	close_directive_block
	or	[assembly_mode],AMODE_POSTPONED
	jmp	assembly_line
    ignore_suspended_block:
	call	close_directive_block
	jmp	assemble_suspended_block
    no_suspended_blocks:
	mov	esi,[directives_stack]
    signal_unclosed_blocks:
	cmp	esi,[directives_stack_base]
	je	unclosed_blocks_signalled
	sub	esi,sizeof.DirectiveBlock
	mov	eax,[esi+DirectiveBlock.length_of_data]
	sub	esi,eax
	mov	edx,_missing_end_directive
	call	register_delayed_error
	jmp	signal_unclosed_blocks
    unclosed_blocks_signalled:
	xor	eax,eax
	xchg	eax,[current_area]
	dec	[eax+ValueDefinition.reference_count]
	mov	al,[next_pass_needed]
	sub	al,1
	retn
    main_source_file_not_found:
	mov	edx,_source_file_not_found
	call	register_error
    no_source_to_assemble:
	mov	esi,zero_value
	mov	ecx,4+4
	call	create_output_area
	mov	[current_area],edx
	inc	[edx+ValueDefinition.reference_count]
	stc
	retn

allocate_workspace:
; in:
;  edx - Workspace
;  ecx = size of initial reserve
; preserves: ebx, edx, esi, edi
	push	edx
	call	malloc_growable
	pop	edx
	mov	[edx+Workspace.memory_start],eax
	add	eax,ecx
	mov	[edx+Workspace.memory_end],eax
	retn

reserve_workspace:
; in:
;  edx - Workspace
;  edi - top of used workspace area
;  ecx = size of required reserve
; out:
;  cf set if workspace had to be expanded
;  edi - top of used workspace area (possibly relocated when cf is set)
; preserves: ebx, edx, esi
	mov	eax,[edx+Workspace.memory_end]
	sub	eax,ecx
	jc	not_enough_workspace
	cmp	edi,eax
	ja	not_enough_workspace
	clc
	retn
    not_enough_workspace:
	add	ecx,edi
	jc	out_of_memory
	sub	ecx,[edx+Workspace.memory_start]
	bsr	eax,ecx
	xchg	ecx,eax
	dec	cl
	shr	eax,cl
	inc	eax
	shl	eax,cl
	mov	ecx,eax
	cmp	edi,[edx+Workspace.memory_start]
	je	reestablish_workspace
    expand_workspace:
	mov	eax,[edx+Workspace.memory_start]
	sub	edi,eax
	push	edx
	call	realloc
	pop	edx
	mov	[edx+Workspace.memory_start],eax
	add	edi,eax
	add	eax,ecx
	mov	[edx+Workspace.memory_end],eax
	stc
	retn
    reestablish_workspace:
	push	edx ecx
	xor	eax,eax
	xchg	eax,[edx+Workspace.memory_start]
	call	mfree
	pop	ecx
	call	malloc_growable
	pop	edx
	mov	[edx+Workspace.memory_start],eax
	mov	edi,eax
	add	eax,ecx
	mov	[edx+Workspace.memory_end],eax
	stc
	retn

grow_stack:
; in:
;  eax = base address of memory block containing stack data
;  ecx = required minimum size of memory block
; out:
;  eax = new base address of memory block containing stack data
;  ecx = new size of memory block
; preserves: ebx, esi, edi
	bsr	edx,ecx
	xchg	ecx,edx
	sub	cl,2
	shr	edx,cl
	inc	edx
	shl	edx,cl
	mov	ecx,edx
	call	realloc
	retn

create_source_entry:
; out:
;  cf set when the maximum number of entries in context has been exceeded
;  when cf = 0:
;   ebx - new SourceEntry in the main SourceContext
; preserves: edx, esi
	mov	ebx,[source_context]
	mov	eax,[ebx+SourceContext.number_of_entries]
	cmp	eax,[maximum_depth_of_stack]
	jae	source_context_full
	inc	[ebx+SourceContext.number_of_entries]
	imul	eax,sizeof.SourceEntry
	add	eax,sizeof.SourceContext
	mov	edi,eax
	add	eax,sizeof.SourceEntry
	cmp	eax,[source_context_maximum_length]
	jbe	source_context_length_ok
	mov	ecx,eax
	mov	eax,ebx
	mov	ebx,edx
	call	realloc
	mov	[source_context],eax
	mov	[source_context_maximum_length],ecx
	mov	edx,ebx
	mov	ebx,eax
    source_context_length_ok:
	add	ebx,edi
	mov	edi,ebx
	mov	ecx,sizeof.SourceEntry shr 2
	assert	sizeof.SourceEntry and 11b = 0
	xor	eax,eax
	rep	stosd
	clc
	retn
    source_context_full:
	stc
	retn

create_parameter_namespace:
; out:
;  eax - SymbolTree_Root
; preserves: ebx, edx, esi, edi
	mov	ecx,sizeof.SymbolTree_Root + sizeof.SymbolTree_LocalNode
	call	create_tree_element
	or	[eax+SymbolTree_Root.attributes],SYMTREE_LOCAL
	or	[eax+SymbolTree_Root.flags],NAMESPACE_UNATTACHED
	retn

clone_source_context:
; in:
;  esi - SourceContext
;  edi - buffer
; out:
;  edi = pointer advanced past the stored data
; preserves: ebx
	cmp	edi,[source_context]
	sete	[source_context_affected]
	mov	eax,[esi+SourceContext.number_of_entries]
	assert	sizeof.SourceContext and 11b = 0
	mov	ecx,sizeof.SourceContext shr 2
	rep	movsd
	test	eax,eax
	jnz	clone_source_entry
	retn
    clone_source_entry:
	cmp	[esi+SourceEntry.type],SOURCE_MACRO
	jne	copy_source_entry
	mov	edx,[esi+SourceEntry.text]
	inc	[edx+ValueDefinition.reference_count]
	cmp	[source_context_affected],0
	je	copy_source_entry
	or	[edx+ValueDefinition.flags],VAL_IN_USE
      copy_source_entry:
	assert	sizeof.SourceEntry and 11b = 0
	mov	ecx,sizeof.SourceEntry shr 2
	rep	movsd
	dec	eax
	jnz	clone_source_entry
	retn

release_source_context:
; in:
;  eax - SourceContext
; preserves: eax, ebx, esi, edi
	cmp	eax,[source_context]
	sete	[source_context_affected]
	push	eax
	mov	ecx,[eax+SourceContext.number_of_entries]
	add	eax,sizeof.SourceContext
	test	ecx,ecx
	jnz	release_source_entry
	pop	eax
	retn
    release_source_entry:
	cmp	[eax+SourceEntry.type],SOURCE_MACRO
	jne	source_entry_released
	mov	edx,[eax+SourceEntry.text]
	dec	[edx+ValueDefinition.reference_count]
	cmp	[source_context_affected],0
	je	source_entry_released
	and	[edx+ValueDefinition.flags],not VAL_IN_USE
      source_entry_released:
	add	eax,sizeof.SourceEntry
	loop	release_source_entry
	pop	eax
	retn

get_file_source_entry:
; out:
;  ebx - SourceEntry in the main SourceContext
; preserves: edx, esi, edi
	mov	ebx,[source_context]
	mov	ecx,[ebx+SourceContext.number_of_entries]
	mov	eax,ecx
	imul	eax,sizeof.SourceEntry
	lea	ebx,[ebx+sizeof.SourceContext+eax]
      find_file_source_entry:
	sub	ebx,sizeof.SourceEntry
	cmp	[ebx+SourceEntry.type],SOURCE_FILE
	loopne	find_file_source_entry
	retn

get_line:
; out:
;  cf set if there are no more lines to assemble
;  esi = pointer into preprocessed line
	xor	eax,eax
	mov	[symbol_class],SYMCLASS_PARAMETER
	mov	[symbol_expected],al
	mov	[symbol_required],al
	mov	[name_volatile],al
	mov	[preprocessed_context],eax
	mov	ebx,[source_context]
	mov	ecx,[ebx+SourceContext.number_of_entries]
	dec	ecx
	imul	ecx,sizeof.SourceEntry
	lea	ebx,[ebx+sizeof.SourceContext+ecx]
	xchg	eax,[ebx+SourceEntry.number_of_attached_lines]
	inc	eax
	add	[ebx+SourceEntry.line_number],eax
	mov	eax,[ebx+SourceEntry.local_namespace]
	mov	[parameter_namespace],eax
	test	[eax+SymbolTree_Root.flags],NAMESPACE_UNATTACHED
	jz	local_namespace_ok
	xor	eax,eax
    local_namespace_ok:
	mov	[local_namespace],eax
	cmp	[ebx+SourceEntry.type],SOURCE_MACRO
	je	get_line_from_macro
	and	[name_token],0
	mov	esi,[ebx+SourceEntry.text]
	mov	eax,[ebx+SourceEntry.offset]
	add	esi,eax
	mov	[ebx+SourceEntry.line_offset],eax
	cmp	byte [esi],0
	je	source_ended
	mov	edi,[preprocessing_workspace.memory_start]
    preprocess_line_from_file:
	mov	ecx,14
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	lodsb
	cmp	al,1Ah
	je	convert_name_symbol
	cmp	al,22h
	je	convert_quoted_string
	cmp	al,27h
	je	convert_quoted_string
	test	al,al
	jz	file_ended
	cmp	al,0Ah
	je	line_ended
	cmp	al,';'
	jne	preprocess_syntactical_character
	xor	edx,edx
	test	[preprocessing_mode],PMODE_RETAIN_COMMENTS
	jz	skip_comment
    preprocess_syntactical_character:
	stosb
	cmp	al,'`'
	je	convert_parameter
	cmp	al,'\'
	jne	preprocess_line_from_file
	test	[preprocessing_mode],PMODE_ISOLATE_LINES
	jnz	preprocess_line_from_file
	mov	edx,esi
    detect_line_concatenation:
	mov	al,[edx]
	inc	edx
	cmp	al,0Ah
	je	concatenate_line
	cmp	al,';'
	je	concatenation_comment
	cmp	al,20h
	jne	preprocess_line_from_file
	jmp	detect_line_concatenation
    concatenate_line:
	mov	byte [edi-1],20h
	inc	esi
	inc	[ebx+SourceEntry.number_of_attached_lines]
	jmp	preprocess_line_from_file
    concatenation_comment:
	test	[preprocessing_mode],PMODE_RETAIN_COMMENTS
	jnz	preprocess_line_from_file
	mov	byte [edi-1],20h
	mov	esi,edx
	inc	[ebx+SourceEntry.number_of_attached_lines]
    skip_comment:
	lodsb
	test	al,al
	jz	file_ended
	cmp	al,0Ah
	je	comment_ended
	cmp	al,22h
	je	skip_quoted_string
	cmp	al,27h
	je	skip_quoted_string
	cmp	al,1Ah
	jne	skip_comment
	lodsd
	lea	esi,[esi+eax+12]
	jmp	skip_comment
    comment_ended:
	test	edx,edx
	jnz	preprocess_line_from_file
	jmp	line_ended
    skip_quoted_string:
	lodsd
	add	esi,eax
	jmp	skip_comment
    convert_quoted_string:
	stosb
	mov	eax,esi
	stosd
	lodsd
	add	esi,eax
	jmp	preprocess_line_from_file
    convert_name_symbol:
	mov	ecx,[esi]
	lea	eax,[esi+4+ecx+12]
	push	eax ebx
	mov	eax,[eax-4]
	test	eax,eax
	jz	name_symbol_not_cached
	mov	esi,eax
      name_symbol_not_cached:
	call	preprocess_symbol
    name_symbol_converted:
	pop	ebx esi
	jmp	preprocess_line_from_file
    convert_parameter:
	cmp	byte [esi],1Ah
	jne	preprocess_line_from_file
	lodsb
	mov	ecx,[esi]
	lea	eax,[esi+4+ecx+12]
	push	eax ebx
	mov	eax,[eax-4]
	test	eax,eax
	jz	parameter_to_convert_not_cached
	mov	esi,eax
      parameter_to_convert_not_cached:
	call	preprocess_symbol
	jc	name_symbol_converted
	call	convert_parameter_to_string
	jmp	name_symbol_converted
    file_ended:
	dec	esi
    line_ended:
	sub	esi,[ebx+SourceEntry.text]
	mov	[ebx+SourceEntry.offset],esi
    got_line:
	mov	[line_end],edi
	mov	esi,[preprocessing_workspace.memory_start]
	mov	[line_start],esi
	xor	eax,eax
	mov	[line_context],eax
	mov	[embedded_context],eax
      ; clc
	retn
    source_ended:
	mov	ebx,[source_context]
	dec	[ebx+SourceContext.number_of_entries]
	jnz	get_line
	cmp	[ebx+sizeof.SourceContext+SourceEntry.type],SOURCE_MEMORY
	jne	no_more_lines
	mov	esi,[source_file]
	test	esi,esi
	jz	no_more_lines
	cmp	byte [esi],0
	je	no_more_lines
	lea	edi,[ebx+sizeof.SourceContext]
	mov	ecx,sizeof.SourceEntry shr 2
	assert	sizeof.SourceEntry and 11b = 0
	xor	eax,eax
	rep	stosd
	call	read_source
	test	eax,eax
	jz	main_source_file_not_found
	mov	ebx,[source_context]
	inc	[ebx+SourceContext.number_of_entries]
	add	ebx,sizeof.SourceContext
	mov	[ebx+SourceEntry.type],SOURCE_FILE
	mov	[ebx+SourceEntry.name],esi
	mov	[ebx+SourceEntry.text],eax
	mov	eax,[parameter_namespace]
	mov	[ebx+SourceEntry.local_namespace],eax
	jmp	get_line
    no_more_lines:
	stc
	retn
    get_line_from_macro:
	mov	edx,[ebx+SourceEntry.text]
	mov	esi,[edx+ValueDefinition.value]
	mov	eax,[edx+ValueDefinition.value_length]
	add	eax,esi
	mov	[source_end],eax
	mov	eax,[ebx+SourceEntry.offset]
	add	esi,eax
	mov	[ebx+SourceEntry.line_offset],eax
	mov	edi,[preprocessing_workspace.memory_start]
    preprocess_line_from_macro:
	cmp	esi,[source_end]
	je	macro_ended
	mov	ecx,14
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	lodsb
	cmp	al,1Ah
	je	reproduce_name_symbol
	test	al,al
	jz	macro_line_ended
	stosb
	cmp	al,22h
	je	reproduce_quoted_string
	cmp	al,27h
	je	reproduce_quoted_string
	cmp	al,30h
	je	reproduce_internal_token
	cmp	al,40h
	je	reproduce_context_token
	cmp	al,'`'
	je	conversion_operator_in_macro
	jmp	preprocess_line_from_macro
    reproduce_quoted_string:
	movsd
	jmp	preprocess_line_from_macro
    reproduce_internal_token:
	mov	ecx,[esi]
	add	ecx,4
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	lodsd
	stosd
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	jmp	preprocess_line_from_macro
    reproduce_context_token:
	mov	[preprocessed_context],esi
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	cmp	esi,[source_end]
	je	macro_ended
	cmp	byte [esi],40h
	jne	preprocess_line_from_macro
	inc	esi
	sub	edi,sizeof.RecognitionContext
	jmp	reproduce_context_token
    reproduce_name_symbol:
	mov	[name_token],esi
	lodsd
	push	esi ebx
	mov	esi,eax
	call	preprocess_symbol
    name_symbol_reproduced:
	pop	ebx esi
	jmp	preprocess_line_from_macro
    conversion_operator_in_macro:
	cmp	esi,[source_end]
	je	macro_ended
	cmp	byte [esi],1Ah
	jne	preprocess_line_from_macro
	inc	esi
	mov	[name_token],esi
	lodsd
	push	esi ebx
	mov	esi,eax
	call	preprocess_symbol
	jc	name_symbol_reproduced
	call	convert_parameter_to_string
	jmp	name_symbol_reproduced
    macro_line_ended:
	mov	edx,[ebx+SourceEntry.text]
	sub	esi,[edx+ValueDefinition.value]
	mov	[ebx+SourceEntry.offset],esi
	jmp	got_line
    macro_ended:
	mov	edx,[ebx+SourceEntry.text]
	and	[edx+ValueDefinition.flags],not VAL_IN_USE
	dec	[edx+ValueDefinition.reference_count]
	mov	ebx,[source_context]
	dec	[ebx+SourceContext.number_of_entries]
	jnz	get_line
	stc
	retn
    preprocess_symbol:
	mov	eax,edi
	sub	eax,[preprocessing_workspace.memory_start]
	mov	[symbol_value_start],eax
	mov	[symbol_data],esi
	lodsd
	mov	ecx,eax
	mov	eax,[esi+ecx+4]
	mov	[case_insensitive_hash],eax
	mov	edx,[esi+ecx]
	mov	[name_kind],NAME_CASESENSITIVE
	mov	ebx,[parameter_namespace]
	call	scan_namespace
	jnc	parameter_found
	mov	[name_kind],NAME_CASEINSENSITIVE
	mov	ebx,[parameter_namespace]
	test	[ebx+SymbolTree_Root.flags],NAMESPACE_WITH_CASEINSENSITIVE_PARAMETERS
	jz	no_local_parameter_recognized
	mov	edx,[case_insensitive_hash]
	call	scan_namespace
	jnc	parameter_found
    no_local_parameter_recognized:
	cmp	byte [esi],'%'
	jne	no_parameter_recognized
	cmp	ecx,2
	ja	no_parameter_recognized
	jb	current_counter_value
	cmp	byte [esi+1],'%'
	je	current_limit_value
    no_parameter_recognized:
	mov	edi,[preprocessing_workspace.memory_start]
	add	edi,[symbol_value_start]
	mov	al,1Ah
	stosb
	mov	eax,[symbol_data]
	stosd
	stc
	retn
    parameter_found:
	mov	edi,[preprocessing_workspace.memory_start]
	add	edi,[symbol_value_start]
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	je	simple_parameter_value
	cmp	al,VALTYPE_SYMBOLIC_SEQUENCE
	je	iterator_value
	cmp	al,VALTYPE_NUMERIC_SEQUENCE
	je	named_counter_value
	cmp	al,VALTYPE_NATIVE_COMMAND
	jne	no_parameter_recognized
	jmp	[edx+ValueDefinition.value]
    current_counter_value:
	mov	esi,[current_counter]
	cmp	byte [esi],0
	je	no_parameter_recognized
	mov	edi,[preprocessing_workspace.memory_start]
	add	edi,[symbol_value_start]
	movzx	ecx,byte [esi]
	add	ecx,6
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	mov	al,30h
	stosb
	mov	edx,edi
	xor	eax,eax
	lodsb
	stosd
	mov	ecx,eax
	rep	movsb
	test	byte [edi-1],80h
	jz	parameter_replaced
	xor	al,al
	stosb
	inc	dword [edx]
	jmp	parameter_replaced
    current_limit_value:
	mov	ebx,[preprocessing_workspace.memory_start]
	add	ebx,[symbol_value_start]
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	no_block_with_limit
      find_block_with_limit:
	test	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE
	jnz	found_breakable_block
	call	find_next_directive_block
	jnc	find_block_with_limit
      no_block_with_limit:
	mov	edi,ebx
	jmp	no_parameter_recognized
      found_breakable_block:
	test	[edi+DirectiveBlock.flags],CTRLF_HAS_REPEAT_DATA
	jz	no_block_with_limit
	xchg	ebx,edi
	sub	ebx,sizeof.RepeatData
	mov	ecx,[ebx+RepeatData.limit_length]
	add	ecx,6
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	mov	al,30h
	stosb
	mov	edx,edi
	mov	eax,[ebx+RepeatData.limit_length]
	stosd
	sub	ebx,eax
	mov	esi,ebx
	mov	ecx,eax
	rep	movsb
	test	byte [edi-1],80h
	jz	parameter_replaced
	xor	al,al
	stosb
	inc	dword [edx]
	jmp	parameter_replaced
    named_counter_value:
	mov	edx,[edx+ValueDefinition.value]
	mov	ebx,[edx]
	add	ebx,[counters_stack_base]
	cmp	byte [ebx],0
	je	no_parameter_recognized
	add	edx,4
	movzx	ecx,byte [ebx]
	cmp	ecx,[edx]
	jae	estimate_counter_length
	mov	ecx,[edx]
      estimate_counter_length:
	add	ecx,6
	mov	esi,edx
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	mov	al,30h
	stosb
	xor	eax,eax
	stosd
	push	edi
	movzx	ecx,byte [ebx]
	inc	ebx
	lodsd
	sub	eax,ecx
	jnc	counter_base_selected
	add	ecx,eax
	neg	eax
	xchg	ebx,esi
      counter_base_selected:
	mov	edx,eax
	jecxz	counter_added_to_base
	xor	ah,ah
      add_counter_to_base:
	lodsb
	add	al,ah
	setc	ah
	add	al,[ebx]
	adc	ah,0
	inc	ebx
	add	al,-1
	adc	ah,0
	stosb
	loop	add_counter_to_base
      counter_added_to_base:
	mov	ecx,edx
	jecxz	counter_carried
      carry_counter:
	lodsb
	add	al,ah
	setc	ah
	add	al,-1
	adc	ah,0
	stosb
	loop	carry_counter
      counter_carried:
	pop	edx
	mov	al,ah
	dec	al
	jnz	extend_counter_value
	cmp	edx,edi
	je	counter_value_finished
	test	byte [edi-1],80h
	jz	counter_value_finished
      extend_counter_value:
	stosb
      counter_value_finished:
	mov	ecx,edi
	sub	ecx,edx
	mov	[edx-4],ecx
	jmp	parameter_replaced
    iterator_value:
	mov	edx,[edx+ValueDefinition.value]
	mov	ebx,[edx]
	add	ebx,[counters_stack_base]
	movzx	ecx,byte [ebx]
	test	ecx,ecx
	jz	no_parameter_recognized
	push	edi
	mov	edi,value_index
	xor	eax,eax
	mov	[edi],eax
	mov	esi,ebx
	inc	esi
	rep	movsb
	mov	eax,[value_index]
	pop	edi
	shl	eax,2
	mov	esi,[edx+eax]
	mov	ecx,[edx+eax+4]
	sub	ecx,esi
	add	esi,edx
	jmp	copy_parameter_value
    simple_parameter_value:
	mov	esi,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
    copy_parameter_value:
	push	ecx
	mov	edx,preprocessing_workspace
	call	reserve_workspace
	pop	ecx
	cmp	[preprocessed_context],0
	jne	copy_with_preprocessed_context
	mov	al,cl
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	jmp	parameter_replaced
      copy_token_pointer:
	movsd
	sub	ecx,4
      copy_with_preprocessed_context:
	test	ecx,ecx
	jz	parameter_replaced
	lodsb
	stosb
	dec	ecx
	cmp	al,1Ah
	je	copy_token_pointer
	cmp	al,22h
	je	copy_token_pointer
	cmp	al,27h
	je	copy_token_pointer
	cmp	al,30h
	je	copy_token_data
	cmp	al,40h
	jne	copy_with_preprocessed_context
	mov	eax,ecx
	cmp	dword [esi],0
	jne	copy_parameter_context
	mov	edx,esi
	mov	esi,[preprocessed_context]
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	lea	esi,[edx+sizeof.RecognitionContext]
	mov	ecx,eax
	sub	ecx,sizeof.RecognitionContext
	jmp	copy_with_preprocessed_context
      copy_parameter_context:
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	mov	ecx,eax
	sub	ecx,sizeof.RecognitionContext
	jmp	copy_with_preprocessed_context
      copy_token_data:
	lodsd
	stosd
	sub	ecx,4
	sub	ecx,eax
	xchg	ecx,eax
	rep	movsb
	xchg	ecx,eax
	jmp	copy_with_preprocessed_context
    local_symbol_name:
	mov	al,40h
	stosb
	mov	eax,[local_namespace]
	mov	[edi+RecognitionContext.base_namespace],eax
	xor	eax,eax
	mov	[edi+RecognitionContext.current_label],eax
	add	edi,sizeof.RecognitionContext
	mov	al,1Ah
	stosb
	mov	eax,[symbol_data]
	stosd
	mov	al,40h
	stosb
	mov	esi,[preprocessed_context]
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	test	esi,esi
	jz	reset_context
	rep	movsd
	stc
	retn
    reset_context:
	xor	eax,eax
	rep	stosd
	stc
	retn
    parameter_replaced:
	mov	[symbol_value_end],edi
	mov	eax,[preprocessing_workspace.memory_start]
	add	[symbol_value_start],eax
	clc
	retn
    convert_parameter_to_string:
	mov	esi,[symbol_value_start]
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	edi,4
      convert_token_to_text:
	mov	ecx,[symbol_value_end]
	sub	ecx,esi
	jbe	finish_parameter_to_string_conversion
	mov	edx,assembly_workspace
	call	reserve_workspace
	lodsb
	cmp	al,1Ah
	je	convert_name_token_to_text
	cmp	al,22h
	je	convert_string_token_to_text
	cmp	al,27h
	je	convert_string_token_to_text
	cmp	al,30h
	je	convert_internal_number_to_text
	cmp	al,40h
	je	ignore_context_token
	stosb
	jmp	convert_token_to_text
      ignore_context_token:
	add	esi,sizeof.RecognitionContext
	jmp	convert_token_to_text
      convert_name_token_to_text:
	lodsd
	mov	ebx,esi
	mov	esi,eax
	mov	ecx,[esi]
	mov	edx,assembly_workspace
	call	reserve_workspace
	lodsd
	mov	ecx,eax
	rep	movsb
	mov	esi,ebx
	jmp	convert_token_to_text
      convert_string_token_to_text:
	lodsd
	mov	ebx,esi
	mov	esi,eax
	call	enclose_string
	mov	esi,ebx
	cmp	byte [esi-4-1],27h
	jne	convert_token_to_text
	dec	edi
	jmp	convert_token_to_text
      enclose_string:
	mov	ecx,[esi]
	inc	ecx
	shl	ecx,1
	mov	edx,assembly_workspace
	call	reserve_workspace
	lodsd
	mov	ecx,eax
	mov	al,27h
	stosb
      copy_string_characters:
	jecxz	string_characters_copied
	lodsb
	stosb
	dec	ecx
	cmp	al,27h
	jne	copy_string_characters
	stosb
	jmp	copy_string_characters
      string_characters_copied:
	mov	al,27h
	stosb
	retn
      convert_internal_number_to_text:
	mov	edx,esi
	lodsd
	add	esi,eax
	push	esi edi
	call	convert_number_back
	pop	edi
	mov	esi,edx
	mov	ecx,[esi]
	mov	edx,assembly_workspace
	call	reserve_workspace
	lodsd
	mov	ecx,eax
	rep	movsb
	pop	esi
	jmp	convert_token_to_text
      finish_parameter_to_string_conversion:
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	lea	eax,[ecx-4]
	mov	[esi],eax
	mov	ebx,[memory_source_cache]
	xor	eax,eax
	call	put_into_map
	mov	edi,[symbol_value_start]
	dec	edi
	mov	al,22h
	stosb
	mov	eax,esi
	stosd
	retn

move_to_next_symbol:
; in:
;  esi = pointer into preprocessed line or current embedded value
;  zeroed ecx is recommended for whitespace detection
; out:
;  esi = pointer advanced past the whitespace and context tokens
;  cf set when reached end of line or embedded value
;  ecx increased when there was whitespace on the way, preserved otherwise
;  when cf = 0:
;   esi - next symbol
;   al = initial byte of the next symbol
; preserves: ebx, edx, edi
	cmp	esi,[line_end]
	jb	next_token_available
	stc
	retn
    next_token_available:
	mov	al,[esi]
	cmp	al,40h
	je	set_embedded_context
	cmp	al,20h
	je	pass_whitespace
	clc
	retn
    pass_whitespace:
	inc	esi
	inc	ecx
	jmp	move_to_next_symbol
    set_embedded_context:
	inc	esi
	cmp	[esi+RecognitionContext.base_namespace],0
	je	restore_embedded_context
	mov	[embedded_context],esi
	add	esi,sizeof.RecognitionContext
	jmp	move_to_next_symbol
    restore_embedded_context:
	add	esi,sizeof.RecognitionContext
	mov	eax,[number_of_line_embeddings]
	test	eax,eax
	jz	clear_embedded_context
	dec	eax
	imul	eax,sizeof.LineEmbedding
	add	eax,[line_embeddings]
	mov	eax,[eax+LineEmbedding.recognition_context]
	mov	[embedded_context],eax
	jmp	move_to_next_symbol
    clear_embedded_context:
	and	[embedded_context],0
	jmp	move_to_next_symbol

warp_to_next_symbol:
; in:
;  esi = pointer into preprocessed line or current embedded value
;  zeroed ecx is recommended for whitespace detection
; out:
;  esi - next symbol
;  cf set when end of line reached
;  ecx increased when there was whitespace on the way, preserved otherwise
;  when cf = 0:
;   al = initial byte of the next symbol
; preserves: ebx, edx, edi
	call	move_to_next_symbol
	jc	warp_through_embedding_boundary
	retn
    warp_through_embedding_boundary:
	mov	eax,[number_of_line_embeddings]
	sub	eax,1
	jc	reached_end_of_line
	mov	[number_of_line_embeddings],eax
	imul	eax,sizeof.LineEmbedding
	add	eax,[line_embeddings]
	mov	esi,eax
	push	edx
	add	ecx,[esi+LineEmbedding.whitespace]
	mov	edx,[esi+LineEmbedding.recognition_context]
	mov	[embedded_context],edx
	mov	edx,[esi+LineEmbedding.definition]
	dec	[edx+ValueDefinition.reference_count]
	and	[edx+ValueDefinition.flags],not VAL_IN_USE
	mov	edx,[esi+LineEmbedding.previous_end]
	mov	[line_end],edx
	mov	esi,[esi+LineEmbedding.previous_pointer]
	pop	edx
	jmp	warp_to_next_symbol
    reached_end_of_line:
       ; stc
	retn

cut_piece_of_line:
; in:
;  esi = pointer into preprocessed line or current embedded value
;  edi - LineExcerpt to be filled with information about cut piece of line
;  dl = initial byte of symbol that should end the cut value, zero to cut up to the end of line (or current embedded value)
;  dh = initial byte of symbol that would open the nested piece, zero for no nesting
;  [breakpoint_token] = initial byte of symbol that should uncoditionally end the cut value if it is met
; out:
;  esi = pointer advanced past the cut piece
;  al = initial byte of symbol at pointer, zero when no more symbols there
; preserves: ebx, edx, edi
; note: when [breakpoint_token] has the same value as either dl or dh, it has no effect
	mov	[number_of_enclosings],1
	call	move_to_next_symbol
	mov	[edi+LineExcerpt.data_start],esi
	mov	[edi+LineExcerpt.data_end],esi
	jc	last_piece_in_line
	mov	ecx,[embedded_context]
	mov	[edi+LineExcerpt.recognition_context],ecx
    cut_piece:
	cmp	al,dh
	je	nested_piece
	cmp	al,dl
	je	close_nested_piece
	cmp	al,[breakpoint_token]
	jne	cut_token
	retn
    close_nested_piece:
	dec	[number_of_enclosings]
	jz	end_of_piece
    cut_token:
	cmp	al,1Ah
	je	cut_token_with_data
	cmp	al,22h
	je	cut_token_with_data
	cmp	al,27h
	je	cut_token_with_data
	cmp	al,30h
	je	cut_internal_token
	cmp	al,40h
	je	cut_context_token
	inc	esi
    cut_next_token:
	mov	[edi+LineExcerpt.data_end],esi
	call	move_to_next_symbol
	jnc	cut_piece
    last_piece_in_line:
	xor	al,al
    end_of_piece:
	mov	ecx,[embedded_context]
	mov	[edi+LineExcerpt.leftover_context],ecx
	retn
    nested_piece:
	inc	[number_of_enclosings]
	jmp	cut_token
    cut_token_with_data:
	add	esi,1+4
	jmp	cut_next_token
    cut_internal_token:
	inc	esi
	lodsd
	add	esi,eax
	jmp	cut_next_token
    cut_context_token:
	add	esi,1+sizeof.RecognitionContext
	jmp	cut_next_token

compare_symbolic_values:
; in:
;  esi - first symbolic value
;  edi - second symbolic value
;  ecx = length to compare
; out:
;  ecx = zero when values equal, or a number of bytes following the point of difference
;  esi - the point of difference in first value
;  edi - the point of difference in second value
	mov	al,[esi]
	cmp	al,[edi]
	jne	symbolic_values_compared
	inc	esi
	inc	edi
	cmp	al,1Ah
	je	compare_tokens_with_data
	cmp	al,22h
	je	compare_tokens_with_data
	cmp	al,27h
	je	compare_tokens_with_data
	cmp	al,30h
	je	compare_internal_tokens
	cmp	al,40h
	je	compare_context_tokens
	loop	compare_symbolic_values
    symbolic_values_compared:
	retn
    compare_tokens_with_data:
	dec	ecx
	mov	eax,[esi]
	mov	edx,[edi]
	cmp	eax,edx
	jne	compare_token_data
	add	esi,4
	add	edi,4
	sub	ecx,4
	jnz	compare_symbolic_values
	retn
    compare_token_data:
	mov	ebx,[eax]
	cmp	ebx,[edx]
	jne	symbolic_values_compared
	add	eax,ebx
	add	edx,ebx
	xchg	esi,eax
	xchg	edi,edx
	xchg	ecx,ebx
	dec	ecx
	shr	ecx,2
	inc	ecx
	cmp	byte [eax-1],1Ah
	jne	compare_token_dwords
	inc	ecx
	add	esi,4
	add	edi,4
    compare_token_dwords:
	std
	repe	cmpsd
	cld
	jne	token_data_content_differs
	lea	esi,[eax+4]
	lea	edi,[edx+4]
	mov	ecx,ebx
	sub	ecx,4
	jnz	compare_symbolic_values
	retn
    token_data_content_differs:
	mov	esi,eax
	mov	edi,edx
	mov	ecx,ebx
	retn
    compare_internal_tokens:
	mov	eax,[esi]
	cmp	eax,[edi]
	jne	symbolic_values_compared
	add	esi,4
	add	edi,4
	sub	ecx,1+4
	sub	ecx,eax
	xchg	ecx,eax
	repe	cmpsb
	je	internal_tokens_equal
	inc	ecx
	add	ecx,eax
	retn
    internal_tokens_equal:
	add	ecx,eax
	jnz	compare_symbolic_values
	retn
    compare_context_tokens:
	dec	ecx
	assert	sizeof.RecognitionContext and 11b = 0
	mov	edx,sizeof.RecognitionContext shr 2
    compare_contexts:
	mov	eax,[esi]
	cmp	eax,[edi]
	jne	symbolic_values_compared
	add	esi,4
	add	edi,4
	sub	ecx,4
	jz	symbolic_values_compared
	dec	edx
	jnz	compare_contexts
	jmp	compare_symbolic_values

embed_symbolic_value:
; in:
;  ebx - SymbolTree_Leaf
;  edx - ValueDefinition
;  esi = pointer into preprocessed line or current embedded value
;  ecx = number of whitespace tokens to imply after emdedding
; out:
;  esi = pointer into embedded value
; preserves: ebx, edx
	mov	eax,[number_of_line_embeddings]
	inc	eax
	mov	[number_of_line_embeddings],eax
	imul	eax,sizeof.LineEmbedding
	cmp	eax,[line_embeddings_maximum_length]
	jbe	line_embedding_allocated
	push	eax ecx edx
	mov	ecx,sizeof.LineEmbedding
	add	ecx,[line_embeddings_maximum_length]
	mov	eax,[line_embeddings]
	call	grow_stack
	mov	[line_embeddings_maximum_length],ecx
	mov	[line_embeddings],eax
	pop	edx ecx eax
    line_embedding_allocated:
	sub	eax,sizeof.LineEmbedding
	add	eax,[line_embeddings]
	mov	[eax+LineEmbedding.whitespace],ecx
	mov	edi,[line_end]
	mov	[eax+LineEmbedding.previous_pointer],esi
	mov	[eax+LineEmbedding.previous_end],edi
	mov	ecx,[embedded_context]
	mov	[eax+LineEmbedding.recognition_context],ecx
	mov	esi,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	add	ecx,esi
	mov	[line_end],ecx
	mov	[eax+LineEmbedding.definition],edx
	inc	[edx+ValueDefinition.reference_count]
	or	[edx+ValueDefinition.flags],VAL_IN_USE
	retn

clear_line_embeddings:
; preserves: ebx, ecx, edx, esi, edi
	xor	eax,eax
	mov	[line_start],eax
	mov	[line_end],eax
	cmp	eax,[number_of_line_embeddings]
	je	embedded_values_ok
	push	ecx
    discard_line_embedding:
	mov	ecx,eax
	imul	ecx,sizeof.LineEmbedding
	add	ecx,[line_embeddings]
	mov	ecx,[ecx+LineEmbedding.definition]
	dec	[ecx+ValueDefinition.reference_count]
	and	[ecx+ValueDefinition.flags],not VAL_IN_USE
	inc	eax
	cmp	eax,[number_of_line_embeddings]
	jne	discard_line_embedding
	and	[number_of_line_embeddings],0
	pop	ecx
    embedded_values_ok:
	retn

identify_symbol:
; in:
;  esi = pointer into preprocessed line or current embedded value
;  dl = SYMCLASS_#
;  [symbol_definition] = non-zero when symbol needs to be identified for the purpose of definition
; out:
;  cf set when there were no more symbols in preprocessed line or current embedded value
;  esi = pointer advanced past the processed identifier and following whitespace or to the first token of a different symbol
;  ecx = number of whitespace tokens encountered immediately before the new position
;  [recognition_context] = applied recognition context
;  when cf = 0:
;   ebx - SymbolTree_Leaf, null when no symbol was identified
;   edx - SymbolTree_Foliage, may be null if either ebx or edi is null
;   edi - SymbolTree_Root, null when there was no identifier at all or when identified symbol is not in a namespace
;   [symbol_start] - first token of identified symbol (within current embedded value or preprocessed line)
;   [symbol_solid] = non-zero when initial component of identifier was a name and not a dot
; note:
;  when ebx is null but edi is not, the identifier was malformed
;  when edi is null but ebx is not, a special symbol was identified; this is possible only for SYMCLASS_INSTRUCTION and SYMCLASS_STRUCTURE
	xor	ebx,ebx
	mov	[expected_class],dl
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	identification_finished
	mov	edx,[embedded_context]
	test	edx,edx
	jz	use_current_namespace
	mov	eax,[edx+RecognitionContext.base_namespace]
	mov	[recognition_context.base_namespace],eax
	mov	eax,[edx+RecognitionContext.current_label]
	mov	[recognition_context.current_label],eax
	jmp	namespaces_ok
    identify_symbol_in_namespace:
    ; in:
    ;  ebx - SymbolTree_Root
    ;  esi = pointer into preprocessed line or current embedded value
    ;  dl = SYMCLASS_#
    ;  [symbol_definition] = non-zero when symbol needs to be identified for the purpose of definition
    ; out:
    ;  cf set when there were no more symbols in preprocessed line or current embedded value
    ;  esi = pointer advanced past the processed identifier and following whitespace or to the first token of a different symbol
    ;  ecx = number of whitespace tokens encountered immediately before the new position
    ;  when cf = 0:
    ;	ebx - SymbolTree_Leaf, null when no symbol was identified
    ;	edx - SymbolTree_Foliage, may be null if either ebx or edi is null
    ;	edi - SymbolTree_Root, null when there was no identifier at all or when identified symbol is not in a namespace
    ;	[symbol_start] - first token of identified symbol
    ;	[symbol_solid] = non-zero when initial component of identifier was a name and not a dot
    ; note:
    ;  when ebx is null but edi is not, the identifier was malformed
    ;  when edi is null but ebx is not, a special symbol was identified; this is possible only for SYMCLASS_INSTRUCTION and SYMCLASS_STRUCTURE
	mov	[expected_class],dl
	xor	ecx,ecx
	xor	edx,edx
	call	move_to_next_symbol
	jnc	namespaces_ok
	retn
    use_current_namespace:
	mov	eax,[current_namespace]
	mov	[recognition_context.base_namespace],eax
	mov	edx,[local_namespace]
	test	edx,edx
	jz	no_structure_label
	mov	edx,[edx+SymbolTree_Root.current_label]
	test	edx,edx
	jz	no_structure_label
	mov	[recognition_context.current_label],edx
	jmp	namespaces_ok
    no_structure_label:
	mov	eax,[eax+SymbolTree_Root.current_label]
	mov	[recognition_context.current_label],eax
    namespaces_ok:
	mov	[symbol_start],esi
	and	[symbol_solid],0
	cmp	[symbol_definition],0
	setnz	dh
	shl	dh,bsf RECOGNIZE_DEFINITION
	mov	[recognizer_setting],dh
	xor	edx,edx
	xor	edi,edi
	mov	al,[esi]
	cmp	al,1Ah
	je	starting_with_name
	cmp	al,'.'
	je	starting_dot
	cmp	al,'#'
	je	starting_with_concatenation
	cmp	al,'?'
	je	starting_question_mark
    return_no_identifier:
	xor	ebx,ebx
    return_no_namespace:
	xor	edx,edx
	xor	edi,edi
	clc
	retn
    starting_with_name:
	call	detect_numeric_symbol
	jc	return_no_identifier
    valid_starting_name:
	or	[symbol_solid],1
    valid_name:
	inc	esi
	mov	[name_token],esi
	lodsd
	mov	edi,eax
	and	[name_volatile],0
	xor	ecx,ecx
    identify_name:
	mov	dh,[recognizer_setting]
	call	move_to_next_symbol
	jc	run_recognizer
	cmp	al,'#'
	jne	name_complete
	call	check_concatenation
	jnc	name_concatenation
	xor	al,al
    name_complete:
	test	ecx,ecx
	jnz	run_recognizer
	cmp	al,'?'
	jne	run_recognizer
	or	dh,RECOGNIZE_CASE_INSENSITIVE
	inc	esi
	call	move_to_next_symbol
	jc	run_recognizer
	cmp	al,'#'
	jne	name_modifiers_complete
	call	check_concatenation
	jc	run_recognizer
    name_modifiers_complete:
	test	ecx,ecx
	jnz	run_recognizer
	cmp	al,1Ah
	je	malformed_identifier
    run_recognizer:
	push	esi ecx
	cmp	esi,[line_end]
	je	recognize_final_symbol
	test	ecx,ecx
	jnz	recognize_final_symbol
	cmp	byte [esi],'.'
	jne	recognize_final_symbol
	mov	esi,edi
	lodsd
	mov	ecx,eax
	mov	dl,SYMCLASS_EXPRESSION
	and	dh,not RECOGNIZE_DEFINITION
	call	recognize_symbol
	pop	ecx esi
    dot_operator:
	inc	esi
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	ending_with_dot
	cmp	al,'#'
	je	name_concatenation_after_dot
	test	ecx,ecx
	jnz	ending_with_dot
	cmp	al,'.'
	je	multiple_dot_operator
    identify_name_after_dot:
	cmp	al,30h
	je	number_after_dot
	cmp	al,1Ah
	jne	ending_with_dot
	call	get_symbol_namespace
	xor	edi,edi
	jmp	valid_name
    name_concatenation_after_dot:
	call	check_concatenation
	jc	ending_with_dot
	cmp	al,'.'
	je	multiple_dot_operator
    number_after_dot:
	call	get_symbol_namespace
	mov	al,[esi]
	xor	edi,edi
	jmp	name_concatenation
    recognize_final_symbol:
	mov	dl,[expected_class]
	mov	esi,edi
	lodsd
	mov	ecx,eax
	call	recognize_symbol
	pop	ecx esi
    symbol_identified:
	clc
    identification_finished:
	retn
    ending_with_dot:
	mov	al,[expected_class]
	cmp	al,SYMCLASS_EXPRESSION
	jne	find_expected_class
	clc
	retn
    starting_question_mark:
	inc	esi
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	alone_question_mark
	cmp	al,'#'
	jne	symbol_after_question_mark
	call	check_concatenation
	jc	alone_question_mark
    symbol_after_question_mark:
	test	ecx,ecx
	jnz	alone_question_mark
	cmp	[symbol_definition],0
	jne	no_forced_definition
	mov	[expected_class],SYMCLASS_EXPRESSION
	or	[recognizer_setting],RECOGNIZE_DEFINITION
    no_forced_definition:
	cmp	al,'.'
	je	starting_dot
	cmp	al,1Ah
	je	valid_starting_name
	cmp	al,30h
	je	concatenation_with_internal_number
    alone_question_mark:
	mov	dl,[expected_class]
	cmp	dl,SYMCLASS_INSTRUCTION
	je	return_interceptor_symbol
	cmp	dl,SYMCLASS_STRUCTURE
	je	return_label_interceptor_symbol
	mov	eax,[symbol_start]
	cmp	byte [eax],'?'
	jne	malformed_identifier
	mov	esi,eax
	jmp	return_no_identifier
    return_label_interceptor_symbol:
	mov	ebx,[label_interceptor_symbol]
	jmp	return_no_namespace
    return_interceptor_symbol:
	mov	ebx,[interceptor_symbol]
	jmp	return_no_namespace
    multiple_dot_operator:
	mov	ebx,edi
	xor	edi,edi
	jmp	starting_dot
    additional_dot:
	inc	edi
    starting_dot:
	inc	esi
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	alone_dot
	cmp	al,'#'
	jne	symbol_after_starting_dot
	call	check_concatenation
	jc	alone_dot
    symbol_after_starting_dot:
	test	ecx,ecx
	jnz	alone_dot
	cmp	al,'.'
	je	additional_dot
	cmp	al,30h
	je	name_after_starting_dot
	cmp	al,1Ah
	jne	alone_dot
    name_after_starting_dot:
	test	ebx,ebx
	jnz	name_after_multiple_dot_operator
	call	get_current_label
	mov	edi,ebx
	mov	al,[esi]
	jmp	identify_name_after_dot
    name_after_multiple_dot_operator:
	test	edx,edx
	jz	parent_namespace_ready
	call	get_symbol_namespace
    parent_namespace_ready:
	call	synthesize_dot_label
	mov	edi,ebx
	mov	al,[esi]
	jmp	identify_name_after_dot
    alone_dot:
	test	ebx,ebx
	jz	identify_current_label
	test	edx,edx
	jz	malformed_identifier
	push	ecx
	call	get_symbol_namespace
	pop	ecx
	call	synthesize_dot_label
	jmp	get_label_symbol
    malformed_identifier:
	xor	ebx,ebx
	xor	edx,edx
	mov	edi,[recognition_context.base_namespace]
	jmp	symbol_identified
    identify_current_label:
	call	get_current_label
    get_label_symbol:
	mov	edi,ebx
	mov	al,[expected_class]
    find_expected_class:
	mov	[symbol_class],al
	mov	al,[symbol_definition]
	mov	[symbol_required],al
	mov	[symbol_expected],al
	mov	ebx,edx
	call	scan_symbol_branch
	jnc	current_label_identified
	mov	[symbol_class],SYMCLASS_EXPRESSION
	or	[symbol_required],1
	or	[symbol_expected],1
	mov	ebx,edx
	call	scan_symbol_branch
      current_label_identified:
	clc
	retn
    get_current_label:
	mov	edx,[recognition_context.current_label]
	test	edi,edi
	jnz	synthesize_dot_label
	test	edx,edx
	jnz	current_label_ready
    synthesize_dot_label:
	push	ecx esi
	mov	esi,[identifier_workspace.memory_start]
	mov	eax,edi
	mov	[esi],eax
	mov	edx,FNV_OFFSET
	xor	ecx,ecx
      hash_dot_label:
	test	eax,eax
	jz	dot_label_synthesised
	xor	dl,al
	imul	edx,FNV_PRIME
	inc	ecx
	shr	eax,8
	jmp	hash_dot_label
      dot_label_synthesised:
	or	[name_volatile],1
	and	[name_token],0
	mov	[name_kind],NAME_SYNTHETIC
	mov	[symbol_class],SYMCLASS_EXPRESSION
	or	[symbol_required],1
	or	[symbol_expected],1
	test	ebx,ebx
	jnz	identify_dot_label
	mov	ebx,[recognition_context.base_namespace]
      identify_dot_label:
	call	scan_namespace
	pop	esi ecx
      current_label_ready:
	mov	ebx,[edx+SymbolTree_Foliage.root]
	retn
    starting_with_concatenation:
	xor	ecx,ecx
	call	check_concatenation
	jc	empty_concatenation
    name_concatenation:
	cmp	al,30h
	je	concatenation_with_internal_number
	cmp	al,1Ah
	jne	empty_concatenation
	test	edi,edi
	jz	starting_with_name
	inc	esi
	lodsd
	push	ebx esi
	mov	ebx,eax
    attach_to_name:
	mov	esi,edi
	mov	edi,[identifier_workspace.memory_start]
	cmp	esi,edi
	jne	initial_concatenation
	lodsd
	add	esi,eax
	mov	edi,esi
	jmp	get_precalculated_hashes
    initial_concatenation:
	test	esi,esi
	jz	initial_conversion
	mov	ecx,[esi]
	add	ecx,4+8
	mov	edx,identifier_workspace
	call	reserve_workspace
	lodsd
	stosd
	mov	ecx,eax
	rep	movsb
    get_precalculated_hashes:
	xchg	ebx,esi
	mov	ecx,[esi]
	mov	eax,[identifier_workspace.memory_start]
	add	[eax],ecx
	add	ecx,8
	mov	edx,identifier_workspace
	call	reserve_workspace
	mov	ecx,[ebx]
	mov	edx,[ebx+4]
	lodsd
	lea	ebx,[esi+eax]
	xor	eax,eax
    concatenation_hash:
	lodsb
	xor	cl,al
	xor	dl,[characters+eax]
	imul	ecx,FNV_PRIME
	imul	edx,FNV_PRIME
	stosb
	cmp	esi,ebx
	jne	concatenation_hash
	mov	eax,ecx
	stosd
	mov	eax,edx
	stosd
	pop	esi ebx
	mov	edi,[identifier_workspace.memory_start]
	or	[name_volatile],1
	and	[name_token],0
	xor	ecx,ecx
	jmp	identify_name
    initial_conversion:
	xor	eax,eax
	stosd
	mov	esi,edi
	mov	eax,FNV_OFFSET
	mov	[esi],eax
	mov	[esi+4],eax
	jmp	get_precalculated_hashes
    concatenation_with_internal_number:
	inc	esi
	mov	edx,esi
	lodsd
	add	esi,eax
	push	ebx esi
	push	edi
	call	convert_number_back
	pop	edi
	mov	ebx,edx
	jmp	attach_to_name
    empty_concatenation:
	test	edi,edi
	jnz	identify_name_with_empty_concatenation
	call	move_to_next_symbol
	jc	malformed_identifier
	cmp	al,'.'
	je	starting_dot
	cmp	al,'?'
	je	starting_question_mark
	jmp	malformed_identifier
    identify_name_with_empty_concatenation:
	mov	dh,[recognizer_setting]
	call	move_to_next_symbol
	jc	run_recognizer
	jmp	name_complete

detect_numeric_symbol:
; in:
;  esi - name token in preprocessed line or in current embedded value
; out:
;  cf set when symbol starting with this token should be considered numeric
; preserves: ebx, ecx, edx, esi, edi
	mov	eax,[esi+1]
	mov	al,[eax+4]
	cmp	al,'$'
	je	detect_pascal_hexadecimal
	sub	al,'0'
	cmp	al,10
	jb	numeric_symbol_detected
      not_a_numeric_symbol:
	clc
	retn
      detect_pascal_hexadecimal:
	mov	eax,[esi+1]
	cmp	dword [eax],2
	jb	not_a_numeric_symbol
	movzx	eax,byte [eax+4+1]
	mov	al,[characters+eax]
	sub	al,'0'
	cmp	al,10
	jb	numeric_symbol_detected
	sub	al,'a'-'0'
	jc	not_a_numeric_symbol
	cmp	al,16-10
	jae	not_a_numeric_symbol
      numeric_symbol_detected:
	stc
	retn

check_concatenation:
; in:
;  esi - concatenation character in preprocessed line or in current embedded value
;  ecx = number of whitespace tokens encountered immediately before current position
; out:
;  cf set when there is no concatenation applicable to previous symbol
;  esi = pointer advanced past the processed operator and the whitespace that follows it
;  ecx = number of whitespace tokens encountered immediately before the new position
;  when cf = 0:
;   esi - symbol that follows concatenation operator
;   al = initial byte of symbol following concatenation operator
; preserves: ebx, edx, edi
	test	ecx,ecx
	jnz	no_concatenation
	inc	esi
	call	move_to_next_symbol
	jc	no_concatenation
	cmp	al,'#'
	je	check_concatenation
	test	ecx,ecx
	jnz	no_concatenation
      ; clc
	retn
    no_concatenation:
	stc
	retn

get_literal:
; in:
;  esi - token in preprocessed line or in current embedded value
; out:
;  al = type of value
;  esi = pointer advanced past the processed literal and the whitespace that follows it
;  ecx = number of whitespace tokens encountered immediately before the new position
;  when al = 22h:
;   edx - 32-bit length followed by string data
;  when al = 30h:
;   edx - 32-bit length followed by numeric data, null when invalid number
;  when al = 2Eh:
;   edx - FloatData, null when invalid number
;  when al is any other value, it is a simple special character, and edx is zero
	xor	ecx,ecx
	mov	al,[esi]
	cmp	al,1Ah
	je	get_literal_number
	cmp	al,22h
	je	get_literal_string
	cmp	al,27h
	je	missing_end_quote
	cmp	al,30h
	je	get_internal_number
	inc	esi
	mov	dl,al
	call	move_to_next_symbol
	mov	al,dl
	xor	edx,edx
	retn
    get_literal_number:
	mov	edx,[esi+1]
	add	esi,5
    check_for_more_parts_of_number:
	call	move_to_next_symbol
	jc	number_ready_for_conversion
	test	ecx,ecx
	jnz	check_for_number_concatenation
	cmp	al,'.'
	je	get_literal_float
    check_for_number_concatenation:
	cmp	al,'#'
	jne	number_ready_for_conversion
	call	check_concatenation
	jc	number_ready_for_conversion
    number_concatenation:
	cmp	al,30h
	je	attach_internal_number_to_number
	cmp	al,'.'
	je	get_literal_float
	cmp	al,1Ah
	jne	number_ready_for_conversion
    attach_name_to_number:
	mov	ebx,[esi+1]
	add	esi,5
	push	esi
    attach_to_number:
	mov	esi,edx
	mov	edi,[identifier_workspace.memory_start]
	cmp	esi,edi
	jne	initial_concatenation_of_number
	lodsd
	add	esi,eax
	mov	edi,esi
	jmp	attach_segment_of_number
    initial_concatenation_of_number:
	mov	ecx,[esi]
	add	ecx,4
	mov	edx,identifier_workspace
	call	reserve_workspace
	lodsd
	stosd
	mov	ecx,eax
	rep	movsb
    attach_segment_of_number:
	mov	esi,ebx
	mov	ecx,[esi]
	mov	eax,[identifier_workspace.memory_start]
	add	[eax],ecx
	mov	edx,identifier_workspace
	call	reserve_workspace
	lodsd
	mov	ecx,eax
	rep	movsb
	pop	esi
	mov	edx,[identifier_workspace.memory_start]
	jmp	check_for_more_parts_of_number
    attach_internal_number_to_number:
	mov	ebx,edx
	inc	esi
	mov	edx,esi
	lodsd
	add	esi,eax
	push	esi
	push	ebx
	call	convert_number_back
	mov	ebx,edx
	pop	edx
	jmp	attach_to_number
    number_ready_for_conversion:
	push	ecx
	call	convert_number
	pop	ecx
	jc	get_literal_float
	mov	al,30h
	retn
    missing_end_quote:
	mov	edx,_missing_end_quote
	call	register_error
    get_literal_string:
	mov	edx,[esi+1]
	add	esi,5
	call	move_to_next_symbol
	mov	al,22h
	retn
    get_internal_number:
	lea	edx,[esi+1]
	mov	eax,[edx]
	lea	esi,[esi+1+4+eax]
	call	move_to_next_symbol
	jc	internal_number_ready
	test	ecx,ecx
	jnz	check_for_internal_number_concatenation
	cmp	al,'.'
	je	convert_internal_number_back
    check_for_internal_number_concatenation:
	cmp	al,'#'
	jne	internal_number_ready
	call	check_concatenation
	jc	internal_number_ready
	cmp	al,1Ah
	je	convert_internal_number_back
	cmp	al,30h
	jne	internal_number_ready
    convert_internal_number_back:
	push	esi
	call	convert_number_back
	mov	esi,edx
	mov	ecx,[esi]
	add	ecx,4
	mov	edi,[identifier_workspace.memory_start]
	mov	edx,identifier_workspace
	call	reserve_workspace
	lodsd
	stosd
	mov	ecx,eax
	rep	movsb
	pop	esi
	mov	edx,[identifier_workspace.memory_start]
	mov	al,[esi]
	jmp	number_concatenation
    internal_number_ready:
	mov	al,30h
	retn
    get_literal_float:
	xor	eax,eax
	mov	[zero_digits],eax
	mov	[decimal_places],eax
	mov	[literal_exponent],eax
	mov	[literal_exponent_sign],al
	mov	[literal_fractional_part],al
	mov	[waiting_for_digit],al
	push	esi ecx
	mov	ebx,edx
	call	start_decimal_converter
	lea	esi,[ebx+4]
	mov	ecx,[ebx]
    get_float_digit:
	lodsb
	cmp	al,27h
	je	skip_float_digit
	cmp	al,'_'
	je	skip_float_digit
	cmp	al,'9'
	ja	invalid_float_digit
	sub	al,'0'
	jz	float_digit_zero
	jc	invalid_float_digit
	inc	[decimal_places]
	push	esi ecx
	xor	ecx,ecx
	xchg	ecx,[zero_digits]
	call	convert_decimal_digit
	pop	ecx esi
	and	[waiting_for_digit],0
      skip_float_digit:
	loop	get_float_digit
	jmp	float_digits_ok
      float_digit_zero:
	inc	[decimal_places]
	inc	[zero_digits]
	and	[waiting_for_digit],0
	loop	get_float_digit
      float_digits_ok:
	pop	ecx esi
	call	move_to_next_symbol
	jc	no_more_tokens_in_float
	cmp	al,'.'
	je	decimal_point
	cmp	al,'#'
	jne	no_more_tokens_in_float
	call	check_concatenation
	jc	no_more_tokens_in_float
	jmp	next_token_in_float
    decimal_point:
	test	ecx,ecx
	jnz	no_more_tokens_in_float
	mov	al,[literal_fractional_part]
	or	al,[literal_exponent_sign]
	jnz	no_more_tokens_in_float
	or	[literal_fractional_part],1
	or	[waiting_for_digit],1
	and	[decimal_places],0
    get_following_digits:
	inc	esi
	xor	ecx,ecx
	call	move_to_adjacent_symbol
	jc	invalid_float_value
    next_token_in_float:
	cmp	al,1Ah
	je	next_digits_section
	cmp	al,30h
	je	internal_number_as_digits
    no_more_tokens_in_float:
	cmp	[waiting_for_digit],0
	jne	invalid_float_value
	call	finish_decimal_conversion
	push	esi ecx
	mov	esi,edi
	xor	ecx,ecx
	cmp	[literal_fractional_part],0
	je	float_decimal_places_ok
	mov	ecx,[decimal_places]
      float_decimal_places_ok:
	sub	ecx,[zero_digits]
	neg	ecx
	add	ecx,[literal_exponent]
	jo	float_conversion_failed
	call	multiply_float_by_power_of_ten
	test	[edi+FloatData.attributes],FLOAT_INFINITE or FLOAT_INDETERMINATE or FLOAT_UNDERFLOW
	jnz	float_conversion_failed
	mov	edx,edi
	pop	ecx esi
	mov	al,2Eh
	retn
    float_conversion_failed:
	pop	ecx esi
    invalid_float_value:
	xor	edx,edx
	mov	al,2Eh
	retn
    next_digits_section:
	inc	esi
	lodsd
	xor	ecx,ecx
	push	esi ecx
	mov	esi,eax
	lodsd
	mov	ecx,eax
	cmp	[literal_exponent_sign],0
	jne	get_exponent_digit
	jmp	get_float_digit
    internal_number_as_digits:
	inc	esi
	mov	edx,esi
	lodsd
	add	esi,eax
	xor	ecx,ecx
	push	esi ecx
	call	convert_number_back
	mov	esi,edx
	lodsd
	mov	ecx,eax
	cmp	[literal_exponent_sign],0
	jne	get_exponent_digit
	jmp	get_float_digit
    get_literal_exponent:
	mov	al,1
	xchg	al,[literal_exponent_sign]
	test	al,al
	jnz	invalid_literal_float
	or	[waiting_for_digit],1
	loop	get_exponent_digit
	pop	ecx esi
	call	move_to_adjacent_symbol
	jc	invalid_literal_exponent
	cmp	al,1Ah
	je	next_digits_section
	cmp	al,30h
	je	internal_number_as_digits
	cmp	al,'+'
	je	get_following_digits
	cmp	al,'-'
	jne	invalid_literal_exponent
	neg	[literal_exponent_sign]
	jmp	get_following_digits
    get_exponent_digit:
	xor	eax,eax
	lodsb
	cmp	al,27h
	je	skip_exponent_digit
	cmp	al,'_'
	je	skip_exponent_digit
	sub	al,'0'
	jc	invalid_float_digit
	cmp	al,10
	jae	invalid_float_digit
	mov	edx,[literal_exponent]
	imul	edx,10
	jo	invalid_literal_float
	cmp	[literal_exponent_sign],0
	jnl	exponent_digit_ok
	neg	eax
      exponent_digit_ok:
	add	edx,eax
	jo	invalid_literal_float
	mov	[literal_exponent],edx
	and	[waiting_for_digit],0
      skip_exponent_digit:
	loop	get_exponent_digit
	jmp	float_digits_ok
    invalid_float_digit:
	cmp	[waiting_for_digit],0
	jne	invalid_literal_float
	movzx	eax,byte [esi-1]
	mov	al,[characters+eax]
	cmp	al,'e'
	je	get_literal_exponent
	cmp	al,'f'
	jne	invalid_literal_float
	dec	ecx
	jnz	invalid_literal_float
	pop	ecx esi
	or	edx,-1
	jmp	check_for_redundant_segments
    invalid_literal_float:
	pop	ecx esi
    invalid_literal_exponent:
	xor	edx,edx
    check_for_redundant_segments:
	call	move_to_next_symbol
	jc	no_redundant_segments
	cmp	al,'#'
	jne	no_redundant_segments
	call	check_concatenation
	jc	no_redundant_segments
	cmp	al,1Ah
	je	skip_redundant_segment
	cmp	al,30h
	jne	no_redundant_segments
	add	esi,[esi+1]
      skip_redundant_segment:
	add	esi,1+4
	jmp	check_for_redundant_segments
    no_redundant_segments:
	test	edx,edx
	jnz	no_more_tokens_in_float
	mov	al,[literal_fractional_part]
	or	al,[literal_exponent_sign]
	jnz	invalid_float_value
	mov	al,30h
	retn
    move_to_adjacent_symbol:
	call	move_to_next_symbol
	jc	adjacent_symbol_ok
	cmp	al,'#'
	je	check_concatenation
	neg	ecx
      adjacent_symbol_ok:
	retn

skip_literal:
; in:
;  esi - token in preprocessed line or in current embedded value
; out:
;  esi = pointer advanced past the processed literal and the whitespace that follows it
;  ecx = number of whitespace tokens encountered immediately before the new position
	xor	ecx,ecx
	mov	al,[esi]
	cmp	al,1Ah
	je	skip_literal_number
	cmp	al,22h
	je	skip_literal_string
	cmp	al,27h
	je	skip_literal_string
	cmp	al,30h
	je	skip_internal_number
	inc	esi
	call	move_to_next_symbol
	retn
    skip_literal_number:
	add	esi,5
    skip_number_segments:
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	literal_symbol_skipped
	cmp	al,'#'
	jne	literal_symbol_skipped
    check_literal_concatenation:
	call	check_concatenation
	jc	literal_symbol_skipped
	cmp	al,30h
	je	skip_attached_internal_number
	cmp	al,1Ah
	jne	literal_symbol_skipped
    skip_attached_number:
	add	esi,5
	jmp	skip_number_segments
    skip_attached_internal_number:
	inc	esi
	lodsd
	add	esi,eax
	jmp	skip_number_segments
    skip_literal_string:
	add	esi,5
	call	move_to_next_symbol
    literal_symbol_skipped:
	retn
    skip_internal_number:
	inc	esi
	lodsd
	add	esi,eax
	call	move_to_next_symbol
	jc	literal_symbol_skipped
	cmp	al,'#'
	je	check_literal_concatenation
	retn

get_processed_value:
; in:
;  esi = pointer into preprocessed line or current embedded value
; out:
;  cf set when there were no more symbols on the line
;  when cf = 0:
;   al = type of value
;   esi = pointer advanced past the processed element and the whitespace that follows it within the boundaries of current embedded value or preprocessed line
;   ecx = number of whitespace tokens encountered immediately before the new position
;   when al = 1Ah:
;    ebx - SymbolTree_Leaf, null when malformed identifier
;    edx - ValueDefinition, null when symbol with no valid value
;    additional variables set as by identify_symbol
;   when al = 22h:
;    edx - 32-bit length followed by string data
;   when al = 30h:
;    edx - 32-bit length followed by numeric data, null when invalid number
;   when al = 2Eh:
;    edx - FloatData, null when invalid number
;   when al is any other value, it is a simple special character, and edx is zero
; note:
;  to detect whitespace between two consecutive symbols, warp_to_next_symbol should be called after this function,
;  since it may further increase ecx when there is additional whitespace outside of current embedded value
	call	get_raw_value
	jc	no_more_values
	cmp	al,1Ah
	jne	value_ok
	test	edx,edx
	jz	value_ok
	cmp	[edx+ValueDefinition.type],VALTYPE_SYMBOLIC
	je	symbolic_value
    value_ok:
	clc
	retn
    symbolic_value:
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	no_infinite_regress
	test	[edx+ValueDefinition.flags],VAL_IN_USE
	jz	no_infinite_regress
	push	edx
	mov	edx,_symbolic_self_reference
	call	register_error
	pop	edx
	retn
    no_infinite_regress:
	call	embed_symbolic_value
	jmp	get_processed_value

get_raw_value:
; same as get_processed_value, but it does not expand symbolic value and returns it just like any other
	call	warp_to_next_symbol
	jc	no_more_values
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	get_raw_value
	test	edi,edi
	jz	literal_value
	mov	edi,edx
	test	ebx,ebx
	jz	invalid_symbol_value
	call	get_available_value
	jc	invalid_symbol_value
	mov	al,1Ah
	clc
	retn
    invalid_symbol_value:
	xor	edx,edx
	mov	al,1Ah
	clc
	retn
    literal_value:
	call	get_literal
	clc
	retn
    no_more_values:
       ; stc
	retn
