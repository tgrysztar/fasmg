
struct CompiledMacroHeader
	shared_namespace dd ?
	literals_offset dd ?
	label_argument_leaf dd ?
ends

struct CompiledMacroArgument
	symbol_leaf dd ?
	default_value_length dd ?
	default_value_offset dd ?
ends

struct UnresolvedJump
	offset dd ?
	entry_length dd ?
      ; source_context SourceContext
ends

struct MatchedExcerpt
	data_start dd ?
	data_end dd ?
	recognition_context dd ?
	symbol_leaf dd ?
ends

BOOL_NEG = -1
BOOL_AND = -2
BOOL_OR = -3

define_calm_instruction:
	mov	dl,DBLOCK_CALMINSTRUCTION
	mov	ecx,sizeof.MacroData
	call	add_directive_block
	mov	edx,[ebx+SymbolTree_Leaf.branch]
	call	get_symbol_namespace
	mov	[alm_namespace],ebx
	or	[assembly_mode],AMODE_CALM_DEFINITION
	mov	[calm_definition_active],0
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	mov	eax,[calm_code_buffer.memory_start]
	mov	[calm_code_cursor],eax
	mov	eax,[calm_literals_buffer.memory_start]
	mov	[calm_literals_cursor],eax
	mov	eax,[calm_auxiliary_buffer.memory_start]
	mov	[calm_auxiliary_cursor],eax
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,'('
	je	calm_labeled_instruction
	and	[argument_start],0
	mov	dl,SYMCLASS_INSTRUCTION
    get_calm_instruction_identifier:
	call	get_macro_definition_symbol
	jc	invalid_identifier
	push	esi
	mov	eax,ebx
	mov	ecx,[alm_namespace]
	call	get_local_anchor
	call	get_symbol_namespace
	or	[ebx+SymbolTree_Root.flags],NAMESPACE_CALM
	mov	[new_local_namespace],ebx
	pop	esi
	mov	edi,[calm_code_buffer.memory_start]
	mov	[edi+CompiledMacroHeader.shared_namespace],ebx
	or	[symbol_definition],1
	mov	ebx,[argument_start]
	test	ebx,ebx
	jz	calm_instruction_label_argument_ok
	push	esi edi
	mov	esi,ebx
	mov	dl,SYMCLASS_EXPRESSION
	mov	ebx,[new_local_namespace]
	call	identify_symbol_in_namespace
	mov	eax,esi
	pop	edi esi
	jc	invalid_argument
	test	ebx,ebx
	jz	invalid_argument
	cmp	byte [eax],')'
	jne	invalid_argument
	call	update_value_definition
	push	esi edi
	mov	[value_type],VALTYPE_RESERVED
	xor	ecx,ecx
	call	assign_value
	pop	edi esi
    calm_instruction_label_argument_ok:
	mov	[edi+CompiledMacroHeader.label_argument_leaf],ebx
	add	edi,sizeof.CompiledMacroHeader
    calm_instruction_argument_declaration:
	call	move_to_next_symbol
	jc	calm_instruction_arguments_declared
	mov	dl,SYMCLASS_EXPRESSION
	push	edi
	mov	ebx,[new_local_namespace]
	call	identify_symbol_in_namespace
	pop	edi
	jc	invalid_argument
	test	ebx,ebx
	jz	invalid_argument
	mov	edx,calm_code_buffer
	mov	ecx,sizeof.CompiledMacroArgument+4
	call	reserve_workspace
	mov	[edi+CompiledMacroArgument.symbol_leaf],ebx
	call	update_value_definition
	push	esi edi
	mov	[value_type],VALTYPE_RESERVED
	xor	ecx,ecx
	call	assign_value
	pop	edi esi
	xor	edx,edx
	call	move_to_next_symbol
	jc	calm_instruction_argument_ready
	cmp	al,':'
	je	calm_instruction_argument_default_value
	cmp	al,'='
	je	calm_instruction_argument_default_value
	cmp	al,'*'
	jne	calm_instruction_argument_ready
	dec	edx
	inc	esi
    calm_instruction_argument_ready:
	mov	[edi+CompiledMacroArgument.default_value_length],edx
	add	edi,sizeof.CompiledMacroArgument
	call	move_to_next_symbol
	jc	calm_instruction_arguments_declared
	inc	esi
	cmp	al,'&'
	je	calm_instruction_greedy_argument
	cmp	al,','
	jne	invalid_argument
	call	move_to_next_symbol
	jc	invalid_argument
	jmp	calm_instruction_argument_declaration
    calm_instruction_argument_default_value:
	inc	esi
	mov	[calm_code_cursor],edi
	mov	edx,calm_literals_buffer
	mov	edi,[calm_literals_cursor]
	mov	ecx,[line_end]
	sub	ecx,esi
	add	ecx,1+sizeof.RecognitionContext
	call	reserve_workspace
	mov	[calm_literals_cursor],edi
	call	move_to_next_symbol
	jc	calm_plain_default_value
	cmp	al,'<'
	je	calm_enclosed_default_value
    calm_plain_default_value:
	mov	dl,','
	xor	dh,dh
	mov	[breakpoint_token],'&'
	call	extract_piece_of_line
	jmp	calm_default_value_ready
    calm_enclosed_default_value:
	inc	esi
	mov	dl,'>'
	mov	dh,'<'
	mov	[breakpoint_token],0
	call	extract_piece_of_line
	cmp	al,'>'
	jne	invalid_argument
	inc	esi
    calm_default_value_ready:
	mov	edx,edi
	xchg	edi,[calm_literals_cursor]
	mov	eax,edi
	sub	edx,edi
	sub	eax,[calm_literals_buffer.memory_start]
	mov	edi,[calm_code_cursor]
	mov	[edi+CompiledMacroArgument.default_value_offset],eax
	jmp	calm_instruction_argument_ready
    calm_instruction_greedy_argument:
	or	eax,-1
	stosd
	jmp	call_instruction_begin_code
    calm_labeled_instruction:
	inc	esi
	call	move_to_next_symbol
	jc	missing_argument
	mov	[argument_start],esi
	mov	edi,[expression_workspace.memory_start]
	mov	dl,')'
	xor	dh,dh
	mov	[breakpoint_token],dh
	call	cut_piece_of_line
	cmp	al,')'
	jne	invalid_argument
	inc	esi
	mov	dl,SYMCLASS_STRUCTURE
	jmp	get_calm_instruction_identifier
    calm_instruction_arguments_declared:
	xor	eax,eax
	stosd
    call_instruction_begin_code:
	mov	[calm_code_cursor],edi
	mov	eax,[new_local_namespace]
	mov	[current_context.base_namespace],eax
	and	[calm_line_number],0
	mov	[calm_definition_active],1
	jmp	instruction_assembled

assemble_alm_instruction:
	or	[alm_statement],1
	mov	ebx,[source_context]
	mov	eax,[ebx+SourceContext.number_of_entries]
	dec	eax
	imul	eax,sizeof.SourceEntry
	test	[ebx+sizeof.SourceContext+eax+SourceEntry.flags],SRCF_ALM_STATEMENT
	jnz	identify_alm_instruction
	inc	[calm_line_number]
    identify_alm_instruction:
	call	move_to_next_symbol
	jc	empty_line
	call	get_alm_identifier
	jc	unrecognized_alm_instruction
	mov	dl,SYMCLASS_INSTRUCTION
	call	identify_alm_symbol
	jc	alm_label
	call	get_available_value
	jc	alm_label
	test	[edx+ValueDefinition.flags],VAL_UNCONDITIONAL
	jnz	execute_alm_instruction
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
    execute_alm_instruction:
	cmp	[edx+ValueDefinition.type],VALTYPE_CALM
	je	launch_calm
	cmp	[edx+ValueDefinition.type],VALTYPE_SYMBOLIC
	je	use_macro
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_COMMAND
	jne	unrecognized_alm_instruction
	jmp	[edx+ValueDefinition.value]
    alm_label:
	call	move_to_next_symbol
	jc	unrecognized_alm_instruction
	cmp	al,':'
	jne	unrecognized_alm_instruction
	inc	esi
	test	[assembly_mode],AMODE_SKIP
	jnz	identify_alm_instruction
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_label_to_macro
	call	identify_calm_location
	jc	unrecognized_alm_instruction
	call	create_constant_value_definition
	test	edx,edx
	jz	assembly_line
	mov	[edx+ValueDefinition.type],VALTYPE_PLAIN
	mov	eax,[calm_code_cursor]
	sub	eax,[calm_code_buffer.memory_start]
	mov	[edx+ValueDefinition.value],eax
	mov	eax,[current_pass]
	mov	[edx+ValueDefinition.pass],eax
	jmp	identify_alm_instruction
    unrecognized_alm_instruction:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	mov	edx,_unexpected_instruction
	call	register_error
	jmp	assembly_line
    get_alm_identifier:
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	malformed_alm_identifier
	cmp	al,1Ah
	je	alm_plain_name
	cmp	al,30h
	je	alm_numeric_name
	cmp	al,'#'
	jne	malformed_alm_identifier
	call	check_concatenation
	jnc	get_alm_identifier
      malformed_alm_identifier:
	stc
	retn
      alm_plain_name:
	inc	esi
	lodsd
	mov	ebx,eax
	mov	ecx,[ebx]
	mov	eax,[ebx+4+ecx]
	mov	[case_sensitive_hash],eax
	mov	eax,[ebx+4+ecx+4]
	mov	[case_insensitive_hash],eax
      alm_check_for_concatenation:
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	alm_identifier_ok
	cmp	al,'#'
	jne	alm_identifier_ok
	call	check_concatenation
	jc	alm_identifier_ok
	call	move_to_next_symbol
	jc	alm_identifier_ok
	cmp	al,1Ah
	je	alm_concatenation
	cmp	al,30h
	je	alm_concatenation
      alm_identifier_ok:
	mov	[symbol_data],ebx
	cmp	ebx,[assembly_workspace.memory_start]
	je	alm_identifier_volatile
	mov	[name_volatile],0
	clc
	retn
      alm_identifier_volatile:
	sub	edi,ebx
	sub	edi,4
	mov	[ebx],edi
	mov	[name_volatile],1
	clc
	retn
      alm_concatenation:
	mov	edx,assembly_workspace
	cmp	ebx,[edx+Workspace.memory_start]
	je	alm_name_segment
	mov	edi,[edx+Workspace.memory_start]
	add	edi,4
	mov	ecx,[ebx]
	call	reserve_workspace
	xchg	ebx,esi
	lodsd
	mov	ecx,eax
	rep	movsb
	mov	esi,ebx
	mov	ebx,[edx+Workspace.memory_start]
      alm_name_segment:
	lodsb
	cmp	al,30h
	je	alm_attach_numeric_segment
	lodsd
	push	ebx esi
      alm_append_name:
	mov	esi,eax
	mov	ecx,[esi]
	mov	edx,assembly_workspace
	call	reserve_workspace
	lodsd
	lea	ebx,[esi+eax]
	mov	ecx,[case_sensitive_hash]
	mov	edx,[case_insensitive_hash]
	xor	eax,eax
      alm_name_hash:
	lodsb
	xor	cl,al
	xor	dl,[characters+eax]
	imul	ecx,FNV_PRIME
	imul	edx,FNV_PRIME
	stosb
	cmp	esi,ebx
	jne	alm_name_hash
	mov	[case_sensitive_hash],ecx
	mov	[case_insensitive_hash],edx
	pop	esi ebx
	jmp	alm_check_for_concatenation
      alm_numeric_name:
	inc	esi
	mov	ebx,[assembly_workspace.memory_start]
	lea	edi,[ebx+4]
	mov	eax,FNV_OFFSET
	mov	[case_sensitive_hash],eax
	mov	[case_insensitive_hash],eax
      alm_attach_numeric_segment:
	mov	edx,esi
	mov	ecx,[edx]
	lea	esi,[esi+4+ecx]
	push	ebx esi
	push	edi
	call	convert_number_back
	pop	edi
	mov	eax,edx
	jmp	alm_append_name
    identify_alm_symbol:
	mov	[symbol_class],dl
	push	esi
	mov	esi,[symbol_data]
	lodsd
	mov	ecx,eax
	mov	edx,[case_insensitive_hash]
	mov	ebx,[alm_namespace]
	mov	[name_kind],NAME_CASEINSENSITIVE
	and	[name_token],0
	and	[symbol_required],0
	and	[symbol_expected],0
	call	scan_namespace
	pop	esi
	retn
    identify_calm_location:
	mov	[symbol_class],SYMCLASS_CALM_LOCATION
	push	esi
	mov	esi,[symbol_data]
	lodsd
	mov	ecx,eax
	mov	edx,[case_sensitive_hash]
	mov	ebx,[current_context.base_namespace]
	and	[name_kind],0
	and	[name_token],0
	or	[symbol_required],1
	or	[symbol_expected],1
	call	scan_namespace
	pop	esi
	retn

alm_jyes:
	mov	[value],calm_jyes
	jmp	assemble_alm_jump
alm_jno:
	mov	[value],calm_jno
	jmp	assemble_alm_jump
alm_jump:
	mov	[value],calm_jump
    assemble_alm_jump:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	call	move_to_next_symbol
	jc	missing_argument
	call	get_alm_identifier
	jc	invalid_argument
	call	identify_calm_location
	jc	invalid_argument
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	test	edx,edx
	jz	alm_unresolved_jump
	mov	eax,[edx+ValueDefinition.pass]
	cmp	eax,[current_pass]
	jne	alm_unresolved_jump
	mov	ebx,[edx+ValueDefinition.value]
    emit_calm_jump:
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+4
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,[value]
	stosd
	mov	eax,ebx
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled
    alm_unresolved_jump:
	push	esi
	mov	edx,calm_auxiliary_buffer
	mov	edi,[calm_auxiliary_cursor]
	mov	esi,[source_context]
	mov	ecx,[esi+SourceContext.number_of_entries]
	imul	ecx,sizeof.SourceEntry
	add	ecx,sizeof.SourceContext
	add	ecx,sizeof.UnresolvedJump
	call	reserve_workspace
	mov	[calm_auxiliary_cursor],edi
	mov	eax,[calm_code_cursor]
	add	eax,8
	sub	eax,[calm_code_buffer.memory_start]
	mov	[edi+UnresolvedJump.offset],eax
	add	edi,sizeof.UnresolvedJump
	call	clone_source_context
	mov	ecx,edi
	xchg	edi,[calm_auxiliary_cursor]
	sub	ecx,edi
	mov	[edi+UnresolvedJump.entry_length],ecx
	pop	esi
	jmp	emit_calm_jump

alm_end:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_unconditional_alm_instruction
	call	move_to_next_symbol
	jc	finish_calm_definition
	call	get_alm_identifier
	jc	invalid_argument
	mov	dl,SYMCLASS_STRUCTURE
	call	identify_alm_symbol
	jc	invalid_argument
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_COMMAND
	jne	invalid_argument
	cmp	[edx+ValueDefinition.value],alm_end
	jne	invalid_argument
    finish_calm_definition:
	mov	dl,DBLOCK_CALMINSTRUCTION
	call	find_directive_block
	jc	unexpected_unconditional_alm_instruction
	call	close_directive_block
	and	[assembly_mode],not AMODE_CALM_DEFINITION
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	cmp	[calm_definition_active],0
	je	instruction_assembled
	push	esi
	mov	esi,[calm_auxiliary_buffer.memory_start]
    resolve_calm_jumps:
	cmp	esi,[calm_auxiliary_cursor]
	je	calm_jumps_resolved
	mov	eax,[esi+UnresolvedJump.offset]
	add	eax,[calm_code_buffer.memory_start]
	mov	ebx,[eax]
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	test	edx,edx
	jz	unresolvable_calm_jump
	mov	ecx,[edx+ValueDefinition.pass]
	cmp	ecx,[current_pass]
	jne	unresolvable_calm_jump
	mov	ebx,[edx+ValueDefinition.value]
	mov	[eax],ebx
    resolve_next_calm_jump:
	add	esi,[esi+UnresolvedJump.entry_length]
	jmp	resolve_calm_jumps
    unresolvable_calm_jump:
	mov	edx,[calm_code_cursor]
	sub	edx,[calm_code_buffer.memory_start]
	mov	[eax],edx
	add	esi,sizeof.UnresolvedJump
	mov	edx,_undefined_jump_target
	call	register_delayed_error
	sub	esi,sizeof.UnresolvedJump
	jmp	resolve_next_calm_jump
    calm_jumps_resolved:
	mov	ebx,[current_context.base_namespace]
	mov	edx,[ebx+SymbolTree_Root.parent_branch]
	mov	eax,[edx+SymbolTree_Foliage.root]
	mov	[current_context.base_namespace],eax
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,[calm_literals_cursor]
	mov	esi,[calm_literals_buffer.memory_start]
	sub	ecx,esi
	add	ecx,8
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_end
	stosd
	mov	ebx,[calm_code_buffer.memory_start]
	mov	eax,edi
	sub	eax,ebx
	mov	[ebx+CompiledMacroHeader.literals_offset],eax
	mov	ecx,[calm_literals_cursor]
	sub	ecx,esi
	rep	movsb
	mov	ebx,[macro_leaf]
	test	ebx,ebx
	jz	calm_definition_done
	call	create_value_definition
	test	edx,edx
	jz	calm_definition_done
	mov	esi,[calm_code_buffer.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	mov	[value_type],VALTYPE_CALM
	call	assign_value
	mov	al,[macro_flags]
	or	[edx+ValueDefinition.flags],al
    calm_definition_done:
	pop	esi
	jmp	instruction_assembled
    unexpected_unconditional_alm_instruction:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	jmp	unexpected_instruction

alm_local:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	call	move_to_next_symbol
	jc	missing_argument
	or	[symbol_definition],1
	mov	dl,SYMCLASS_EXPRESSION
	mov	ebx,[current_context.base_namespace]
	call	identify_symbol_in_namespace
	jc	invalid_argument
	test	ebx,ebx
	jz	invalid_argument
	call	update_value_definition
	test	edx,edx
	jz	alm_next_local
	push	esi
	mov	[value_type],VALTYPE_RESERVED
	xor	ecx,ecx
	call	assign_value
	pop	esi
    alm_next_local:
	call	move_to_next_symbol
	jc	instruction_assembled
	cmp	al,','
	jne	invalid_argument
	inc	esi
	jmp	alm_local

alm_assemble:
	mov	[value],calm_assemble
	jmp	assemble_alm_operation_on_variable
alm_stringify:
	mov	[value],calm_stringify
    assemble_alm_operation_on_variable:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+4
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,[value]
	stosd
	mov	eax,ebx
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled

alm_arrange:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,','
	jne	instruction_assembled
	inc	esi
	call	move_to_next_symbol
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+12
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_arrange
	stosd
	mov	eax,ebx
	stosd
	mov	edx,calm_literals_buffer
	mov	ebx,[calm_literals_cursor]
	mov	eax,ebx
	sub	eax,[edx+Workspace.memory_start]
	stosd
	mov	[calm_code_cursor],edi
	mov	edi,ebx
	mov	ecx,[line_end]
	sub	ecx,esi
	shl	ecx,1
	call	reserve_workspace
	mov	[breakpoint_token],0
	call	parse_pattern
	mov	[calm_literals_cursor],edi
	mov	eax,edi
	mov	edi,[calm_code_cursor]
	sub	eax,[calm_literals_buffer.memory_start]
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled
    parse_pattern:
    ; in:
    ;  esi - first token of the pattern in the preprocessed line
    ;  edi - buffer with enough space to hold twice the length of the preprocessed text left in line
    ;  [breakpoint_token] = initial byte of symbol that should end the pattern
    ; out:
    ;  esi - after the processed portion of line (might be at [line_end])
    ;  edi - in the buffer after the stored pattern
	cmp	esi,[line_end]
	je	pattern_parsed
	lodsb
	cmp	al,[breakpoint_token]
	je	pattern_breakpoint
    token_in_pattern:
	cmp	al,'='
	je	parse_pattern_literal
	cmp	al,1Ah
	je	parse_reference_in_pattern
	cmp	al,40h
	je	ignore_context_in_pattern
	cmp	al,20h
	je	whitespace_in_pattern
	stosb
    literal_token_in_pattern:
	cmp	al,22h
	je	copy_pattern_data
	cmp	al,27h
	je	copy_pattern_data
	cmp	al,30h
	jne	parse_pattern
	lodsd
	mov	ecx,eax
	stosd
	rep	movsb
	jmp	parse_pattern
    whitespace_in_pattern:
	cmp	esi,[line_end]
	je	pattern_parsed
	lodsb
	cmp	al,20h
	je	whitespace_in_pattern
	cmp	al,[breakpoint_token]
	je	pattern_breakpoint
	cmp	al,40h
	jne	store_whitespace_in_pattern
	call	pass_context
	jmp	whitespace_in_pattern
    store_whitespace_in_pattern:
	mov	byte [edi],20h
	inc	edi
	jmp	token_in_pattern
    pattern_breakpoint:
	dec	esi
    pattern_parsed:
	retn
    pass_context:
	mov	eax,esi
	add	esi,sizeof.RecognitionContext
	cmp	dword [eax],0
	je	zero_context
	mov	[embedded_context],eax
	retn
    zero_context:
	and	[embedded_context],0
	retn
    copy_pattern_data:
	movsd
	jmp	parse_pattern
    ignore_context_in_pattern:
	call	pass_context
	jmp	parse_pattern
    ignore_context_in_pattern_literal:
	call	pass_context
    parse_pattern_literal:
	cmp	esi,[line_end]
	je	error_in_parsed_pattern
	lodsb
	cmp	al,40h
	je	ignore_context_in_pattern_literal
	cmp	al,20h
	je	parse_hard_space_in_pattern
	stosb
	cmp	al,1Ah
	je	copy_pattern_data
	jmp	literal_token_in_pattern
    parse_hard_space_in_pattern:
	mov	al,0A0h
	stosb
	jmp	parse_pattern
    parse_reference_in_pattern:
	dec	esi
	call	detect_numeric_symbol
	jc	numeric_name_in_pattern
	inc	esi
	mov	[name_token],esi
	lodsd
	mov	dx,SYMCLASS_EXPRESSION
	mov	[name_volatile],0
	push	esi edi
	mov	esi,eax
	lodsd
	mov	ecx,eax
	xor	ebx,ebx
	mov	eax,[embedded_context]
	test	eax,eax
	jz	use_current_namespace_in_pattern
	mov	eax,[eax+RecognitionContext.base_namespace]
	jmp	namespace_in_pattern_ok
      use_current_namespace_in_pattern:
	mov	eax,[current_context.base_namespace]
      namespace_in_pattern_ok:
	mov	[recognition_context.base_namespace],eax
	call	recognize_symbol
	pop	edi esi
	mov	al,1Bh
	stosb
	mov	eax,[name_token]
	mov	eax,[eax]
	stosd
	mov	eax,ebx
	stosd
	jmp	parse_pattern
    numeric_name_in_pattern:
	movsb
	movsd
	jmp	parse_pattern
    error_in_parsed_pattern:
	mov	edx,_invalid_argument
	call	register_error
	jmp	parse_pattern

alm_match:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	call	move_to_next_symbol
	jc	missing_argument
	mov	edx,calm_literals_buffer
	mov	edi,[calm_literals_cursor]
	mov	ecx,[line_end]
	sub	ecx,esi
	shl	ecx,1
	call	reserve_workspace
	mov	[calm_literals_cursor],edi
	mov	[breakpoint_token],','
	call	parse_pattern
	xchg	edi,[calm_literals_cursor]
	mov	[pattern_start],edi
	cmp	esi,[line_end]
	je	missing_argument
	lodsb
	cmp	al,','
	jne	invalid_argument
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	and	[brackets],0
	call	move_to_next_symbol
	jc	no_brackets
	cmp	al,','
	jne	no_brackets
	inc	esi
	call	move_to_next_symbol
	jc	invalid_argument
	mov	dl,al
	lea	edi,[esi+1]
	call	skip_token
	cmp	esi,edi
	jne	invalid_argument
	call	move_to_next_symbol
	jc	invalid_argument
	mov	dh,al
	lea	edi,[esi+1]
	call	skip_token
	cmp	esi,edi
	jne	invalid_argument
	mov	esi,[pattern_start]
    ensure_no_brackets_in_pattern:
	cmp	esi,[calm_literals_cursor]
	je	brackets_ok
	mov	al,[esi]
	cmp	al,dl
	je	invalid_argument
	cmp	al,dh
	je	invalid_argument
	cmp	al,1Bh
	je	skip_reference_in_pattern
	call	skip_token
	jmp	ensure_no_brackets_in_pattern
      skip_reference_in_pattern:
	add	esi,1+8
	jmp	ensure_no_brackets_in_pattern
    brackets_ok:
	mov	esi,edi
	mov	word [brackets],dx
    no_brackets:
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+16
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_match
	stosd
	mov	eax,ebx
	stosd
	mov	ebx,[calm_literals_buffer.memory_start]
	mov	eax,[pattern_start]
	sub	eax,ebx
	stosd
	mov	eax,[calm_literals_cursor]
	sub	eax,ebx
	stosd
	mov	eax,[brackets]
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled

alm_compute:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,','
	jne	instruction_assembled
	inc	esi
	call	move_to_next_symbol
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+8
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_compute
	stosd
	mov	eax,ebx
	stosd
	xor	eax,eax
	stosd
	mov	[calm_code_cursor],edi
	mov	edi,[expression_workspace.memory_start]
	and	[leave_opening_parentheses],0
	or	[use_raw_values],1
	call	parse_expression
	and	[use_raw_values],0
	push	esi
	mov	esi,[expression_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	call	reserve_workspace
	mov	[calm_code_cursor],edi
	call	convert_parsed_expression
	pop	esi
	jc	no_computable_expression
	mov	ecx,edi
	xchg	edi,[calm_code_cursor]
	sub	ecx,edi
	mov	[edi-4],ecx
	jmp	instruction_assembled
    no_computable_expression:
	sub	[calm_code_cursor],8+8
	jmp	assembly_line
    convert_parsed_expression:
	lodsd
	test	eax,eax
	jz	parsed_expression_converted
	cmp	al,EXPR_SYMBOL_VALUE
	je	copy_parsed_symbol
	cmp	al,EXPR_OPERATOR
	je	copy_parsed_operator
	cmp	al,EXPR_NUMBER
	je	copy_parsed_value
	cmp	al,EXPR_STRING
	je	copy_parsed_value
	cmp	al,EXPR_FLOAT
	je	copy_parsed_value
	cmp	al,EXPR_MISSING_PARENTHESIS
	je	expression_missing_parenthesis
	jmp	invalid_expression
      copy_parsed_value:
	or	eax,EXPRF_CALM_LITERAL
	mov	ebx,[esi]
	add	esi,4
	test	eax,EXPRF_VALUE_IN_WORKSPACE
	jnz	parsed_value_in_workspace
	test	ebx,ebx
	jnz	parsed_value_pointer_ok
	jmp	invalid_expression
      parsed_value_in_workspace:
	add	ebx,[value_workspace.memory_start]
	and	eax,not EXPRF_VALUE_IN_WORKSPACE
      parsed_value_pointer_ok:
	cmp	al,EXPR_FLOAT
	je	parsed_float_to_copy
	mov	ecx,[ebx]
	add	ecx,4
	jmp	parsed_value_length_ok
      parsed_float_to_copy:
	mov	ecx,sizeof.FloatData
      parsed_value_length_ok:
	stosd
	mov	edx,[calm_literals_cursor]
	mov	eax,edx
	sub	eax,[calm_literals_buffer.memory_start]
	stosd
	push	edi
	push	ecx
	mov	edi,edx
	mov	edx,calm_literals_buffer
	call	reserve_workspace
	pop	ecx
	xchg	esi,ebx
	rep	movsb
	mov	[calm_literals_cursor],edi
	mov	esi,ebx
	pop	edi
	jmp	convert_parsed_expression
    copy_parsed_symbol:
	mov	al,EXPR_SYMBOL
	stosd
	lodsd
	test	eax,eax
	jz	invalid_expression
	stosd
	add	esi,4
	jmp	convert_parsed_expression
    copy_parsed_operator:
	stosd
	movsd
	jmp	convert_parsed_expression
    parsed_expression_converted:
	stosd
	clc
	retn

alm_check:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	eax,edi
	sub	eax,[edx+Workspace.memory_start]
	mov	[calm_rollback_offset],eax
	mov	ecx,8
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_check
	stosd
	mov	[calm_code_cursor],edi
	or	[use_raw_values],1
	mov	ebx,[condition_stack_base]
	and	dword [ebx],0
	add	ebx,4
	mov	[condition_stack],ebx
    parse_alm_condition:
	call	peek_at_constituent_value
	jc	calm_condition_parsed
	cmp	al,'~'
	jne	parse_alm_logical_value
	and	[current_constituent],0
	mov	ecx,BOOL_NEG
	call	push_to_condition_stack
	jmp	parse_alm_condition
    alm_logical_value_empty:
	test	ecx,ecx
	jz	calm_condition_parsed
	call	push_to_condition_stack
	jmp	parse_alm_condition
    parse_alm_logical_value:
	call	parse_logical_value
	jc	alm_logical_value_empty
	jecxz	convert_logical_value
	call	push_to_condition_stack
    convert_logical_value:
	mov	[calm_source_pointer],esi
	mov	esi,[expression_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	add	ecx,8
	add	ecx,[condition_stack]
	sub	ecx,[condition_stack_base]
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	call	reserve_workspace
	xor	eax,eax
	stosd
	mov	[calm_code_cursor],edi
	mov	eax,[comparator]
	stosd
      convert_argument_sequences:
	call	convert_parsed_expression
	jc	condition_malformed
	cmp	esi,[expression_end]
	jb	convert_argument_sequences
	mov	ecx,edi
	xchg	edi,[calm_code_cursor]
	sub	ecx,edi
	mov	[edi-4],ecx
	mov	esi,[calm_source_pointer]
    parse_alm_logical_operator:
	call	get_constituent_value
	jc	calm_condition_parsed
	cmp	al,')'
	je	close_subconditions
	cmp	al,'|'
	je	parse_or
	cmp	al,'&'
	jne	condition_malformed
    parse_and:
	mov	ecx,BOOL_AND
	jmp	prepare_stack_for_logical_operator
    parse_or:
	mov	ecx,BOOL_OR
      prepare_stack_for_logical_operator:
	mov	ebx,[condition_stack]
	sub	ebx,4
	cmp	dword [ebx],0
	jge	push_logical_operator
	mov	[condition_stack],ebx
	mov	edi,[calm_code_cursor]
	mov	eax,[ebx]
	stosd
	mov	[calm_code_cursor],edi
	jmp	prepare_stack_for_logical_operator
      push_logical_operator:
	call	push_to_condition_stack
	jmp	parse_alm_condition
    close_subconditions:
	mov	ebx,[condition_stack]
	sub	ebx,4
	cmp	dword [ebx],0
	jl	store_logical_operator
	je	condition_malformed
	dec	dword [ebx]
	jnz	parse_alm_logical_operator
	mov	[condition_stack],ebx
	jmp	parse_alm_logical_operator
      store_logical_operator:
	mov	edi,[calm_code_cursor]
	mov	eax,[ebx]
	mov	[condition_stack],ebx
	stosd
	mov	[calm_code_cursor],edi
	jmp	close_subconditions
    push_to_condition_stack:
	mov	ebx,[condition_stack]
	mov	[ebx],ecx
	add	ebx,4
	cmp	[condition_stack_end],ebx
	je	grow_condition_stack
	mov	[condition_stack],ebx
	retn
      grow_condition_stack:
	mov	eax,[condition_stack_base]
	sub	ebx,eax
	lea	ecx,[ebx+4]
	call	grow_stack
	mov	[condition_stack_base],eax
	add	ebx,eax
	mov	[condition_stack],ebx
	add	ecx,eax
	mov	[condition_stack_end],ecx
	retn
    calm_condition_parsed:
	mov	edi,[calm_code_cursor]
	mov	ebx,[condition_stack]
	mov	ecx,ebx
	sub	ecx,[condition_stack_base]
	mov	edx,calm_code_buffer
	call	reserve_workspace
    finish_calm_condition:
	sub	ebx,4
	mov	eax,[ebx]
	cmp	eax,0
	jg	condition_malformed
	stosd
	jne	finish_calm_condition
	mov	[calm_code_cursor],edi
	and	[use_raw_values],0
	jmp	instruction_assembled
    condition_malformed:
	and	[use_raw_values],0
	mov	edi,[calm_rollback_offset]
	add	edi,[calm_code_buffer.memory_start]
	mov	[calm_code_cursor],edi
	mov	edx,_invalid_expression
	call	register_error
	jmp	assembly_line

alm_publish:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	mov	[value],calm_publish_variable
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,':'
	jne	get_identifier_variable
	inc	esi
	mov	[value],calm_publish_stack
    get_identifier_variable:
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	mov	[label_leaf],ebx
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,':'
	jne	get_value_variable
	mov	eax,calm_publish_constant
	xchg	[value],eax
	cmp	eax,calm_publish_variable
	jne	invalid_argument
	inc	esi
	call	move_to_next_symbol
	jc	missing_argument
    get_value_variable:
	cmp	al,','
	jne	missing_argument
	inc	esi
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+8
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,[value]
	stosd
	mov	eax,[label_leaf]
	stosd
	mov	eax,ebx
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled

alm_transform:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	call	move_to_next_symbol
	jc	missing_argument
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	mov	[label_leaf],ebx
	xor	ebx,ebx
	call	move_to_next_symbol
	jc	alm_transform_arguments_ready
	cmp	al,','
	jne	alm_transform_arguments_ready
	inc	esi
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	edx,edx
	jz	invalid_identifier
	call	get_symbol_namespace
    alm_transform_arguments_ready:
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+8
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_transform
	stosd
	mov	eax,[label_leaf]
	stosd
	mov	eax,ebx
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled

alm_take:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	call	move_to_next_symbol
	jc	missing_argument
	xor	ebx,ebx
	and	[symbol_definition],0
	cmp	al,','
	je	alm_take_destination_ready
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,','
	jne	missing_argument
    alm_take_destination_ready:
	mov	[label_leaf],ebx
	inc	esi
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_identifier
	test	ebx,ebx
	jz	invalid_identifier
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8+8
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_take
	stosd
	mov	eax,[label_leaf]
	stosd
	mov	eax,ebx
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled

alm_exit:
	test	[assembly_mode],AMODE_CALM_DEFINITION
	jz	unexpected_instruction
	mov	edi,[calm_code_cursor]
	mov	edx,calm_code_buffer
	mov	ecx,8
	call	reserve_workspace
	mov	eax,[calm_line_number]
	stosd
	mov	eax,calm_end
	stosd
	mov	[calm_code_cursor],edi
	jmp	instruction_assembled

launch_calm:
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	mov	eax,[ebx+SymbolTree_Leaf.branch]
	mov	[instruction_branch],eax
	mov	[calm_value],edx
	mov	[calm_source_pointer],esi
	mov	esi,[edx+ValueDefinition.value]
	mov	eax,[esi+CompiledMacroHeader.literals_offset]
	add	eax,esi
	mov	[calm_literals],eax
	mov	[breakpoint_token],0
	mov	ebx,[esi+CompiledMacroHeader.label_argument_leaf]
	test	ebx,ebx
	jz	calm_label_argument_ok
	call	update_value_definition
	test	edx,edx
	jz	calm_label_argument_ok
	push	esi
	push	edx
	push	[line_end]
	push	[embedded_context]
	mov	eax,[line_context]
	mov	[embedded_context],eax
	mov	esi,[line_start]
	mov	ecx,[label_instruction_start]
	mov	[line_end],ecx
	sub	ecx,esi
	add	ecx,1+sizeof.RecognitionContext
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	call	reserve_workspace
	xor	edx,edx
	mov	[breakpoint_token],dl
	call	extract_piece_of_line
	pop	[embedded_context]
	pop	[line_end]
	pop	edx
	mov	[value_type],VALTYPE_SYMBOLIC
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	call	assign_value
	pop	esi
    calm_label_argument_ok:
	add	esi,sizeof.CompiledMacroHeader
	mov	eax,[esi]
	inc	eax
	cmp	eax,1
	jbe	calm_arguments_defined
    get_calm_argument:
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	mov	ecx,[line_end]
	sub	ecx,[calm_source_pointer]
	add	ecx,1+sizeof.RecognitionContext
	call	reserve_workspace
	mov	eax,[esi+sizeof.CompiledMacroArgument]
	xchg	esi,[calm_source_pointer]
	inc	eax
	jz	get_calm_greedy_argument
	call	extract_argument_value
	jmp	calm_argument_value_cut
    get_calm_greedy_argument:
	xor	edx,edx
	call	extract_piece_of_line
    calm_argument_value_cut:
	xchg	esi,[calm_source_pointer]
	mov	ebx,[esi+CompiledMacroArgument.symbol_leaf]
	call	update_value_definition
	test	edx,edx
	jz	calm_argument_ok
	mov	ecx,edi
	sub	ecx,[assembly_workspace.memory_start]
	test	ecx,ecx
	jnz	calm_argument_value_ready
	or	ecx,[esi+CompiledMacroArgument.default_value_length]
	jz	calm_argument_value_ready
	cmp	ecx,-1
	je	missing_argument
	push	esi
	mov	esi,[esi+CompiledMacroArgument.default_value_offset]
	add	esi,[calm_literals]
	jmp	calm_assign_argument_value
    calm_argument_value_ready:
	push	esi
	mov	esi,[assembly_workspace.memory_start]
    calm_assign_argument_value:
	mov	[value_type],VALTYPE_SYMBOLIC
	call	assign_value
	pop	esi
    calm_argument_ok:
	add	esi,sizeof.CompiledMacroArgument
	mov	eax,[esi]
	inc	eax
	cmp	eax,1
	jbe	calm_arguments_defined
	xchg	esi,[calm_source_pointer]
	call	move_to_next_symbol
	jc	calm_next_argument
	cmp	al,','
	jne	invalid_argument
	inc	esi
    calm_next_argument:
	xchg	esi,[calm_source_pointer]
	jmp	get_calm_argument
    calm_arguments_defined:
	add	esi,4
	and	[calm_instruction_number],0
	call	create_source_entry
	jc	calm_exceeded_stack_limit
	mov	edx,[calm_value]
	mov	[ebx+SourceEntry.type],SOURCE_CALM
	mov	[ebx+SourceEntry.text],edx
	sub	esi,[edx+ValueDefinition.value]
	mov	[ebx+SourceEntry.offset],esi
	or	[edx+ValueDefinition.flags],VAL_IN_USE
	inc	[edx+ValueDefinition.reference_count]
	mov	eax,[parameter_namespace]
	mov	[ebx+SourceEntry.local_namespace],eax
	mov	esi,[calm_source_pointer]
	mov	ecx,[instruction_branch]
	test	ecx,ecx
	jz	instruction_assembled
	mov	[ebx+SourceEntry.name],ecx
	or	[ebx+SourceEntry.name_length],-1
	jmp	instruction_assembled
    calm_exceeded_stack_limit:
	mov	edx,_stack_limit_exceeded
	call	register_error
	jmp	assembly_line

calm_virtual_machine:
	mov	al,[ebx+SourceEntry.saved_result]
	mov	[calm_result],al
	mov	edx,[ebx+SourceEntry.text]
	mov	[calm_value],edx
	mov	esi,[edx+ValueDefinition.value]
	mov	eax,[esi+CompiledMacroHeader.literals_offset]
	add	eax,esi
	mov	[calm_literals],eax
	add	esi,[ebx+SourceEntry.offset]
    calm_execution_unit:
	lodsd
	mov	[calm_instruction_number],eax
	lodsd
	jmp	eax

calm_end:
	mov	ebx,[source_context]
	mov	ecx,[ebx+SourceContext.number_of_entries]
	dec	ecx
       ; jz	 internal_error
	mov	[ebx+SourceContext.number_of_entries],ecx
	imul	ecx,sizeof.SourceEntry
	mov	edx,[ebx+sizeof.SourceContext+ecx+SourceEntry.text]
	and	[edx+ValueDefinition.flags],not VAL_IN_USE
	dec	[edx+ValueDefinition.reference_count]
	jmp	assembly_line

calm_arrange:
	lodsd
	mov	[label_leaf],eax
	lodsd
	mov	ecx,eax
	lodsd
	push	esi
	mov	esi,[calm_literals]
	add	eax,esi
	add	esi,ecx
	mov	[pattern_end],eax
	mov	edi,[assembly_workspace.memory_start]
    arrange_by_pattern:
	mov	edx,assembly_workspace
	mov	ecx,[pattern_end]
	sub	ecx,esi
	call	reserve_workspace
    arrange_token:
	cmp	esi,[pattern_end]
	je	assign_arranged_value
	lodsb
	cmp	al,1Bh
	je	arrange_by_reference
	cmp	al,0A0h
	je	downgrade_hard_space
	stosb
	cmp	al,1Ah
	je	copy_arranged_token_data
	cmp	al,22h
	je	copy_arranged_token_data
	cmp	al,27h
	je	copy_arranged_token_data
	cmp	al,30h
	jne	arrange_token
	lodsd
	stosd
	mov	ecx,eax
	rep	movsb
	jmp	arrange_token
    copy_arranged_token_data:
	movsd
	jmp	arrange_token
    downgrade_hard_space:
	mov	al,20h
	stosb
	jmp	arrange_token
    arrange_by_reference:
	mov	ebx,[esi+4]
	call	use_available_value
	jc	arrange_missing_value
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_NUMERIC
	je	arrange_numeric_value
	cmp	al,VALTYPE_SYMBOLIC
	jne	arrange_unsupported_value
	mov	ecx,[edx+ValueDefinition.value_length]
	jecxz	arrange_empty_value
	add	ecx,1+sizeof.RecognitionContext
	mov	ebx,edx
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	edx,ebx
	mov	ebx,esi
	mov	esi,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	rep	movsb
	lea	esi,[ebx+8]
	cmp	esi,[pattern_end]
	je	assign_arranged_value
	mov	al,40h
	stosb
	xor	eax,eax
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	stosd
	jmp	arrange_by_pattern
    arrange_numeric_value:
	mov	eax,[edx+ValueDefinition.value]
	mov	ecx,[eax]
	cmp	dword [eax+4+ecx],0
	jne	arrange_unsupported_value
	test	byte [eax+4+ecx-1],80h
	jnz	arrange_unsupported_value
	mov	ebx,esi
	mov	esi,eax
	mov	edx,assembly_workspace
	add	ecx,1+4
	call	reserve_workspace
	mov	al,30h
	stosb
	lodsd
	stosd
	mov	ecx,eax
	rep	movsb
	lea	esi,[ebx+8]
	jmp	arrange_by_pattern
    arrange_empty_value:
	add	esi,8
	jmp	arrange_token
    arrange_unsupported_value:
	mov	edx,_invalid_symbol_value
	jmp	arrange_failed_reference
    arrange_missing_value:
	mov	edx,_undefined_symbol
    arrange_failed_reference:
	call	register_error
	mov	al,1Ah
	stosb
	movsd
	lodsd
	mov	eax,[eax+SymbolTree_Leaf.branch]
	cmp	[eax+SymbolTree_Foliage.name_kind],NAME_CASEINSENSITIVE
	jne	arrange_token
	mov	al,'?'
	stosb
	jmp	arrange_token
    assign_arranged_value:
	mov	ebx,[label_leaf]
	call	update_value_definition
	test	edx,edx
	jz	arranged_value_assigned
	mov	[value_type],VALTYPE_SYMBOLIC
	mov	ecx,edi
	mov	esi,[assembly_workspace.memory_start]
	sub	ecx,esi
	call	assign_value
    arranged_value_assigned:
	pop	esi
	jmp	calm_execution_unit

calm_match:
	and	[calm_result],0
	lodsd
	mov	ebx,eax
	lodsd
	mov	ecx,eax
	lodsd
	mov	edi,[calm_literals]
	add	eax,edi
	add	edi,ecx
	mov	[pattern_end],eax
	lodsd
	mov	[brackets],eax
	call	use_available_value
	jc	calm_undefined_symbol
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_RESERVED
	je	calm_undefined_symbol
	cmp	al,VALTYPE_SYMBOLIC
	jne	calm_invalid_value
	push	esi
	mov	[value],edx
	inc	[edx+ValueDefinition.reference_count]
	mov	esi,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	add	ecx,esi
	mov	[line_end],ecx
	mov	ebx,[expression_workspace.memory_start]
	and	[matched_context],0
	and	[stored_position],0
    calm_match_with_pattern:
	cmp	edi,[pattern_end]
	je	calm_end_of_pattern
	mov	ah,[edi]
	cmp	ah,1Bh
	jne	calm_exact_match
    calm_wildcard_match:
	and	[stored_position],0
	add	edi,1+8
	cmp	edi,[pattern_end]
	je	calm_required_wildcard_match
	cmp	byte [edi],'?'
	je	calm_optional_wildcard_match
    calm_required_wildcard_match:
	cmp	esi,[line_end]
	je	calm_match_done
	call	calm_consume_whitespace
	jz	calm_match_done
	xchg	edi,ebx
	mov	edx,expression_workspace
	mov	ecx,sizeof.MatchedExcerpt
	call	reserve_workspace
	xchg	edi,ebx
	mov	eax,[edi-4]
	mov	ecx,[matched_context]
	mov	[ebx+MatchedExcerpt.symbol_leaf],eax
	mov	[ebx+MatchedExcerpt.recognition_context],ecx
	mov	[ebx+MatchedExcerpt.data_start],esi
	mov	al,[esi]
	call	calm_consume_token
	jc	calm_match_done
	mov	[ebx+MatchedExcerpt.data_end],esi
	add	ebx,sizeof.MatchedExcerpt
	jmp	calm_match_with_pattern
    calm_optional_wildcard_match:
	xchg	edi,ebx
	mov	edx,expression_workspace
	mov	ecx,sizeof.MatchedExcerpt
	call	reserve_workspace
	xchg	edi,ebx
	mov	eax,[edi-4]
	inc	edi
	mov	ecx,[matched_context]
	mov	[ebx+MatchedExcerpt.symbol_leaf],eax
	mov	[ebx+MatchedExcerpt.recognition_context],ecx
	mov	[ebx+MatchedExcerpt.data_start],esi
	mov	[ebx+MatchedExcerpt.data_end],esi
	add	ebx,sizeof.MatchedExcerpt
	jmp	calm_match_with_pattern
    calm_exact_match:
	cmp	esi,[line_end]
	je	calm_end_of_text
	mov	al,[esi]
	cmp	al,40h
	jne	found_token_to_match
	inc	esi
	mov	[matched_context],esi
	add	esi,sizeof.RecognitionContext
	jmp	calm_exact_match
      found_token_to_match:
	cmp	[stored_position],0
	jne	calm_match_position_stored
	mov	[stored_position],esi
	mov	ecx,[matched_context]
	mov	[stored_context],ecx
	mov	[stored_pattern],edi
      calm_match_position_stored:
	cmp	al,20h
	je	calm_match_whitespace
	cmp	ah,20h
	je	calm_skip_pattern_whitespace
	cmpsb
	jne	calm_token_mismatch
	cmp	ah,1Ah
	je	calm_match_name_tokens
	cmp	ah,22h
	je	calm_match_string_tokens
	cmp	ah,27h
	je	calm_match_string_tokens
	cmp	ah,30h
	jne	calm_match_with_pattern
	mov	ecx,esi
	mov	edx,edi
	lodsd
	add	esi,eax
	mov	eax,[edi]
	lea	edi,[edi+4+eax]
	cmp	byte [edi],'?'
	jne	calm_compare_token_contents
	inc	edi
	jmp	calm_compare_token_contents
      calm_match_string_tokens:
	lodsd
	mov	ecx,eax
	mov	edx,[edi]
	add	edi,4
      calm_compare_token_contents:
	cmp	ecx,edx
	je	calm_match_with_pattern
	push	esi edi
	mov	esi,ecx
	mov	edi,edx
	mov	ecx,[esi]
	add	ecx,4
	repe	cmpsb
	pop	edi esi
	jne	calm_retract_match
	jmp	calm_match_with_pattern
      calm_match_name_tokens:
	lodsd
	mov	ecx,eax
	mov	edx,[edi]
	add	edi,4
	cmp	byte [edi],'?'
	je	calm_case_insensitive_match
	cmp	ecx,edx
	je	calm_match_with_pattern
	push	esi edi
	mov	esi,ecx
	mov	edi,edx
	lodsd
	scasd
	jne	calm_name_mismatch
	mov	ecx,eax
	mov	eax,[esi+ecx]
	cmp	eax,[edi+ecx]
	jne	calm_name_mismatch
	repe	cmpsb
	jne	calm_name_mismatch
	pop	edi esi
	jmp	calm_match_with_pattern
      calm_name_mismatch:
	pop	edi esi
	jmp	calm_retract_match
      calm_case_insensitive_match:
	inc	edi
	cmp	ecx,edx
	je	calm_match_with_pattern
	push	esi edi
	mov	esi,ecx
	mov	edi,edx
	lodsd
	scasd
	jne	calm_name_mismatch
	mov	ecx,eax
	mov	eax,[esi+ecx+4]
	cmp	eax,[edi+ecx+4]
	jne	calm_name_mismatch
	xor	eax,eax
      calm_compare_case_insensitively:
	lodsb
	mov	dl,[characters+eax]
	mov	al,[edi]
	inc	edi
	cmp	dl,[characters+eax]
	jne	name_mismatch
	loop	calm_compare_case_insensitively
	pop	edi esi
	jmp	calm_match_with_pattern
      calm_match_converted_number:
	call	convert_number_to_match
	jmp	calm_compare_token_contents
      calm_match_with_converted_number:
	xchg	esi,edi
	call	convert_number_to_match
	xchg	esi,edi
	jmp	calm_compare_token_contents
      calm_match_whitespace:
	cmp	ah,20h
	je	calm_optional_whitespace
	cmp	ah,0A0h
	je	calm_required_whitespace
	cmp	[stored_pattern],edi
	jne	calm_retract_match
	call	calm_consume_whitespace
	jz	calm_match_done
	and	[stored_position],0
	jmp	calm_match_with_pattern
      calm_optional_whitespace:
	inc	edi
	cmp	edi,[pattern_end]
	je	calm_whitespace_matched
	cmp	byte [edi],0A0h
	jne	calm_whitespace_matched
      calm_required_whitespace:
	inc	edi
	cmp	edi,[pattern_end]
	je	calm_whitespace_matched
	cmp	byte [edi],20h
	jne	calm_whitespace_matched
	inc	edi
      calm_whitespace_matched:
	call	calm_consume_whitespace
	jz	calm_end_of_text
	jmp	calm_match_with_pattern
      calm_skip_pattern_whitespace:
	inc	edi
	jmp	calm_match_with_pattern
      calm_token_mismatch:
	cmp	ax,1A30h
	je	calm_match_converted_number
	cmp	ax,301Ah
	je	calm_match_with_converted_number
    calm_retract_match:
	xor	esi,esi
	xchg	esi,[stored_position]
	mov	ecx,[stored_context]
	mov	[matched_context],ecx
	mov	edi,[stored_pattern]
	call	calm_consume_whitespace
	jz	calm_match_done
    calm_expand_wildcard_match:
	cmp	ebx,[expression_workspace.memory_start]
	je	calm_match_done
	call	calm_consume_token
	jc	calm_match_done
	mov	[ebx-sizeof.MatchedExcerpt+MatchedExcerpt.data_end],esi
	jmp	calm_match_with_pattern
    calm_consume_whitespace:
	mov	al,[esi]
	cmp	al,40h
	je	calm_consume_context_token
	cmp	al,20h
	jne	calm_whitespace_consumed
	inc	esi
	cmp	esi,[line_end]
	jne	calm_consume_whitespace
      calm_whitespace_consumed:
	retn
      calm_consume_context_token:
	inc	esi
	mov	[matched_context],esi
	add	esi,sizeof.RecognitionContext
	cmp	esi,[line_end]
	jne	calm_consume_whitespace
	retn
    calm_consume_token:
	xor	ecx,ecx
	mov	edx,[brackets]
	inc	esi
      consume_token_content:
	cmp	al,dh
	je	consume_closing
	cmp	al,dl
	je	consume_opening
	cmp	al,1Ah
	je	consume_token_with_data
	cmp	al,22h
	je	consume_token_with_data
	cmp	al,27h
	je	consume_token_with_data
	cmp	al,30h
	je	consume_internal_token
	cmp	al,40h
	je	consume_context_token
	test	ecx,ecx
	jnz	consume_more
	retn
      consume_token_with_data:
	add	esi,4
	test	ecx,ecx
	jnz	consume_more
	retn
      consume_internal_token:
	lodsd
	add	esi,eax
	test	ecx,ecx
	jnz	consume_more
	retn
      consume_context_token:
	add	esi,sizeof.RecognitionContext
	test	ecx,ecx
	jnz	consume_more
	retn
      consume_closing:
	sub	ecx,1
	jg	consume_more
	retn
      consume_opening:
	inc	ecx
      consume_more:
	cmp	esi,[line_end]
	je	consume_unsatisfied
	lodsb
	jmp	consume_token_content
      consume_unsatisfied:
	stc
	retn
    calm_end_of_pattern:
	cmp	esi,[line_end]
	je	calm_match_found
	lodsb
	cmp	al,20h
	je	calm_end_of_pattern
	cmp	al,40h
	jne	calm_another_token_to_match
	add	esi,sizeof.RecognitionContext
	jmp	calm_end_of_pattern
      calm_another_token_to_match:
	dec	esi
	cmp	[stored_position],0
	je	calm_expand_wildcard_match
	jmp	calm_retract_match
    calm_end_of_text:
	cmp	edi,[pattern_end]
	je	calm_match_found
	mov	ah,[edi]
	cmp	ah,1Bh
	je	calm_wildcard_match
	cmp	ah,20h
	jne	calm_match_done
	inc	edi
	jmp	calm_end_of_text
    calm_match_found:
	or	[calm_result],1
	mov	esi,ebx
	mov	[value_type],VALTYPE_SYMBOLIC
      calm_assign_matched_values:
	cmp	esi,[expression_workspace.memory_start]
	je	calm_match_done
	sub	esi,sizeof.MatchedExcerpt
	mov	ebx,[esi+MatchedExcerpt.symbol_leaf]
	call	update_value_definition
	test	edx,edx
	jz	calm_assign_matched_values
	push	esi
	mov	eax,[esi+MatchedExcerpt.recognition_context]
	mov	ecx,[esi+MatchedExcerpt.data_end]
	mov	esi,[esi+MatchedExcerpt.data_start]
	sub	ecx,esi
	jz	calm_matched_value_ready
	test	eax,eax
	jz	calm_matched_value_ready
	cmp	dword [eax],0
	je	calm_matched_value_ready
	push	esi ecx edx
	mov	esi,eax
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	ecx,1+sizeof.RecognitionContext
	call	reserve_workspace
	mov	al,40h
	stosb
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	pop	edx ecx esi
	rep	movsb
	mov	ecx,edi
	mov	esi,[assembly_workspace.memory_start]
	sub	ecx,esi
      calm_matched_value_ready:
	call	assign_value
	pop	esi
	jmp	calm_assign_matched_values
    calm_match_done:
	mov	edx,[value]
	dec	[edx+ValueDefinition.reference_count]
	pop	esi
	jmp	calm_execution_unit

calm_compute:
	lodsd
	mov	[label_leaf],eax
	lodsd
	add	eax,esi
	push	eax
	mov	edi,[calculation_workspace.memory_start]
	and	[value_position],0
	call	calculate_parsed_expression
	call	pop_terms
	jc	calm_compute_done
	mov	al,byte [edi+ExpressionTerm.attributes]
	cmp	al,EXPR_STRING
	je	calm_computed_string
	cmp	al,EXPR_FLOAT
	je	calm_computed_float
	call	convert_terms_to_numeric_value
	mov	[value_type],VALTYPE_NUMERIC
    calm_computed_value_ready:
	mov	edi,ecx
	mov	ebx,[label_leaf]
	call	update_value_definition
	test	edx,edx
	jz	calm_compute_done
	mov	ecx,edi
	call	assign_value
    calm_compute_done:
	pop	esi
	jmp	calm_execution_unit
    calm_computed_string:
	call	get_term_value
	mov	esi,edx
	mov	ecx,[esi]
	add	ecx,4
	mov	[value_type],VALTYPE_STRING
	jmp	calm_computed_value_ready
    calm_computed_float:
	call	get_term_value
	mov	esi,edx
	mov	ecx,sizeof.FloatData
	mov	[value_type],VALTYPE_FLOAT
	jmp	calm_computed_value_ready

calm_check:
	lodsd
	test	eax,eax
	jz	calm_execution_unit
	js	calm_check
    calm_evaluate_logical_value:
	call	evaluate_stored_logical_value
	mov	[calm_result],al
	add	esi,[esi-4]
    find_next_logical_value:
	lodsd
	test	eax,eax
	jz	calm_execution_unit
	jns	next_logical_value_found
	cmp	eax,BOOL_NEG
	jne	find_next_logical_value
	xor	[calm_result],1
	jmp	find_next_logical_value
    next_logical_value_found:
	mov	ebx,esi
	xor	ecx,ecx
    skip_next_logical_value:
	inc	ecx
	add	esi,eax
    skip_logical_operator:
	lodsd
	assert	BOOL_NEG = -1
	cmp	eax,BOOL_NEG
	jg	skip_next_logical_value
	je	skip_logical_operator
	dec	ecx
	jnz	skip_logical_operator
	cmp	eax,BOOL_AND
	je	calm_and
    calm_or:
	cmp	[calm_result],0
	jne	find_next_logical_value
	mov	esi,ebx
	jmp	calm_evaluate_logical_value
    calm_and:
	cmp	[calm_result],0
	je	find_next_logical_value
	mov	esi,ebx
	jmp	calm_evaluate_logical_value

calm_jyes:
	cmp	[calm_result],0
	jne	calm_jump
	add	esi,4
	jmp	calm_execution_unit
calm_jno:
	cmp	[calm_result],0
	je	calm_jump
	add	esi,4
	jmp	calm_execution_unit
calm_jump:
	lodsd
	mov	edx,[calm_value]
	add	eax,[edx+ValueDefinition.value]
	mov	esi,eax
	jmp	calm_execution_unit

calm_assemble:
	lodsd
	mov	ebx,eax
	mov	eax,[ebx+SymbolTree_Leaf.branch]
	mov	[instruction_branch],eax
	call	use_available_value
	jc	calm_undefined_symbol
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	jne	calm_invalid_value
	call	create_source_entry
	jc	calm_exceeded_stack_limit
	mov	[ebx+SourceEntry.type],SOURCE_MACRO
	or	[ebx+SourceEntry.flags],SRCF_PREPROCESSED
	mov	[ebx+SourceEntry.text],edx
	or	[edx+ValueDefinition.flags],VAL_IN_USE
	inc	[edx+ValueDefinition.reference_count]
	mov	eax,[calm_instruction_number]
	mov	[ebx-sizeof.SourceEntry+SourceEntry.line_number],eax
	mov	edx,[ebx-sizeof.SourceEntry+SourceEntry.text]
	mov	ecx,[edx+ValueDefinition.value]
	sub	esi,ecx
	mov	[ebx-sizeof.SourceEntry+SourceEntry.offset],esi
	mov	eax,[ebx-sizeof.SourceEntry+SourceEntry.local_namespace]
	mov	[ebx+SourceEntry.local_namespace],eax
	mov	al,[calm_result]
	mov	[ebx-sizeof.SourceEntry+SourceEntry.saved_result],al
	mov	ecx,[instruction_branch]
	test	ecx,ecx
	jz	assembly_line
	mov	[ebx+SourceEntry.name],ecx
	or	[ebx+SourceEntry.name_length],-1
	jmp	assembly_line
    calm_undefined_symbol:
	mov	edx,_undefined_symbol
	call	register_error
	jmp	calm_execution_unit
    calm_invalid_value:
	mov	edx,_invalid_symbol_value
	call	register_error
	jmp	calm_execution_unit

calm_transform:
	mov	[calm_result],0
	lodsd
	mov	[label_leaf],eax
	mov	ebx,eax
	lodsd
	mov	[transforming_namespace],eax
	call	get_available_value
	jc	calm_undefined_symbol
	cmp	[edx+ValueDefinition.type],VALTYPE_SYMBOLIC
	jne	calm_invalid_value
	push	esi
	call	clear_line_embeddings
	mov	esi,[edx+ValueDefinition.value]
	mov	[line_start],esi
	xor	eax,eax
	mov	[embedded_context],eax
	mov	[line_context],eax
	mov	ecx,[edx+ValueDefinition.value_length]
	add	ecx,esi
	mov	[line_end],ecx
	mov	edi,[assembly_workspace.memory_start]
	mov	[hidden_context],0
    transform_symbolic_value:
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	push	edi
	mov	eax,[embedded_context]
	mov	[line_context],eax
	mov	ebx,[transforming_namespace]
	test	ebx,ebx
	jz	transform_identify
	call	identify_symbol_in_namespace
	jmp	transform_identification_done
    transform_identify:
	call	identify_symbol
    transform_identification_done:
	jc	symbolic_value_transformed
	test	edi,edi
	jnz	ready_to_transform
	call	skip_literal
	xor	ebx,ebx
    ready_to_transform:
	pop	edi
	test	ebx,ebx
	jz	untransformed_literal
	mov	[further_whitespace],ecx
	call	get_available_value
	jc	untransformed_literal
	cmp	[edx+ValueDefinition.type],VALTYPE_SYMBOLIC
	jne	untransformed_literal
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	mov	[calm_result],1
	mov	[line_start],esi
	mov	esi,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.value_length]
	mov	ebx,ecx
	add	ecx,[further_whitespace]
	add	ecx,1+sizeof.RecognitionContext
	mov	edx,assembly_workspace
	call	reserve_workspace
	test	ebx,ebx
	jz	symbol_transformed
	mov	al,1
	xchg	al,[hidden_context]
	test	al,al
	jnz	reset_hidden_context
	cmp	[line_context],0
	je	copy_transformed_value
    reset_hidden_context:
	xor	edx,edx
	call	reset_transformed_context
    copy_transformed_value:
	mov	ecx,ebx
	rep	movsb
    symbol_transformed:
	mov	ecx,[further_whitespace]
	mov	al,20h
	rep	stosb
	mov	esi,[line_start]
	jmp	transform_symbolic_value
    untransformed_literal:
	mov	ecx,esi
	xchg	esi,[line_start]
	sub	ecx,esi
	mov	ebx,ecx
	add	ecx,1+sizeof.RecognitionContext
	mov	edx,assembly_workspace
	call	reserve_workspace
	xor	al,al
	xchg	al,[hidden_context]
	test	al,al
	jz	copy_untransformed_literal
	mov	edx,[line_context]
	call	reset_transformed_context
    copy_untransformed_literal:
	mov	ecx,ebx
	rep	movsb
	jmp	transform_symbolic_value
    reset_transformed_context:
	mov	al,40h
	cmp	[esi],al
	je	transformed_context_ok
	stosb
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	test	edx,edx
	jz	clear_transformed_context
	xchg	esi,edx
	rep	movsd
	mov	esi,edx
      transformed_context_ok:
	retn
      clear_transformed_context:
	xor	eax,eax
	rep	stosd
	retn
    symbolic_value_transformed:
	pop	edi
	cmp	[calm_result],0
	je	calm_transform_done
	mov	ebx,[label_leaf]
	call	update_value_definition
	test	edx,edx
	jz	calm_transform_done
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	mov	[value_type],VALTYPE_SYMBOLIC
	call	assign_value
    calm_transform_done:
	pop	esi
	jmp	calm_execution_unit

calm_stringify:
	lodsd
	mov	[label_leaf],eax
	mov	ebx,eax
	call	get_available_value
	jc	calm_undefined_symbol
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	jne	calm_invalid_value
	mov	eax,[edx+ValueDefinition.value]
	mov	[symbol_value_start],eax
	mov	ecx,[edx+ValueDefinition.value_length]
	add	ecx,eax
	mov	[symbol_value_end],ecx
	push	esi
	call	convert_symbolic_value_to_string
	mov	edi,ecx
	mov	ebx,[label_leaf]
	call	update_value_definition
	test	edx,edx
	jz	calm_stringify_done
	mov	ecx,edi
	mov	[value_type],VALTYPE_STRING
	call	assign_value
    calm_stringify_done:
	pop	esi
	jmp	calm_execution_unit

calm_publish_constant:
	mov	edi,create_constant_value_definition
	jmp	calm_publish
calm_publish_stack:
	mov	edi,create_value_definition
	jmp	calm_publish
calm_publish_variable:
	mov	edi,update_value_definition
    calm_publish:
	lodsd
	mov	ebx,eax
	lodsd
	mov	[label_leaf],eax
	call	use_available_value
	jc	calm_undefined_symbol
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	jne	calm_invalid_value
	push	esi edi
	call	clear_line_embeddings
	mov	esi,[edx+ValueDefinition.value]
	mov	[line_start],esi
	mov	ecx,[edx+ValueDefinition.value_length]
	add	ecx,esi
	mov	[line_end],ecx
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	mov	eax,esi
	pop	edi esi
	jc	calm_invalid_identifier
	test	ebx,ebx
	jz	calm_invalid_identifier
	cmp	eax,[line_end]
	jne	calm_invalid_identifier
	xchg	ebx,[label_leaf]
	call	use_available_value
	jc	calm_undefined_symbol
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	jb	calm_invalid_value
	cmp	al,VALTYPE_AREA
	ja	calm_invalid_value
	mov	[value_type],al
	mov	[value],edx
	mov	ebx,[label_leaf]
	call	edi
	test	edx,edx
	jz	calm_execution_unit
	push	esi
	mov	eax,[value]
	mov	esi,[eax+ValueDefinition.value]
	mov	ecx,[eax+ValueDefinition.value_length]
	call	assign_value
	pop	esi
	jmp	calm_execution_unit
    calm_invalid_identifier:
	mov	edx,_invalid_identifier
	call	register_error
	jmp	calm_execution_unit

calm_take:
	lodsd
	mov	edi,eax
	lodsd
	mov	ebx,eax
	xor	edx,edx
	call	remove_value_definition
	setnc	[calm_result]
	jmp	calm_execution_unit
