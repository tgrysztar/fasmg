
struct DirectiveBlock
	type db ?			; DBLOCK_#
	subtype db ?			; CTRL_# for DBLOCK_CONTROL
	flags db ?			; CTRLF_# for DBLOCK_CONTROL
	prior_assembly_mode db ?
	prior_special_parameters db ?
	prior_counter_position dd ?
	parameter_namespace dd ?
	parameter dd ?			; pointer to first BlockParameter
	length_of_data dd ?
ends

struct BlockParameter
	symbol dd ?			; pointer to SymbolTree_Leaf
	definition dd ? 		; pointer to ValueDefinition
	next dd ?			; pointer to another BlockParameter
ends

struct NamespaceData
	prior_namespace dd ?
	prior_label dd ?
ends

struct VirtualBlockData
	outer_area dd ?
	shift_tracking db ?
ends

struct ConditionalRepeatData
	condition_length dd ?
ends

struct RepeatData
	limit_length dd ?
	index_position dd ?
ends

struct MacroData
	nesting_counter dd ?
ends

struct SymbolSubstitution
	symbol_start dd ?
	symbol_end dd ?
	value_start dd ?
	value_end dd ?
	leftover_context dd ?
ends

struct MatchedParameter
	pattern dd ?
	assembly_position dd ?
ends

DBLOCK_CLOSED = 0
DBLOCK_NAMESPACE = 1
DBLOCK_CONTROL = 2
DBLOCK_MACRO = 3
DBLOCK_VIRTUAL = 4
DBLOCK_CALMINSTRUCTION = 5
DBLOCK_POSTPONED = 8
DBLOCK_SUSPENDED = 9

CTRL_IF = 0
CTRL_WHILE = 2
CTRL_REPEAT = 3
CTRL_IRP = 4
CTRL_IRPV = 5
CTRL_MATCH = 6
CTRL_RMATCH = 7
CTRL_POSTPONE = 9

CTRLF_INACTIVE = 1
CTRLF_BREAKABLE = 2
CTRLF_HAS_REPEAT_DATA = 4
CTRLF_HAS_WRITABLE_INDEX = 8
CTRLF_ALLOWS_ELSE = 16
CTRLF_SUSPENDED = 32

add_directive_block:
; in:
;  dl = DBLOCK_#
;  ecx = length of additional data
; out: edi - new DirectiveBlock
; note: area for additional data is placed below the DirectiveBlock structure
; preserves: ebx, edx, esi
	push	esi ecx
	mov	esi,[source_context]
	mov	eax,[esi+SourceContext.number_of_entries]
	imul	eax,sizeof.SourceEntry
	add	eax,sizeof.SourceContext
	add	ecx,eax
	add	ecx,sizeof.DirectiveBlock
	mov	edi,[directives_stack]
	lea	eax,[edi+ecx]
	cmp	eax,[directives_stack_end]
	jbe	directives_stack_ready
	add	ecx,[directives_stack_end]
	jc	out_of_memory
	mov	eax,[directives_stack_base]
	sub	ecx,eax
	push	edx
	call	grow_stack
	pop	edx
	add	ecx,eax
	mov	[directives_stack_end],ecx
	mov	edi,eax
	xchg	eax,[directives_stack_base]
	sub	edi,eax
	add	edi,[directives_stack]
	mov	[directives_stack],edi
    directives_stack_ready:
	push	edx
	call	clone_source_context
	pop	edx ecx
	add	edi,ecx
	mov	eax,edi
	sub	eax,[directives_stack]
	mov	[edi+DirectiveBlock.length_of_data],eax
	mov	[edi+DirectiveBlock.type],dl
	mov	al,[assembly_mode]
	mov	[edi+DirectiveBlock.prior_assembly_mode],al
	mov	ecx,[parameter_namespace]
	mov	[edi+DirectiveBlock.parameter_namespace],ecx
	mov	al,[ecx+SymbolTree_Root.parameters]
	mov	[edi+DirectiveBlock.prior_special_parameters],al
	xor	eax,eax
	mov	[edi+DirectiveBlock.subtype],al
	mov	[edi+DirectiveBlock.flags],al
	mov	[edi+DirectiveBlock.parameter],eax
	mov	eax,[current_counter]
	sub	eax,[counters_stack_base]
	mov	[edi+DirectiveBlock.prior_counter_position],eax
	lea	eax,[edi+sizeof.DirectiveBlock]
	mov	[directives_stack],eax
	pop	esi
	retn

find_directive_block:
; in:
;  dl = DBLOCK_#
; out:
;  cf set when no such block found
;  when cf = 0
;   edi - DirectiveBlock of the latest block of this type
; preserves: ebx, edx, esi
	mov	edi,[directives_stack]
    scan_directive_blocks:
	cmp	edi,[directives_stack_base]
	je	directive_block_not_found
	sub	edi,sizeof.DirectiveBlock
	cmp	dl,[edi+DirectiveBlock.type]
	je	directive_block_found
    find_next_directive_block:
    ; in:
    ;  dl = DBLOCK_#
    ;  edi - DirectiveBlock
    ; out:
    ;  cf set when no such block found
    ;  when cf = 0
    ;	edi - DirectiveBlock of the next latest block of this type
    ; preserves: ebx, edx, esi
	mov	ecx,[edi+DirectiveBlock.length_of_data]
	sub	edi,ecx
	jmp	scan_directive_blocks
    directive_block_found:
	clc
	retn
    directive_block_not_found:
	stc
	retn

close_control_directive_block:
; in: edi - DirectiveBlock
; preserves: esi
	mov	al,[edi+DirectiveBlock.prior_assembly_mode]
	mov	[assembly_mode],al
	mov	eax,[edi+DirectiveBlock.prior_counter_position]
	add	eax,[counters_stack_base]
	mov	[current_counter],eax
	mov	ecx,[edi+DirectiveBlock.parameter_namespace]
	mov	al,[edi+DirectiveBlock.prior_special_parameters]
	mov	[ecx+SymbolTree_Root.parameters],al
    close_directive_block:
    ; in: edi - DirectiveBlock
    ; preserves: esi
	mov	[edi+DirectiveBlock.type],DBLOCK_CLOSED
	mov	eax,edi
	mov	ecx,[eax+DirectiveBlock.length_of_data]
	sub	eax,ecx
	call	release_source_context
	call	remove_block_parameters
	lea	eax,[edi+sizeof.DirectiveBlock]
	cmp	eax,[directives_stack]
	je	remove_directive_block
	retn
      remove_directive_block:
	sub	eax,sizeof.DirectiveBlock
	mov	ecx,[eax+DirectiveBlock.length_of_data]
	sub	eax,ecx
	cmp	eax,[directives_stack_base]
	je	directives_stack_cleared
	cmp	[eax-sizeof.DirectiveBlock+DirectiveBlock.type],DBLOCK_CLOSED
	je	remove_directive_block
      directives_stack_cleared:
	mov	[directives_stack],eax
	retn

add_block_parameter:
; in:
;  ebx - SymbolTree_Leaf
;  edi - DirectiveBlock
;  esi - value
;  ecx = length of value
;  [value_type] = VALTYPE_#
; preserves: ebx, edi
	push	ecx
	or	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	call	create_value_definition
	pop	ecx
	mov	eax,[current_pass]
	mov	[edx+ValueDefinition.pass],eax
	mov	[edx+ValueDefinition.value_length],ecx
	add	ecx,sizeof.BlockParameter
	push	edi
	mov	edi,[edx+ValueDefinition.value]
	cmp	ecx,[edx+ValueDefinition.block_length]
	jbe	write_parameter_value
	push	edx
	push	ecx
	cmp	[edx+ValueDefinition.block_length],0
	je	allocate_memory_for_parameter
	xor	eax,eax
	xchg	eax,[edx+ValueDefinition.value]
	call	mfree
    allocate_memory_for_parameter:
	pop	ecx
	call	malloc
	pop	edx
	mov	[edx+ValueDefinition.value],eax
	mov	[edx+ValueDefinition.block_length],ecx
	mov	edi,eax
    write_parameter_value:
	mov	al,[value_type]
	mov	[edx+ValueDefinition.type],al
	mov	eax,[edx+ValueDefinition.value_length]
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	mov	ecx,edi
	pop	edi
	mov	[ecx+BlockParameter.symbol],ebx
	mov	[ecx+BlockParameter.definition],edx
	inc	[edx+ValueDefinition.reference_count]
	mov	eax,ecx
	xchg	eax,[edi+DirectiveBlock.parameter]
	mov	[ecx+BlockParameter.next],eax
	retn

remove_block_parameters:
; in: edi - DirectiveBlock
; preserves: esi, edi
	mov	eax,[edi+DirectiveBlock.parameter]
	test	eax,eax
	jz	no_block_parameters
	push	esi edi
	mov	esi,eax
    remove_block_parameter:
	mov	ebx,[esi+BlockParameter.symbol]
	mov	edx,[esi+BlockParameter.definition]
	dec	[edx+ValueDefinition.reference_count]
	xor	edi,edi
	call	remove_value_definition
	mov	esi,[esi+BlockParameter.next]
	test	esi,esi
	jnz	remove_block_parameter
	pop	edi esi
    no_block_parameters:
	retn

identify_parameter_symbol:
; in:
;  esi = pointer into preprocessed line or current embedded value
; out:
;  same as from identify_symbol_in_namespace
	or	[symbol_definition],1
	mov	ebx,[parameter_namespace]
	mov	dl,SYMCLASS_PARAMETER
	call	identify_symbol_in_namespace
	jc	parameter_symbol_identified
	cmp	edi,[parameter_namespace]
	je	parameter_symbol_identified
	xor	ebx,ebx
      parameter_symbol_identified:
	retn

cut_argument_value:
; in:
;  esi = pointer into preprocessed line or current embedded value
;  edi - LineExcerpt to be filled with information about cut piece of line
;  [breakpoint_token] = initial byte of symbol that - if encountered - should immediately end the value
; out:
;  esi = pointer advanced past the cut piece
;  al = initial byte of symbol at pointer, zero when no more symbols there
; preserves: ebx, edi
	call	move_to_next_symbol
	jc	cut_plain_value
	cmp	byte [esi],'<'
	je	cut_enclosed_value
      cut_plain_value:
	mov	dl,','
	xor	dh,dh
	call	cut_piece_of_line
	retn
      cut_enclosed_value:
	inc	esi
	mov	dl,'>'
	mov	dh,'<'
	call	cut_piece_of_line
	cmp	al,'>'
	jne	missing_enclosing
	inc	esi
	call	move_to_next_symbol
	jnc	parameter_value_cut
	xor	al,al
      parameter_value_cut:
	retn
      missing_enclosing:
	mov	edx,_missing_closing_chevron
	call	register_error
	retn

extract_argument_value:
; in:
;  esi = pointer into preprocessed line (not an embedded value)
;  edi - buffer for token sequence, must be large enough to hold all the remaining tokens in line and an additional context token
;  [breakpoint_token] = initial byte of symbol that - if encountered - should immediately end the value
; out:
;  esi = pointer advanced past the processed piece
;  edi - end of the extracted sequence of tokens in provided buffer
;  al = initial byte of symbol at pointer, zero when no more symbols there
; preserves: ebx
; note:
;  when AMODE_DEFINITION is active, current context is not added to the extracted value
	call	move_to_next_symbol
	jc	extract_plain_value
	cmp	byte [esi],'<'
	je	extract_enclosed_value
      extract_plain_value:
	mov	dl,','
	xor	dh,dh
	call	extract_piece_of_line
	retn
      extract_enclosed_value:
	inc	esi
	mov	dl,'>'
	mov	dh,'<'
	call	extract_piece_of_line
	cmp	al,'>'
	jne	missing_enclosing
	inc	esi
	call	move_to_next_symbol
	jnc	parameter_value_extracted
	xor	al,al
      parameter_value_extracted:
	retn

get_macro_definition_symbol:
; in:
;  esi = pointer into preprocessed line or current embedded value
;  al = initial byte of symbol at esi
;  dl = SYMCLASS_#
; out:
;  ebx - SymbolTree_Leaf
;  esi = pointer advanced past the processed declaration
;  [macro_leaf] - SymbolTree_Leaf
;  [macro_flags] = VAL_#
	mov	[macro_flags],VAL_NONRECURSIVE
	cmp	al,'!'
	je	unconditional_interceptor
	or	[symbol_definition],1
	call	identify_symbol
	jc	invalid_macro_symbol
	test	ebx,ebx
	jz	invalid_macro_symbol
    macro_symbol_identified:
	call	move_to_next_symbol
	jc	macro_symbol_ok
	cmp	al,':'
	je	recursive_macro
	test	ecx,ecx
	jnz	macro_symbol_ok
	cmp	al,'!'
	je	unconditional_macro
	jmp	macro_symbol_ok
    unconditional_interceptor:
	mov	ebx,[interceptor_symbol]
	cmp	dl,SYMCLASS_STRUCTURE
	jne	unconditional_macro
	mov	ebx,[label_interceptor_symbol]
    unconditional_macro:
	or	[macro_flags],VAL_UNCONDITIONAL
	inc	esi
	call	move_to_next_symbol
	jc	macro_symbol_ok
	cmp	al,':'
	jne	macro_symbol_ok
    recursive_macro:
	inc	esi
	call	move_to_next_symbol
	and	[macro_flags],not VAL_NONRECURSIVE
	or	[ebx+SymbolTree_Leaf.flags],SYM_CONSTANT
	jmp	macro_symbol_ok
    macro_symbol_ok:
	mov	[macro_leaf],ebx
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	test	edx,edx
	jz	create_placeholder_value
	test	[edx+ValueDefinition.flags],VAL_INTERNAL
	jnz	macro_already_defined
	mov	eax,[current_pass]
	sub	eax,[edx+ValueDefinition.pass]
	jz	macro_already_defined
	cmp	eax,1
	je	macro_symbol_ready
    create_placeholder_value:
    ; for VAL_UNCONDITIONAL macros this helps to detect when a macro tries to call itself,
    ; for regular ones this section interferes with operation of SYM_PREDICTED to sometimes prevent excess passes,
    ; any newly introduced mechanism that would expect SYM_PREDICTED to behave reliably for instruction symbol
    ; should mark the symbol in some way and skip this section upon seeing the mark
	call	create_value_definition
	test	edx,edx
	jz	macro_symbol_ready
	mov	eax,[current_pass]
	dec	eax
	mov	[edx+ValueDefinition.pass],eax
	mov	[edx+ValueDefinition.type],VALTYPE_RESERVED
	mov	al,[macro_flags]
	mov	[edx+ValueDefinition.flags],al
	test	al,VAL_UNCONDITIONAL
	jnz	macro_symbol_ready
	or	[edx+ValueDefinition.flags],VAL_IN_USE
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED
	jz	macro_symbol_ready
	mov	edi,ebx
	mov	ecx,[current_pass]
      check_macro_usage:
	cmp	ecx,[edi+SymbolTree_Leaf.last_use_pass]
	je	macro_possibly_used_earlier
	mov	edx,[edi+SymbolTree_Leaf.fallback_neighbour]
	test	edx,edx
	jz	look_up_parent_namespace_macro
	cmp	ecx,[edx+SymbolTree_Leaf.last_use_pass]
	je	macro_possibly_used_earlier
      look_up_parent_namespace_macro:
	mov	edi,[edi+SymbolTree_Leaf.fallback_parent]
	test	edi,edi
	jnz	check_macro_usage
	and	[ebx+SymbolTree_Leaf.flags],not SYM_PREDICTED
	jmp	macro_symbol_ready
      macro_possibly_used_earlier:
	test	al,VAL_NONRECURSIVE
	jnz	macro_symbol_ready
	or	[next_pass_needed],-1
    macro_symbol_ready:
	clc
	retn
    macro_already_defined:
	test	[ebx+SymbolTree_Leaf.flags],SYM_CONSTANT
	jz	macro_symbol_ready
	mov	edx,_conflicting_definition
	call	register_error
    invalid_macro_symbol:
	stc
	retn

; instruction handler
; in:
;  esi = pointer into preprocessed line
;  ecx = number of whitespace tokens between previous symbol and current position
;  edx - ValueDefinition of instruction
;  ebx - SymbolTree_Leaf of instruction
;  edi - SymbolTree_Root of instruction
;  when [SymbolTree_Leaf.class] = SYMCLASS_STRUCTURE:
;   [label_leaf] - SymbolTree_Leaf of structure label
;   [label_branch] - SymbolTree_Foliage of structure label
;   [label_solid] = non-zero when initial component of label was a name and not a dot
; out:
;  when done, handler should jump to instruction_assembled with esi containing a pointer moved past the processed part of line,
;  or jump directly to assembly_line when the rest of line should be ignored
; note:
;  when esi is equal to [line_end], pointer is at the end of line and there is no data available at this address

set_namespace:
	mov	dl,DBLOCK_NAMESPACE
	mov	ecx,sizeof.NamespaceData
	call	add_directive_block
	mov	edx,[current_context.base_namespace]
	mov	[edi-sizeof.NamespaceData+NamespaceData.prior_namespace],edx
	mov	eax,[edx+SymbolTree_Root.current_label]
	mov	[edi-sizeof.NamespaceData+NamespaceData.prior_label],eax
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
	call	get_symbol_namespace
	mov	[current_context.base_namespace],ebx
	and	[ebx+SymbolTree_Root.current_label],0
	jmp	instruction_assembled
end_namespace:
	mov	dl,DBLOCK_NAMESPACE
	call	find_directive_block
	jc	unexpected_instruction
	mov	edx,[edi-sizeof.NamespaceData+NamespaceData.prior_namespace]
	mov	eax,[edi-sizeof.NamespaceData+NamespaceData.prior_label]
	mov	[current_context.base_namespace],edx
	mov	[edx+SymbolTree_Root.current_label],eax
	call	close_directive_block
	jmp	instruction_assembled

set_base_address:
	mov	edx,[current_area]
	mov	ebx,[edx+ValueDefinition.value]
	test	[ebx+AreaHeader.flags],AREA_VIRTUAL
	jnz	unexpected_instruction
	call	get_expression_value
	jc	invalid_argument
	push	esi
	call	convert_terms_to_numeric_value
    begin_new_output_area:
	call	create_output_area
	inc	[edx+ValueDefinition.reference_count]
	xchg	edx,[current_area]
	dec	[edx+ValueDefinition.reference_count]
       ; jz	 internal_error
	pop	esi
	jmp	instruction_assembled
begin_new_section:
	mov	edx,[current_area]
	mov	ebx,[edx+ValueDefinition.value]
	test	[ebx+AreaHeader.flags],AREA_VIRTUAL
	jnz	unexpected_instruction
	call	get_expression_value
	jc	invalid_argument
	push	esi
	call	convert_terms_to_numeric_value
	call	trim_output
	call	create_output_area
	mov	eax,edx
	inc	[eax+ValueDefinition.reference_count]
	xchg	eax,[current_area]
	dec	[eax+ValueDefinition.reference_count]
       ; jz	 internal_error
	pop	esi
	jmp	instruction_assembled
restart_output:
	mov	edx,[current_area]
	mov	ebx,[edx+ValueDefinition.value]
	test	[ebx+AreaHeader.flags],AREA_VIRTUAL
	jnz	unexpected_instruction
	call	peek_at_constituent_value
	jc	restart_output_at_current_address
	call	get_expression_value
	jc	invalid_argument
	push	esi
	call	convert_terms_to_numeric_value
	jmp	make_new_initial_output_area
    restart_output_at_current_address:
	push	esi
	call	get_current_address_value
    make_new_initial_output_area:
	and	[initial_output_area_entry],0
	jmp	begin_new_output_area

define_label:
	inc	esi
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	define_simple_label
	test	ecx,ecx
	jnz	define_simple_label
	cmp	al,'='
	je	define_numeric_constant
	cmp	al,':'
	je	define_area_label
    define_simple_label:
	test	[assembly_mode],AMODE_SKIP
	jnz	assemble_after_label
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_label_to_macro
	cmp	[interceptor],0
	jne	execute_interceptor
	mov	[argument_start],esi
	call	get_current_address_value
	mov	edi,ecx
	mov	ebx,[label_leaf]
	call	create_constant_value_definition
	test	edx,edx
	jz	assembly_line
	mov	ecx,edi
	call	assign_shiftable_value
    label_defined:
	call	update_current_label
	mov	esi,[argument_start]
    assemble_after_label:
	mov	[line_start],esi
	mov	eax,[embedded_context]
	mov	[line_context],eax
	jmp	assemble_instruction
    define_area_label:
	inc	esi
	test	[assembly_mode],AMODE_SKIP
	jnz	assemble_after_label
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_label_to_macro
	cmp	[interceptor],0
	jne	execute_interceptor
	mov	[argument_start],esi
	mov	ebx,[label_leaf]
	mov	edx,[current_area]
	call	update_value_link
	jmp	label_defined
    assign_shiftable_value:
	mov	[value_type],VALTYPE_NUMERIC
	mov	eax,[current_area]
	mov	eax,[eax+ValueDefinition.value]
	test	[eax+AreaHeader.flags],AREA_SHIFT_TRACKING_DISABLED
	jnz	assign_value
	or	[shift_tracking],-1
	call	update_predicted_shift
	call	assign_value
	or	[edx+ValueDefinition.flags],VAL_SHIFTABLE
	retn
    update_current_label:
	mov	edx,[label_branch]
	cmp	[label_solid],0
	jne	current_label_confirmed
	mov	eax,[local_namespace]
	test	eax,eax
	jz	current_label_ok
	cmp	edx,[eax+SymbolTree_Root.current_label]
	jne	current_label_ok
	test	[eax+SymbolTree_Root.flags],NAMESPACE_CURRENT_LABEL_TO_CONFIRM
	jz	current_label_ok
      current_label_confirmed:
	mov	edi,[edx+SymbolTree_Foliage.root]
	test	[edi+SymbolTree_Root.flags],NAMESPACE_LOCAL
	jnz	current_label_ok
	mov	edi,[current_context.base_namespace]
	mov	[edi+SymbolTree_Root.current_label],edx
      current_label_ok:
	retn

define_numeric_symbol:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	cmp	[interceptor],0
	jne	execute_interceptor
	inc	esi
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	define_numeric_variable
	test	ecx,ecx
	jnz	define_numeric_variable
	cmp	al,':'
	jne	define_numeric_variable
	inc	esi
	call	get_expression_value
	jc	missing_argument
	mov	ebx,[label_leaf]
	call	create_value_definition
	jmp	define_numeric_value
define_numeric_constant:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	cmp	[interceptor],0
	jne	execute_interceptor
	inc	esi
	call	get_expression_value
	jc	missing_argument
	mov	ebx,[label_leaf]
	call	create_constant_value_definition
	jmp	define_numeric_value
define_numeric_variable:
	call	get_expression_value
	jc	missing_argument
	mov	ebx,[label_leaf]
	call	update_value_definition
    define_numeric_value:
	test	edx,edx
	jz	assembly_line
	push	esi
	push	ebx edx
	mov	eax,[edi+ExpressionTerm.attributes]
	cmp	al,EXPR_STRING
	je	define_string_variable
	cmp	al,EXPR_FLOAT
	je	define_float_variable
	mov	[value_type],VALTYPE_NUMERIC
	call	convert_terms_to_numeric_value
    assign_numeric_value:
	pop	edx ebx
	call	assign_value
	pop	esi
	jmp	instruction_assembled
    define_string_variable:
	mov	[value_type],VALTYPE_STRING
	call	get_term_value
	mov	esi,edx
	mov	ecx,[esi]
	add	ecx,4
	jmp	assign_numeric_value
    define_float_variable:
	mov	[value_type],VALTYPE_FLOAT
	call	get_term_value
	mov	esi,edx
	mov	ecx,sizeof.FloatData
	jmp	assign_numeric_value

define_element:
	or	[symbol_definition],1
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
	mov	[label_leaf],ebx
	call	move_to_next_symbol
	jc	element_with_no_metadata
	cmp	al,':'
	jne	get_element_metadata
	inc	esi
    get_element_metadata:
	call	get_expression_value
	jc	invalid_argument
	mov	ebx,[label_leaf]
	call	create_constant_value_definition
	test	edx,edx
	jz	assembly_line
	mov	[value_type],VALTYPE_ELEMENT
	push	esi
	push	ebx edx
	call	convert_terms_to_numeric_value
	pop	edx ebx
	call	assign_value
	pop	esi
	jmp	instruction_assembled
    element_with_no_metadata:
	mov	ebx,[label_leaf]
	call	create_constant_value_definition
	test	edx,edx
	jz	assembly_line
	mov	[value_type],VALTYPE_ELEMENT
	push	esi
	xor	esi,esi
	xor	ecx,ecx
	call	assign_value
	pop	esi
	jmp	instruction_assembled

redefine_raw_symbolic_variable:
	mov	[update_function],update_value_definition
	jmp	raw_symbolic_variable
define_raw_symbolic_variable:
	mov	[update_function],create_value_definition
   raw_symbolic_variable:
	or	[symbol_definition],1
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
	mov	[label_leaf],ebx
	or	[raw_value],1
	jmp	symbolic_variable
redefine_symbolic_variable:
	mov	[update_function],update_value_definition
	and	[raw_value],0
	jmp	symbolic_variable
define_symbolic_variable:
	mov	[update_function],create_value_definition
	and	[raw_value],0
    symbolic_variable:
	mov	edi,[assembly_workspace.memory_start]
	and	[previous_symbol_end],0
    expand_symbols:
	cmp	[raw_value],0
	jne	copy_raw_symbols
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	push	edi
	call	identify_symbol
	jc	symbols_expanded
	test	edi,edi
	jnz	symbol_ready
	call	skip_literal
	xor	ebx,ebx
    symbol_ready:
	pop	edi
	test	ebx,ebx
	jz	copy_symbol
	mov	[further_whitespace],ecx
	call	get_available_value
	jc	copy_symbol
	cmp	[edx+ValueDefinition.type],VALTYPE_SYMBOLIC
	jne	copy_symbol
	and	[previous_symbol_end],0
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	cmp	ebx,[label_leaf]
	jne	replace_symbolic_variable
	mov	ecx,[edx+ValueDefinition.pass]
	cmp	ecx,[current_pass]
	je	replace_symbolic_variable
	mov	edx,_symbolic_self_reference
	call	register_error
	jmp	expand_symbols
    replace_symbolic_variable:
	mov	ecx,[edx+ValueDefinition.value_length]
	test	ecx,ecx
	jz	expand_symbols
	push	esi
	mov	esi,[edx+ValueDefinition.value]
	push	ecx
	add	ecx,1+sizeof.RecognitionContext
	add	ecx,[further_whitespace]
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	al,40h
	cmp	al,[esi]
	je	copy_symbolic_value
	stosb
	mov	eax,esi
	mov	esi,current_context
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	mov	esi,eax
    copy_symbolic_value:
	pop	ecx
	mov	al,cl
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	pop	esi
	mov	al,20h
	mov	ecx,[further_whitespace]
	rep	stosb
	jmp	expand_symbols
    copy_raw_symbols:
	mov	eax,esi
	mov	esi,[line_end]
	cmp	esi,eax
	je	symbolic_value_ready
	mov	[symbol_start],eax
    copy_symbol:
	mov	ebx,esi
	xchg	[previous_symbol_end],esi
	test	esi,esi
	jz	copy_symbol_with_recognition_context
	mov	ecx,ebx
	sub	ecx,esi
	mov	edx,assembly_workspace
	call	reserve_workspace
	and	[context_boundary],0
	jmp	copy_symbol_tokens
    copy_symbol_with_recognition_context:
	mov	ecx,ebx
	sub	ecx,[symbol_start]
	add	ecx,1+sizeof.RecognitionContext
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	al,40h
	stosb
	cmp	[raw_value],0
	je	use_recognition_context
	mov	esi,[embedded_context]
	test	esi,esi
	jz	use_current_context
	call	store_recognition_context
	jmp	symbol_context_stored
      use_current_context:
	call	store_current_context
	jmp	symbol_context_stored
      use_recognition_context:
	mov	esi,recognition_context
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	mov	[context_boundary],edi
      symbol_context_stored:
	mov	esi,[symbol_start]
    copy_symbol_tokens:
	cmp	esi,ebx
	jae	expand_symbols
	call	store_token
	jmp	copy_symbol_tokens
    symbols_expanded:
	pop	edi
    symbolic_value_ready:
	mov	ebx,[label_leaf]
	call	[update_function]
	test	edx,edx
	jz	assembly_line
	mov	[value_type],VALTYPE_SYMBOLIC
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	call	assign_value
	jmp	assembly_line
    store_token:
	lodsb
	cmp	al,40h
	je	make_recognition_context_token
	stosb
	cmp	al,1Ah
	je	store_token_with_data
	cmp	al,22h
	je	store_token_with_data
	cmp	al,27h
	je	store_token_with_data
	cmp	al,30h
	je	store_internal_token
	retn
      store_token_with_data:
	movsd
	retn
      store_internal_token:
	lodsd
	stosd
	mov	ecx,eax
	rep	movsb
	retn

restore_variables:
	mov	al,[edx+ValueDefinition.attribute]
	mov	[chosen_class],al
    restore_symbol_value:
	mov	dl,[chosen_class]
	or	[symbol_definition],1
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
    remove_variable_value:
	xor	edx,edx
	xor	edi,edi
	call	remove_value_definition
	call	move_to_next_symbol
	jc	assembly_line
	cmp	al,','
	jne	invalid_argument
	inc	esi
	jmp	restore_symbol_value
move_variable_values:
	mov	dl,[edx+ValueDefinition.attribute]
	mov	[chosen_class],dl
	or	[symbol_definition],1
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
	mov	[label_leaf],ebx
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,','
	jne	invalid_argument
	inc	esi
	mov	dl,[chosen_class]
	and	[symbol_definition],0
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
	mov	al,[ebx+SymbolTree_Leaf.class]
	cmp	al,[chosen_class]
	jne	instruction_assembled
	xor	edx,edx
	mov	edi,[label_leaf]
	call	remove_value_definition
	jmp	instruction_assembled

label_directive:
	or	[symbol_definition],1
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
	mov	[label_leaf],ebx
	mov	[label_branch],edx
	mov	al,[symbol_solid]
	mov	[label_solid],al
	and	[metadata_length],0
	call	peek_at_constituent_value
	jc	label_at_current_address
	cmp	al,1Ah
	jne	label_with_size
	test	edx,edx
	jz	label_with_size
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	label_with_size
	cmp	[edx+ValueDefinition.value],PREPOSITION_AT
	je	label_at_specified_address
    label_with_size:
	cmp	al,':'
	jne	get_label_size
	and	[current_constituent],0
      get_label_size:
	call	get_expression_value
	jc	invalid_argument
	push	esi
	mov	edx,auxiliary_workspace
	call	convert_terms_to_numeric_value_in_workspace
	mov	[metadata_length],ecx
	pop	esi
	call	peek_at_constituent_value
	jc	label_at_current_address
	cmp	al,1Ah
	jne	label_at_current_address
	test	edx,edx
	jz	invalid_argument
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	label_at_current_address
	cmp	[edx+ValueDefinition.value],PREPOSITION_AT
	jne	invalid_argument
    label_at_specified_address:
	and	[current_constituent],0
	call	get_expression_value
	jc	invalid_argument
	push	esi
	call	convert_terms_to_numeric_value
	mov	edi,ecx
	mov	ebx,[label_leaf]
	call	create_constant_value_definition
	test	edx,edx
	jz	label_ready
	call	prepare_label_value
	mov	[value_type],VALTYPE_NUMERIC
	call	assign_value
	jmp	label_ready
    label_at_current_address:
	push	esi
	call	get_current_address_value
	mov	edi,ecx
	mov	ebx,[label_leaf]
	call	create_constant_value_definition
	test	edx,edx
	jz	label_ready
	call	prepare_label_value
	call	assign_shiftable_value
    label_ready:
	pop	esi
	call	update_current_label
	jmp	instruction_assembled
    prepare_label_value:
	mov	ecx,edi
	cmp	[metadata_length],0
	je	label_metadata_ok
	add	edi,esi
	push	edx ecx
	mov	ecx,[metadata_length]
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	esi,[auxiliary_workspace.memory_start]
	mov	ecx,[metadata_length]
	pop	eax
	add	eax,ecx
	rep	movsb
	pop	edx
	mov	ecx,eax
	mov	esi,edi
	sub	esi,ecx
      label_metadata_ok:
	retn

virtual_block:
	mov	dl,DBLOCK_VIRTUAL
	mov	ecx,sizeof.VirtualBlockData
	call	add_directive_block
	mov	edx,[current_area]
	mov	[edi-sizeof.VirtualBlockData+VirtualBlockData.outer_area],edx
	mov	al,[shift_tracking]
	mov	[edi-sizeof.VirtualBlockData+VirtualBlockData.shift_tracking],al
	call	peek_at_constituent_value
	jc	virtual_at_current_address
	cmp	al,1Ah
	jne	invalid_argument
	test	edx,edx
	jz	invalid_argument
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	continue_virtual_block
	mov	eax,[edx+ValueDefinition.value]
	cmp	eax,PREPOSITION_AS
	je	virtual_at_current_address
	cmp	eax,PREPOSITION_AT
	jne	invalid_argument
	and	[current_constituent],0
	call	get_expression_value
	jc	invalid_argument
	push	esi
	call	convert_terms_to_numeric_value
	jmp	create_virtual_block
    virtual_at_current_address:
	push	esi
	call	get_current_address_value
    create_virtual_block:
	lea	ebx,[virtual_area]
	call	create_area
	or	[ebx+AreaHeader.flags],AREA_VIRTUAL
	cmp	[shift_tracking],0
	je	virtual_block_ready
	or	[ebx+AreaHeader.flags],AREA_SHIFT_TRACKING_DISABLED
      virtual_block_ready:
	inc	[edx+ValueDefinition.reference_count]
	or	[edx+ValueDefinition.flags],VAL_IN_USE
	mov	[current_area],edx
	pop	esi
	call	get_constituent_value
	jc	instruction_assembled
	cmp	al,1Ah
	jne	invalid_argument
	test	edx,edx
	jz	invalid_argument
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	invalid_argument
	cmp	[edx+ValueDefinition.value],PREPOSITION_AS
	jne	invalid_argument
	call	get_constant_value
	cmp	al,22h
	jne	invalid_argument
	mov	ebx,[auxiliary_output_areas]
	mov	edi,esi
	mov	ecx,[edx]
	lea	esi,[edx+4]
	mov	eax,[current_area]
	inc	[eax+ValueDefinition.reference_count]
	push	ecx
	call	put_into_map
	pop	ecx
	jmp	validate_extension
    continue_virtual_block:
	and	[leave_opening_parentheses],0
	mov	edi,[expression_workspace.memory_start]
	call	parse_expression
	mov	edi,[expression_workspace.memory_start]
	call	get_area_value
	jc	invalid_area
	mov	ecx,[current_pass]
	cmp	[edx+ValueDefinition.pass],ecx
	jne	invalid_area
	mov	eax,[edx+ValueDefinition.value]
	test	[eax+AreaHeader.flags],AREA_VIRTUAL
	jz	invalid_area
	test	[edx+ValueDefinition.flags],VAL_IN_USE
	jnz	invalid_area
	inc	[edx+ValueDefinition.reference_count]
	or	[edx+ValueDefinition.flags],VAL_IN_USE
	mov	[current_area],edx
	jmp	instruction_assembled

end_virtual_block:
	mov	dl,DBLOCK_VIRTUAL
	call	find_directive_block
	jc	unexpected_instruction
	mov	al,[edi-sizeof.VirtualBlockData+VirtualBlockData.shift_tracking]
	mov	[shift_tracking],al
	mov	edx,[edi-sizeof.VirtualBlockData+VirtualBlockData.outer_area]
	xchg	edx,[current_area]
	and	[edx+ValueDefinition.flags],not VAL_IN_USE
	dec	[edx+ValueDefinition.reference_count]
	jnz	close_virtual_block
	mov	eax,edx
	xchg	eax,[retired_definition]
	mov	[edx+ValueDefinition.previous],eax
    close_virtual_block:
	call	close_directive_block
	jmp	instruction_assembled

include_source:
	call	move_to_next_symbol
	jc	missing_argument
	cmp	al,'!'
	jne	conditional_include
	inc	esi
	jmp	get_file_name
    conditional_include:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
    get_file_name:
	call	get_constant_value
	cmp	al,22h
	jne	invalid_argument
	push	esi
	mov	[file_name],edx
	call	prepare_file_path
	mov	esi,[assembly_workspace.memory_start]
	call	read_source
	test	eax,eax
	jnz	include_read
	mov	esi,[local_path]
	call	read_source
	test	eax,eax
	jnz	include_read
	mov	ebx,esi
	mov	esi,[include_paths]
    try_include_paths:
	lodsb
	test	al,al
	jz	include_not_found
	cmp	al,';'
	je	try_include_paths
	lea	ebx,[esi-1]
      measure_path:
	lodsb
	test	al,al
	jz	path_measured
	cmp	al,';'
	jne	measure_path
      path_measured:
	dec	esi
	xchg	esi,ebx
	mov	ecx,ebx
	sub	ecx,esi
	mov	edx,[file_name]
	add	ecx,[edx]
	add	ecx,2
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	call	reserve_workspace
	mov	ecx,ebx
	sub	ecx,esi
	rep	movsb
	mov	al,[esi-1]
	cmp	al,'/'
	je	path_separator_ok
	cmp	al,'\'
	je	path_separator_ok
	mov	al,'/'
	stosb
      path_separator_ok:
	mov	esi,[file_name]
	lodsd
	mov	ecx,eax
	rep	movsb
	xor	al,al
	stosb
	mov	esi,[assembly_workspace.memory_start]
	push	ebx
	call	read_source
	mov	ebx,esi
	pop	esi
	test	eax,eax
	jz	try_include_paths
	mov	esi,ebx
    include_read:
	mov	edx,eax
	call	create_source_entry
	jc	include_stack_limit_exceeded
	mov	[ebx+SourceEntry.type],SOURCE_FILE
	mov	[ebx+SourceEntry.name],esi
	mov	[ebx+SourceEntry.text],edx
	mov	edx,[local_parameter_namespace]
	mov	eax,[edx+SymbolTree_Root.chain]
	test	eax,eax
	jnz	new_parameter_namespace_ok
	call	create_parameter_namespace
	mov	[edx+SymbolTree_Root.chain],eax
      new_parameter_namespace_ok:
	and	[eax+SymbolTree_Root.parameters],0
	mov	[ebx+SourceEntry.local_namespace],eax
	pop	esi
	jmp	instruction_assembled
    include_stack_limit_exceeded:
	pop	esi
	mov	edx,_stack_limit_exceeded
	call	register_error
	jmp	instruction_assembled
    include_not_found:
	pop	esi
	mov	edx,_source_file_not_found
	call	register_error
	jmp	instruction_assembled
    prepare_file_path:
	mov	[local_path],edx
	call	get_file_source_entry
	mov	esi,[ebx+SourceEntry.name]
	mov	ecx,esi
      cut_parent_path:
	lodsb
	test	al,al
	jz	parent_path_cut
	cmp	al,'/'
	je	found_path_segment
	cmp	al,'\'
	jne	cut_parent_path
      found_path_segment:
	mov	ecx,esi
	jmp	cut_parent_path
      parent_path_cut:
	mov	esi,[ebx+SourceEntry.name]
	sub	ecx,esi
	mov	ebx,ecx
	add	ecx,[edx]
	inc	ecx
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	call	reserve_workspace
	mov	ecx,ebx
	rep	movsb
	mov	edx,edi
	xchg	edx,[local_path]
	lea	esi,[edx+4]
	mov	ecx,[edx]
	mov	ebx,edi
	rep	movsb
	xor	al,al
	stosb
	retn

evaluate_string:
	mov	edi,[assembly_workspace.memory_start]
	mov	[string_end],edi
    collect_source_string:
	call	get_constant_value
	cmp	al,30h
	je	collect_source_byte
	cmp	al,22h
	jne	invalid_argument
	mov	ebx,edx
	mov	edi,[string_end]
	mov	ecx,[ebx]
	inc	ecx
	mov	edx,assembly_workspace
	call	reserve_workspace
	xchg	esi,ebx
	lodsd
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	mov	esi,ebx
    source_fragment_collected:
	mov	[string_end],edi
	call	get_constituent_value
	jc	source_string_ready
	cmp	al,','
	jne	invalid_argument
	jmp	collect_source_string
    collect_source_byte:
	mov	ebx,edx
	mov	edi,[string_end]
	mov	ecx,2
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	edx,ebx
	mov	ecx,1
	call	fit_value
	jc	value_out_of_range
	inc	edi
	jmp	source_fragment_collected
    source_string_ready:
	mov	edi,[string_end]
	xor	al,al
	stosb
	push	esi
	mov	esi,[assembly_workspace.memory_start]
	call	use_source
	mov	edx,eax
	call	create_source_entry
	jc	include_stack_limit_exceeded
	mov	[ebx+SourceEntry.type],SOURCE_MEMORY
	mov	[ebx+SourceEntry.name],esi
	mov	[ebx+SourceEntry.text],edx
	mov	eax,[parameter_namespace]
	mov	[ebx+SourceEntry.local_namespace],eax
	pop	esi
	jmp	instruction_assembled

assert_condition:
	call	get_condition_value
	test	al,al
	jnz	instruction_assembled
	mov	edx,_assertion_failed
	call	register_error
	jmp	instruction_assembled

conditional_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	xor	ecx,ecx
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],CTRL_IF
	or	[edi+DirectiveBlock.flags],CTRLF_ALLOWS_ELSE
	test	[assembly_mode],AMODE_SKIP
	jnz	skip_conditional_block
    evaluate_conditional_block:
	call	get_condition_value
	test	al,al
	jnz	instruction_assembled
	or	[assembly_mode],AMODE_SKIP
	jmp	instruction_assembled
    ignored_directive:
	test	[assembly_mode],AMODE_SKIP
	jz	add_line_to_macro
	jmp	assembly_line

else_conditional_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
	mov	al,[edi+DirectiveBlock.flags]
	test	al,CTRLF_ALLOWS_ELSE
	jz	unexpected_instruction
	mov	ah,[assembly_mode]
	push	eax
	call	close_control_directive_block
	mov	dl,DBLOCK_CONTROL
	xor	ecx,ecx
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],CTRL_IF
	or	[edi+DirectiveBlock.flags],CTRLF_ALLOWS_ELSE
	pop	eax
	test	al,CTRLF_INACTIVE
	jnz	skip_conditional_block
	test	ah,AMODE_SKIP
	jnz	evaluate_conditional_block
    skip_conditional_block:
	or	[assembly_mode],AMODE_SKIP
	or	[edi+DirectiveBlock.flags],CTRLF_INACTIVE
	jmp	assembly_line

end_conditional_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
	cmp	[edi+DirectiveBlock.subtype],CTRL_IF
	jne	unexpected_instruction
    end_control_block:
	call	close_control_directive_block
	jmp	instruction_assembled

conditionally_repeated_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	test	[assembly_mode],AMODE_SKIP
	jnz	inactive_conditionally_repeated_block
	mov	ecx,[line_end]
	sub	ecx,esi
	add	ecx,sizeof.ConditionalRepeatData
	cmp	[embedded_context],0
	je	condition_length_ok
	add	ecx,1+sizeof.RecognitionContext
      condition_length_ok:
	mov	dl,DBLOCK_CONTROL
	call	add_directive_block
	push	esi
	mov	ecx,[line_end]
	sub	ecx,esi
	sub	edi,sizeof.ConditionalRepeatData
	mov	edx,ecx
	sub	edi,ecx
	mov	eax,ecx
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	mov	esi,[embedded_context]
	test	esi,esi
	jz	condition_stored
	add	edx,1+sizeof.RecognitionContext
	mov	ebx,edi
	sub	edi,edx
	mov	al,40h
	stosb
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	mov	edi,ebx
      condition_stored:
	mov	[edi+ConditionalRepeatData.condition_length],edx
	add	edi,sizeof.ConditionalRepeatData
	pop	esi
	mov	[edi+DirectiveBlock.subtype],CTRL_WHILE
	or	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE
	call	get_condition_value
	test	al,al
	jz	skip_repeated_block
    start_repeated_block:
	mov	ecx,2
	call	allocate_counter
	mov	edi,ebx
	mov	al,1
	stosb
	stosb
	mov	[current_counter],ebx
	mov	ecx,[parameter_namespace]
	mov	[ecx+SymbolTree_Root.parameters],SPECPARM_COUNTER
	test	esi,esi
	jz	assembly_line
	jmp	instruction_assembled
    inactive_conditionally_repeated_block:
	xor	esi,esi
	mov	dh,CTRL_WHILE
    inactive_breakable_block:
	mov	dl,DBLOCK_CONTROL
	xor	ecx,ecx
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],dh
	or	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE
    skip_repeated_block:
	or	[assembly_mode],AMODE_SKIP
	mov	ecx,1
	call	allocate_counter
	mov	byte [ebx],0
	mov	[current_counter],ebx
	mov	ecx,[parameter_namespace]
	mov	[ecx+SymbolTree_Root.parameters],0
	test	esi,esi
	jz	assembly_line
	jmp	instruction_assembled
    allocate_counter:
	mov	ebx,[current_counter]
	movzx	eax,byte [ebx]
	lea	ebx,[ebx+1+eax]
	add	ecx,ebx
	cmp	ecx,[counters_stack_end]
	jbe	counter_allocated
	mov	eax,[counters_stack_base]
	sub	ecx,eax
	sub	ebx,eax
	call	grow_stack
	add	ecx,eax
	mov	[counters_stack_end],ecx
	mov	[counters_stack_base],eax
	add	ebx,eax
      counter_allocated:
	retn

end_conditionally_repeated_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
	cmp	[edi+DirectiveBlock.subtype],CTRL_WHILE
	jne	unexpected_instruction
	test	[assembly_mode],AMODE_SKIP
	jnz	end_control_block
	push	esi edi
	push	[line_end]
	push	[source_context]
	lea	esi,[edi-sizeof.ConditionalRepeatData]
	mov	[line_end],esi
	sub	esi,[esi+ConditionalRepeatData.condition_length]
	sub	edi,[edi+DirectiveBlock.length_of_data]
	mov	[source_context],edi
	and	[embedded_context],0
	call	get_condition_value
	pop	[source_context]
	pop	[line_end]
	pop	edi esi
	test	al,al
	jz	end_control_block
    repeat_condition_upheld:
	mov	ebx,[current_counter]
	movzx	ecx,byte [ebx]
	inc	ebx
      increase_counter:
	inc	byte [ebx]
	jnz	counter_increased
	inc	ebx
	loop	increase_counter
	cmp	ebx,[counters_stack_end]
	jb	counter_length_grown
	lea	ecx,[ebx+1]
	mov	eax,[counters_stack_base]
	sub	ecx,eax
	sub	ebx,eax
	sub	[current_counter],eax
	call	grow_stack
	add	ecx,eax
	mov	[counters_stack_end],ecx
	mov	[counters_stack_base],eax
	add	[current_counter],eax
	add	ebx,eax
      counter_length_grown:
	mov	byte [ebx],1
	mov	ebx,[current_counter]
	inc	byte [ebx]
      counter_increased:
	push	esi
	test	[edi+DirectiveBlock.flags],CTRLF_HAS_REPEAT_DATA
	jz	index_updated
	mov	esi,[current_counter]
	mov	eax,[counters_stack_base]
	add	eax,[edi-sizeof.RepeatData+RepeatData.index_position]
	cmp	eax,esi
	je	index_updated
	xchg	eax,edi
	movzx	ecx,byte [esi]
	inc	ecx
	rep	movsb
	mov	edi,eax
      index_updated:
	mov	eax,[source_context]
	mov	esi,eax
	call	release_source_context
	sub	edi,[edi+DirectiveBlock.length_of_data]
	xchg	esi,edi
	call	clone_source_context
	pop	esi
	jmp	instruction_assembled

repeated_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	test	[assembly_mode],AMODE_SKIP
	jnz	inactive_repeated_block
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_count
	mov	ecx,[edx]
	add	edx,4
	test	ecx,ecx
	jz	inactive_repeated_block
	mov	al,[edx+ecx-1]
	test	al,al
	js	count_out_of_range
	jnz	count_ok
    optimize_counter:
	test	ecx,ecx
	jz	inactive_repeated_block
	dec	ecx
	mov	al,[edx+ecx-1]
	test	al,al
	jz	optimize_counter
    count_ok:
	push	esi ecx
	mov	esi,edx
	mov	dl,DBLOCK_CONTROL
	add	ecx,sizeof.RepeatData
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],CTRL_REPEAT
	or	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE + CTRLF_HAS_REPEAT_DATA
	mov	[directive_block],edi
	pop	ecx
	sub	edi,sizeof.RepeatData
	mov	[edi+RepeatData.limit_length],ecx
	sub	edi,ecx
	rep	movsb
	pop	esi
	mov	ecx,2
	call	allocate_counter
	mov	eax,ebx
	sub	eax,[counters_stack_base]
	mov	[edi+RepeatData.index_position],eax
	mov	edi,ebx
	mov	al,1
	stosb
	stosb
	mov	[current_counter],ebx
	mov	ecx,[parameter_namespace]
	mov	[ecx+SymbolTree_Root.parameters],SPECPARM_COUNTER + SPECPARM_LIMIT
    define_named_counters:
	xor	al,al
	xchg	al,[current_constituent]
	test	al,al
	jz	get_counter_name
	cmp	al,','
	je	get_counter_name
	cmp	al,1Ah
	jne	invalid_argument
	mov	esi,[symbol_start]
    get_counter_name:
	call	identify_parameter_symbol
	jc	instruction_assembled
	test	ebx,ebx
	jz	invalid_argument
	mov	[label_leaf],ebx
	call	peek_at_constituent_value
	jc	counter_with_default_base
	cmp	al,':'
	jne	counter_with_default_base
	and	[current_constituent],0
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_counter_base
	mov	ecx,[edx]
	test	byte [edx+4+ecx-1],80h
	jns	counter_base_ok
	mov	edx,_value_out_of_range
	call	register_error
	jmp	counter_with_default_base
    invalid_counter_base:
	mov	edx,_invalid_value
	call	register_error
    counter_with_default_base:
	mov	edx,singular_value
    counter_base_ok:
	push	esi
	mov	esi,edx
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	mov	ecx,[esi]
	add	ecx,8
	call	reserve_workspace
	mov	eax,[current_counter]
	sub	eax,[counters_stack_base]
	stosd
	lodsd
	mov	ecx,eax
	stosd
	rep	movsb
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	mov	[value_type],VALTYPE_NUMERIC_SEQUENCE
	mov	ebx,[label_leaf]
	mov	edi,[directive_block]
	call	add_block_parameter
	pop	esi
	jmp	define_named_counters
    invalid_count:
	mov	edx,_invalid_value
	call	register_error
	jmp	inactive_repeated_block
    count_out_of_range:
	mov	edx,_value_out_of_range
	call	register_error
    inactive_repeated_block:
	xor	esi,esi
	mov	dh,CTRL_REPEAT
	jmp	inactive_breakable_block

end_repeated_block:
	mov	dh,CTRL_REPEAT
    close_repeat:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
	cmp	dh,[edi+DirectiveBlock.subtype]
	jne	unexpected_instruction
	test	[assembly_mode],AMODE_SKIP
	jnz	end_control_block
	mov	edx,[current_counter]
	movzx	ecx,byte [edx]
	inc	edx
	lea	ebx,[edi-sizeof.RepeatData]
	cmp	ecx,[ebx+RepeatData.limit_length]
	jne	repeat_condition_upheld
	sub	ebx,ecx
      compare_counter_with_limit:
	mov	al,[ebx]
	cmp	al,[edx]
	jne	repeat_condition_upheld
	inc	ebx
	inc	edx
	loop	compare_counter_with_limit
	jmp	end_control_block

iterator_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	test	[assembly_mode],AMODE_SKIP
	jnz	inactive_iterator_block
	xor	eax,eax
	mov	[breakpoint_token],al
	mov	[number_of_parameters],eax
	mov	edi,[expression_workspace.memory_start]
	call	move_to_next_symbol
	jc	invalid_iterator
	cmp	al,'<'
	jne	iterator_parameter_declaration
	inc	esi
	mov	[breakpoint_token],'>'
      iterator_parameter_declaration:
	push	edi
	call	identify_parameter_symbol
	pop	edi
	jc	invalid_iterator
	test	ebx,ebx
	jz	invalid_iterator
	mov	eax,[number_of_parameters]
	shl	eax,2
	add	eax,[assembly_stack_base]
	lea	ecx,[eax+4]
	cmp	ecx,[assembly_stack_end]
	jbe	store_parameter_leaf
	mov	eax,[assembly_stack_base]
	sub	ecx,eax
	call	grow_stack
	mov	[assembly_stack_base],eax
	add	ecx,eax
	mov	[assembly_stack_end],ecx
	mov	ecx,[number_of_parameters]
	shl	ecx,2
	add	eax,ecx
      store_parameter_leaf:
	mov	[eax],ebx
	mov	edx,expression_workspace
	mov	ecx,sizeof.LineExcerpt
	call	reserve_workspace
	mov	[edi+LineExcerpt.data_start],esi
	mov	[edi+LineExcerpt.data_end],esi
	inc	[number_of_parameters]
	call	move_to_next_symbol
	jc	invalid_iterator
	cmp	al,':'
	je	parameter_with_default_value
	cmp	al,'='
	je	parameter_with_default_value
	cmp	al,'*'
	jne	parameter_declaration_done
	and	[edi+LineExcerpt.data_start],0
	inc	esi
	call	move_to_next_symbol
	jc	invalid_iterator
	jmp	parameter_declaration_done
      parameter_with_default_value:
	inc	esi
	call	cut_argument_value
      parameter_declaration_done:
	add	edi,sizeof.LineExcerpt
	mov	ah,[breakpoint_token]
	test	ah,ah
	jz	iterator_parameters_declared
	inc	esi
	cmp	al,','
	je	iterator_parameter_declaration
	cmp	al,ah
	jne	invalid_iterator
	call	move_to_next_symbol
	jc	invalid_iterator
      iterator_parameters_declared:
	cmp	al,','
	jne	invalid_iterator
	mov	ebx,[number_of_parameters]
	mov	[number_of_values],ebx
	and	[number_of_iterations],0
      collect_iterator_values:
	inc	esi
	call	move_to_next_symbol
	jc	initialize_iterator
	mov	edx,expression_workspace
	mov	ecx,sizeof.LineExcerpt
	call	reserve_workspace
	call	cut_argument_value
	add	edi,sizeof.LineExcerpt
	inc	[number_of_values]
	cmp	ebx,[number_of_parameters]
	jne	iterator_value_collected
	inc	[number_of_iterations]
	xor	ebx,ebx
      iterator_value_collected:
	inc	ebx
	cmp	al,','
	je	collect_iterator_values
      initialize_iterator:
	cmp	[number_of_iterations],0
	je	inactive_iterator_block
	mov	dl,DBLOCK_CONTROL
	mov	ecx,5+sizeof.RepeatData
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],CTRL_IRP
	or	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE + CTRLF_HAS_REPEAT_DATA + CTRLF_HAS_WRITABLE_INDEX
	mov	[directive_block],edi
	bsr	ecx,[number_of_iterations]
	shr	ecx,3
	inc	ecx
	push	esi
	sub	edi,sizeof.RepeatData
	mov	[edi+RepeatData.limit_length],ecx
	sub	edi,ecx
	mov	esi,number_of_iterations
	rep	movsb
	mov	ecx,[edi+RepeatData.limit_length]
	add	ecx,3
	call	allocate_counter
	mov	eax,ebx
	sub	eax,[counters_stack_base]
	mov	[edi+RepeatData.index_position],eax
	mov	ecx,[edi+RepeatData.limit_length]
	mov	edi,ebx
	mov	al,1
	stosb
	stosb
	lea	edi,[edi+ecx-1]
	mov	[current_counter],edi
	stosb
	stosb
	mov	ecx,[parameter_namespace]
	mov	[ecx+SymbolTree_Root.parameters],SPECPARM_COUNTER + SPECPARM_LIMIT
	xor	eax,eax
      create_iterator_sequences:
	mov	[parameter_index],eax
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	mov	ecx,[number_of_iterations]
	add	ecx,2
	shl	ecx,2
	call	reserve_workspace
	mov	eax,[directive_block]
	mov	eax,[eax-sizeof.RepeatData+RepeatData.index_position]
	stosd
	mov	eax,[number_of_iterations]
	add	eax,2
	shl	eax,2
	stosd
	mov	[sequence_header_length],eax
	mov	edx,edi
	mov	edi,[assembly_workspace.memory_start]
	sub	edx,edi
	mov	[sequence_header_cursor],edx
	add	edi,eax
	mov	eax,[number_of_parameters]
	add	eax,[parameter_index]
	cmp	eax,[number_of_values]
	jae	copy_default_value
      copy_individual_value:
	mov	[value_index],eax
	mov	ebx,eax
	assert	sizeof.LineExcerpt = 1 shl 4
	shl	ebx,4
	add	ebx,[expression_workspace.memory_start]
	mov	esi,[ebx+LineExcerpt.data_start]
	mov	ecx,[ebx+LineExcerpt.data_end]
	sub	ecx,esi
	jnz	individual_value_ready
      copy_default_value:
	mov	ebx,[parameter_index]
	assert	sizeof.LineExcerpt = 1 shl 4
	shl	ebx,4
	add	ebx,[expression_workspace.memory_start]
	mov	esi,[ebx+LineExcerpt.data_start]
	test	esi,esi
	jz	missing_required_individual_value
	mov	ecx,[ebx+LineExcerpt.data_end]
	sub	ecx,esi
      individual_value_ready:
	mov	edx,assembly_workspace
	add	ecx,2*(1+sizeof.RecognitionContext)
	call	reserve_workspace
	mov	ecx,[ebx+LineExcerpt.data_end]
	sub	ecx,esi
	jz	next_iterator_value
	cmp	[ebx+LineExcerpt.recognition_context],0
	je	copy_iterator_value
	mov	al,40h
	stosb
	mov	eax,esi
	mov	edx,ecx
	mov	esi,[ebx+LineExcerpt.recognition_context]
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	mov	ecx,edx
	mov	esi,eax
      copy_iterator_value:
	mov	al,cl
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	mov	eax,[ebx+LineExcerpt.recognition_context]
	or	eax,[ebx+LineExcerpt.leftover_context]
	jz	next_iterator_value
	mov	al,40h
	stosb
	xor	eax,eax
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	stosd
      next_iterator_value:
	mov	edx,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,edx
	add	edx,[sequence_header_cursor]
	mov	[edx],ecx
	add	[sequence_header_cursor],4
	mov	eax,[value_index]
	add	eax,[number_of_parameters]
	cmp	eax,[number_of_values]
	jb	copy_individual_value
	mov	eax,[sequence_header_length]
	sub	eax,[sequence_header_cursor]
	jne	copy_default_value
	mov	ecx,edi
	mov	esi,[assembly_workspace.memory_start]
	sub	ecx,esi
	mov	eax,[parameter_index]
	shl	eax,2
	add	eax,[assembly_stack_base]
	mov	ebx,[eax]
	mov	edi,[directive_block]
	mov	[value_type],VALTYPE_SYMBOLIC_SEQUENCE
	call	add_block_parameter
	mov	eax,[parameter_index]
	inc	eax
	cmp	eax,[number_of_parameters]
	jb	create_iterator_sequences
	pop	esi
	jmp	instruction_assembled
    missing_required_individual_value:
	mov	edx,_missing_argument
	call	register_error
	jmp	next_iterator_value
    invalid_iterator:
	mov	edx,_invalid_argument
	call	register_error
    inactive_iterator_block:
	xor	esi,esi
	mov	dh,CTRL_IRP
	jmp	inactive_breakable_block

end_iterator_block:
	mov	dh,CTRL_IRP
	jmp	close_repeat

variable_iterator_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	test	[assembly_mode],AMODE_SKIP
	jnz	skipped_variable_iterator_block
	call	move_to_next_symbol
	jc	invalid_iterator
	call	identify_parameter_symbol
	jc	invalid_variable_iterator
	test	ebx,ebx
	jz	invalid_variable_iterator
	mov	[label_leaf],ebx
	call	move_to_next_symbol
	jc	invalid_variable_iterator
	cmp	al,','
	jne	invalid_variable_iterator
	inc	esi
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	invalid_variable_identifier
	test	ebx,ebx
	jz	invalid_variable_identifier
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	mov	edi,[assembly_stack_base]
	xor	ecx,ecx
	mov	[number_of_values],ecx
	mov	edx,[ebx+SymbolTree_Leaf.definition]
      collect_variable_values:
	test	edx,edx
	jz	variable_values_collected
	mov	eax,[edx+ValueDefinition.pass]
	cmp	eax,[current_pass]
	jne	variable_values_collected
	cmp	[edx+ValueDefinition.type],VALTYPE_RESERVED
	je	skip_variable_value
	add	edi,4
	cmp	edi,[assembly_stack_end]
	jbe	store_variable_value
	push	ecx edx
	mov	eax,[assembly_stack_base]
	sub	edi,eax
	mov	ecx,edi
	call	grow_stack
	add	edi,eax
	mov	[assembly_stack_base],eax
	add	eax,ecx
	mov	[assembly_stack_end],eax
	pop	edx ecx
      store_variable_value:
	mov	[edi-4],edx
	inc	[number_of_values]
	add	ecx,[edx+ValueDefinition.value_length]
	add	ecx,1+sizeof.RecognitionContext
      skip_variable_value:
	mov	edx,[edx+ValueDefinition.previous]
	jmp	collect_variable_values
      variable_values_collected:
	mov	eax,[number_of_values]
	test	eax,eax
	jz	inactive_variable_iterator_block
	add	eax,2
	shl	eax,2
	add	ecx,eax
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	call	reserve_workspace
	mov	dl,DBLOCK_CONTROL
	mov	ecx,5+sizeof.RepeatData
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],CTRL_IRPV
	or	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE + CTRLF_HAS_REPEAT_DATA + CTRLF_HAS_WRITABLE_INDEX
	mov	[directive_block],edi
	bsr	ecx,[number_of_values]
	shr	ecx,3
	inc	ecx
	push	esi
	sub	edi,sizeof.RepeatData
	mov	[edi+RepeatData.limit_length],ecx
	sub	edi,ecx
	mov	esi,number_of_values
	rep	movsb
	mov	ecx,[edi+RepeatData.limit_length]
	add	ecx,3
	call	allocate_counter
	mov	eax,ebx
	sub	eax,[counters_stack_base]
	mov	[edi+RepeatData.index_position],eax
	mov	ecx,[edi+RepeatData.limit_length]
	mov	edi,ebx
	mov	al,1
	stosb
	stosb
	lea	edi,[edi+ecx-1]
	mov	[current_counter],edi
	stosb
	stosb
	mov	ecx,[parameter_namespace]
	mov	[ecx+SymbolTree_Root.parameters],SPECPARM_COUNTER + SPECPARM_LIMIT
	mov	edi,[assembly_workspace.memory_start]
	mov	ecx,[number_of_values]
	mov	edx,ecx
	add	ecx,2
	shl	ecx,2
	mov	ebx,edi
	add	edi,ecx
	mov	eax,[directive_block]
	mov	eax,[eax-sizeof.RepeatData+RepeatData.index_position]
	mov	[ebx],eax
	add	ebx,4
	mov	[ebx],ecx
      create_value_sequence:
	mov	eax,edx
	dec	eax
	shl	eax,2
	add	eax,[assembly_stack_base]
	mov	eax,[eax]
	push	edx
	mov	cl,[eax+ValueDefinition.type]
	cmp	cl,VALTYPE_SYMBOLIC
	je	symbolic_value_in_sequence
	cmp	cl,VALTYPE_NUMERIC
	jne	proxy_value_in_sequence
	mov	esi,[eax+ValueDefinition.value]
	mov	ecx,[esi]
	add	esi,4
	test	byte [esi+ecx-1],80h
	jnz	proxy_value_in_sequence
	cmp	dword [esi+ecx],0
	jne	proxy_value_in_sequence
	push	ecx
	add	ecx,1+4
	call	allocate_value_in_sequence
	pop	ecx
	mov	al,30h
	stosb
	mov	eax,ecx
	stosd
	rep	movsb
	jmp	next_value_in_sequence
     allocate_value_in_sequence:
	mov	eax,[ebx]
	add	eax,ecx
	add	ebx,4
	mov	[ebx],eax
	mov	edx,assembly_workspace
	sub	ebx,[edx+Workspace.memory_start]
	call	reserve_workspace
	add	ebx,[assembly_workspace.memory_start]
	retn
      proxy_value_in_sequence:
	mov	esi,eax
	mov	ecx,(1+sizeof.RecognitionContext)*2+1
	call	allocate_value_in_sequence
	push	ebx edi
	push	esi
	xor	eax,eax
	mov	ecx,[proxy_number]
	inc	[proxy_number]
	mov	dl,SYMCLASS_EXPRESSION
	mov	ebx,[current_context.base_namespace]
	call	get_abstract_symbol
	xchg	edx,[esp]
	call	update_value_link
	pop	edx
	pop	edi ebx
	mov	al,40h
	stosb
	mov	eax,[current_context.base_namespace]
	mov	[edi+RecognitionContext.base_namespace],eax
	mov	[edi+RecognitionContext.base_label],edx
	add	edi,sizeof.RecognitionContext
	mov	al,'.'
	stosb
	jmp	close_symbolic_value_in_sequence
      symbolic_value_in_sequence:
	mov	esi,[eax+ValueDefinition.value]
	mov	ecx,[eax+ValueDefinition.value_length]
	push	ecx
	add	ecx,1+sizeof.RecognitionContext
	call	allocate_value_in_sequence
	pop	ecx
	rep	movsb
      close_symbolic_value_in_sequence:
	mov	al,40h
	stosb
	xor	eax,eax
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	stosd
      next_value_in_sequence:
	pop	edx
	dec	edx
	jnz	create_value_sequence
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	mov	ebx,[label_leaf]
	mov	edi,[directive_block]
	mov	[value_type],VALTYPE_SYMBOLIC_SEQUENCE
	call	add_block_parameter
	pop	esi
	jmp	instruction_assembled
    invalid_variable_identifier:
	mov	edx,_invalid_identifier
	call	register_error
	jmp	inactive_variable_iterator_block
    invalid_variable_iterator:
	mov	edx,_invalid_argument
	call	register_error
    skipped_variable_iterator_block:
	xor	esi,esi
    inactive_variable_iterator_block:
	mov	dh,CTRL_IRPV
	jmp	inactive_breakable_block

end_variable_iterator_block:
	mov	dh,CTRL_IRPV
	jmp	close_repeat

set_iterator_index:
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
    find_indexable_block:
	test	[edi+DirectiveBlock.flags],CTRLF_HAS_WRITABLE_INDEX
	jnz	indexable_block_found
	call	find_next_directive_block
	jc	unexpected_instruction
	jmp	find_indexable_block
    indexable_block_found:
       ; test	 [edi+DirectiveBlock.flags],CTRLF_HAS_REPEAT_DATA
       ; jnz	 internal_error
	mov	[directive_block],edi
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_argument
	mov	ecx,[edx]
	add	edx,4
	mov	al,[edx+ecx-1]
	test	al,al
	js	value_out_of_range
	jnz	index_optimized
    optimize_index:
	test	ecx,ecx
	jz	value_out_of_range
	dec	ecx
	mov	al,[edx+ecx-1]
	test	al,al
	jz	optimize_index
    index_optimized:
	mov	edi,[directive_block]
	sub	edi,sizeof.RepeatData
	mov	ebx,[edi+RepeatData.limit_length]
	cmp	ecx,ebx
	ja	value_out_of_range
	jb	index_ok
	sub	edi,ebx
    compare_index_with_limit:
	mov	al,[edx+ebx-1]
	cmp	al,[edi+ebx-1]
	ja	value_out_of_range
	jb	index_ok
	dec	ebx
	jnz	compare_index_with_limit
    index_ok:
	xchg	esi,edx
	mov	edi,[directive_block]
	mov	edi,[edi-sizeof.RepeatData+RepeatData.index_position]
	add	edi,[counters_stack_base]
	mov	al,cl
	stosb
	rep	movsb
	mov	esi,edx
	jmp	instruction_assembled

break_directive:
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
    find_block_to_break:
	test	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE
	jnz	block_to_break_found
	call	find_next_directive_block
	jc	unexpected_instruction
	jmp	find_block_to_break
    block_to_break_found:
	or	[assembly_mode],AMODE_SKIP
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
    break_blocks:
	or	[edi+DirectiveBlock.flags],CTRLF_INACTIVE
	test	[edi+DirectiveBlock.flags],CTRLF_BREAKABLE
	jnz	disable_counter
	or	[edi+DirectiveBlock.prior_assembly_mode],AMODE_SKIP
	call	find_next_directive_block
	jc	unexpected_instruction
	jmp	break_blocks
    disable_counter:
	mov	edi,[current_counter]
	xor	al,al
	stosb
	jmp	instruction_assembled

else_directive:
	mov	edx,[ebx+SymbolTree_Leaf.branch]
	call	get_symbol_namespace
	and	[symbol_definition],0
	mov	dl,SYMCLASS_INSTRUCTION
	call	identify_symbol_in_namespace
	jc	plain_else
	test	ebx,ebx
	jz	invalid_else
	mov	al,[ebx+SymbolTree_Leaf.class]
	cmp	al,SYMCLASS_INSTRUCTION
	jne	invalid_else
	call	get_available_value
	jc	invalid_else
	jmp	prefixed_directive
    invalid_else:
	mov	esi,[symbol_start]
    plain_else:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
	test	[edi+DirectiveBlock.flags],CTRLF_ALLOWS_ELSE
	jz	unexpected_instruction
	and	[edi+DirectiveBlock.flags],not CTRLF_ALLOWS_ELSE
	call	remove_block_parameters
	test	[edi+DirectiveBlock.flags],CTRLF_INACTIVE
	jnz	assembly_line
	xor	[assembly_mode],AMODE_SKIP
	jmp	instruction_assembled

end_directive:
	mov	edx,[ebx+SymbolTree_Leaf.branch]
	call	get_symbol_namespace
	and	[symbol_definition],0
	mov	dl,SYMCLASS_INSTRUCTION
	call	identify_symbol_in_namespace
	jc	missing_prefixed_directive
	test	ebx,ebx
	jz	invalid_prefixed_directive
	mov	al,[ebx+SymbolTree_Leaf.class]
	cmp	al,SYMCLASS_INSTRUCTION
	jne	invalid_prefixed_directive
	call	get_available_value
	jc	invalid_prefixed_directive
    prefixed_directive:
	test	[edx+ValueDefinition.flags],VAL_UNCONDITIONAL
	jnz	execute_prefixed_directive
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
      execute_prefixed_directive:
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_CALM
	je	launch_calm
	cmp	al,VALTYPE_SYMBOLIC
	je	use_macro
	cmp	al,VALTYPE_NATIVE_COMMAND
	jne	invalid_argument
	jmp	[edx+ValueDefinition.value]
    missing_prefixed_directive:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	cmp	[interceptor],0
	jne	execute_interceptor
	jmp	missing_argument
    invalid_prefixed_directive:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	cmp	[interceptor],0
	jne	execute_interceptor
	jmp	invalid_argument

raw_match_block:
	mov	bl,CTRL_RMATCH
	jmp	begin_match_block
match_block:
	mov	bl,CTRL_MATCH
    begin_match_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	cmp	bl,CTRL_RMATCH
	sete	[unprocessed_matching]
	mov	dl,DBLOCK_CONTROL
	xor	ecx,ecx
	call	add_directive_block
	mov	[directive_block],edi
	mov	[edi+DirectiveBlock.subtype],bl
	or	[edi+DirectiveBlock.flags],CTRLF_ALLOWS_ELSE
	test	[assembly_mode],AMODE_SKIP
	jnz	skip_conditional_block
    evaluate_match:
	mov	[pattern_start],esi
	and	[number_of_parameters],0
    get_match_pattern:
	call	move_to_next_symbol
	jc	invalid_match
      qualify_pattern_element:
	cmp	al,','
	je	match_pattern_ok
	cmp	al,1Ah
	je	count_pattern_parameter
	cmp	al,30h
	je	invalid_match
	cmp	al,'='
	jne	skip_pattern_symbol
	call	skip_token
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	invalid_match
	test	ecx,ecx
	jz	skip_pattern_symbol
	jmp	qualify_pattern_element
      count_pattern_parameter:
	call	detect_numeric_symbol
	jc	invalid_match
	inc	[number_of_parameters]
	add	esi,1+4
	jmp	get_match_pattern
      skip_pattern_symbol:
	call	skip_token
	jmp	get_match_pattern
    match_pattern_ok:
	mov	[pattern_end],esi
	inc	esi
	mov	[argument_start],esi
	mov	eax,[embedded_context]
	mov	[matched_context],eax
	mov	edi,[expression_workspace.memory_start]
	mov	ecx,[number_of_parameters]
	assert	sizeof.MatchedParameter = 1 shl 3
	shl	ecx,3
	mov	ebx,ecx
	mov	edx,expression_workspace
	call	reserve_workspace
	add	edi,ebx
	mov	[substitutions_end],edi
	cmp	[unprocessed_matching],0
	jne	substitutions_ready
    detect_substitutions:
	mov	eax,[embedded_context]
	mov	[stored_context],eax
	and	[symbol_definition],0
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	substitutions_ready
	test	edi,edi
	jnz	check_for_substitution
	call	skip_literal
	xor	ebx,ebx
      check_for_substitution:
	test	ebx,ebx
	jz	detect_substitutions
	call	get_available_value
	jc	detect_substitutions
	cmp	[edx+ValueDefinition.type],VALTYPE_SYMBOLIC
	jne	detect_substitutions
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	mov	ebx,edx
	mov	edi,[substitutions_end]
	mov	edx,expression_workspace
	mov	ecx,sizeof.SymbolSubstitution
	call	reserve_workspace
	mov	eax,[ebx+ValueDefinition.value]
	mov	[edi+SymbolSubstitution.value_start],eax
	add	eax,[ebx+ValueDefinition.value_length]
	mov	[edi+SymbolSubstitution.value_end],eax
	mov	ebx,esi
	mov	esi,[symbol_start]
	mov	[edi+SymbolSubstitution.symbol_start],esi
      scan_replaced_symbol:
	mov	al,[esi]
	cmp	al,20h
	je	whitespace_in_replaced_symbol
	cmp	al,40h
	jne	actual_replaced_symbol
	inc	esi
	mov	[stored_context],esi
	add	esi,sizeof.RecognitionContext
	jmp	next_token_in_replaced_symbol
      whitespace_in_replaced_symbol:
	inc	esi
	jmp	next_token_in_replaced_symbol
      actual_replaced_symbol:
	call	skip_token
	mov	ecx,esi
	mov	edx,[stored_context]
      next_token_in_replaced_symbol:
	cmp	esi,ebx
	jb	scan_replaced_symbol
	mov	[edi+SymbolSubstitution.symbol_end],ecx
	mov	[edi+SymbolSubstitution.leftover_context],edx
	add	edi,sizeof.SymbolSubstitution
	mov	[substitutions_end],edi
	jmp	detect_substitutions
    substitutions_ready:
	mov	ebx,[number_of_parameters]
	assert	sizeof.MatchedParameter = 1 shl 3
	shl	ebx,3
	add	ebx,[expression_workspace.memory_start]
	mov	esi,[argument_start]
	mov	edi,[pattern_start]
	xor	eax,eax
	mov	[parameter_index],eax
	mov	[stored_position],eax
	mov	[substitution_active],al
    match_with_pattern:
	call	get_pattern_token
	cmp	ah,1Ah
	jne	exact_match
    wildcard_match:
	call	get_matched_token
	jc	no_match
	cmp	al,20h
	jne	wildcard_matched
	inc	esi
	jmp	wildcard_match
      wildcard_matched:
	xor	eax,eax
	mov	edx,[expression_workspace.memory_start]
	mov	ecx,[parameter_index]
	jecxz	prepare_matched_parameter
	dec	ecx
	assert	sizeof.MatchedParameter = 1 shl 3
	shl	ecx,3
	add	edx,ecx
	push	edi
	mov	edi,[assembly_workspace.memory_start]
	add	edi,[edx+MatchedParameter.assembly_position]
	push	edx
	call	close_matched_value
	pop	edx
	mov	eax,edi
	sub	eax,[assembly_workspace.memory_start]
	mov	[edx+MatchedParameter.assembly_position],eax
	pop	edi
	add	edx,sizeof.MatchedParameter
      prepare_matched_parameter:
	mov	[edx+MatchedParameter.pattern],edi
	inc	[parameter_index]
	mov	edi,[assembly_workspace.memory_start]
	add	edi,eax
	xor	eax,eax
	mov	[embedded_context],eax
	mov	[stored_position],eax
	mov	[whitespace_matching],al
	push	edx
	call	copy_matched_token
	pop	edx
	sub	edi,[assembly_workspace.memory_start]
	mov	[edx+MatchedParameter.assembly_position],edi
	mov	edi,[edx+MatchedParameter.pattern]
	add	edi,1+4
	call	get_pattern_token
	cmp	ah,1Ah
	je	wildcard_match
	cmp	ah,'?'
	jne	exact_match
	inc	edi
	jmp	match_with_pattern
    exact_match:
	cmp	[stored_position],0
	jne	match_position_stored
	mov	[stored_position],esi
	mov	[stored_substitution],ebx
	mov	dl,[substitution_active]
	mov	[stored_substitution_activity],dl
	mov	ecx,[matched_context]
	mov	[stored_context],ecx
	mov	[stored_pattern],edi
	or	[whitespace_matching],1
      match_position_stored:
	cmp	ah,','
	je	end_of_pattern
	cmp	ah,20h
	je	allow_whitespace
	cmp	ah,'='
	jne	match_tokens
	inc	edi
	call	get_pattern_token
      match_tokens:
	call	get_matched_token
	jc	no_match
	cmp	al,20h
	je	match_whitespace
	and	[whitespace_matching],0
	cmpsb
	jne	token_mismatch
	cmp	ah,1Ah
	je	match_name_tokens
	cmp	ah,22h
	je	match_string_tokens
	cmp	ah,27h
	je	match_string_tokens
	cmp	ah,30h
	jne	match_with_pattern
	mov	ecx,esi
	mov	edx,edi
	lodsd
	add	esi,eax
	mov	eax,[edi]
	lea	edi,[edi+4+eax]
	call	get_pattern_token
	cmp	ah,'?'
	jne	compare_token_contents
	inc	edi
	jmp	compare_token_contents
      match_string_tokens:
	lodsd
	mov	ecx,eax
	mov	edx,[edi]
	add	edi,4
      compare_token_contents:
	cmp	ecx,edx
	je	match_with_pattern
	push	esi edi
	mov	esi,ecx
	mov	edi,edx
	mov	ecx,[esi]
	add	ecx,4
	repe	cmpsb
	pop	edi esi
	jne	retract_match
	jmp	match_with_pattern
      match_name_tokens:
	lodsd
	mov	ecx,eax
	mov	edx,[edi]
	add	edi,4
	call	get_pattern_token
	cmp	ah,'?'
	je	case_insensitive_match
	cmp	ecx,edx
	je	match_with_pattern
	push	esi edi
	mov	esi,ecx
	mov	edi,edx
	lodsd
	scasd
	jne	name_mismatch
	mov	ecx,eax
	mov	eax,[esi+ecx]
	cmp	eax,[edi+ecx]
	jne	name_mismatch
	repe	cmpsb
	jne	name_mismatch
	pop	edi esi
	jmp	match_with_pattern
      name_mismatch:
	pop	edi esi
	jmp	retract_match
      case_insensitive_match:
	inc	edi
	cmp	ecx,edx
	je	match_with_pattern
	push	esi edi
	mov	esi,ecx
	mov	edi,edx
	lodsd
	scasd
	jne	name_mismatch
	mov	ecx,eax
	mov	eax,[esi+ecx+4]
	cmp	eax,[edi+ecx+4]
	jne	name_mismatch
	xor	eax,eax
      compare_case_insensitively:
	lodsb
	mov	dl,[characters+eax]
	mov	al,[edi]
	inc	edi
	cmp	dl,[characters+eax]
	jne	name_mismatch
	loop	compare_case_insensitively
	pop	edi esi
	jmp	match_with_pattern
      match_converted_number:
	call	convert_number_to_match
	jmp	compare_token_contents
      match_with_converted_number:
	xchg	esi,edi
	call	convert_number_to_match
	xchg	esi,edi
	jmp	compare_token_contents
      convert_number_to_match:
	mov	edx,esi
	lodsd
	add	esi,eax
	push	ebx esi edi
	call	convert_number_back
	pop	edi esi ebx
	mov	ecx,[edi]
	add	edi,4
	retn
      token_mismatch:
	cmp	ax,1A30h
	je	match_converted_number
	cmp	ax,301Ah
	je	match_with_converted_number
    retract_match:
	xor	esi,esi
	xchg	esi,[stored_position]
	test	esi,esi
	jz	no_match
	mov	ebx,[stored_substitution]
	mov	dl,[stored_substitution_activity]
	mov	[substitution_active],dl
	mov	ecx,[stored_context]
	mov	[matched_context],ecx
	mov	edi,[stored_pattern]
	and	[whitespace_matching],0
	mov	edx,[parameter_index]
	sub	edx,1
	jc	no_match
	assert	sizeof.MatchedParameter = 1 shl 3
	shl	edx,3
	add	edx,[expression_workspace.memory_start]
	push	edi
	mov	edi,[assembly_workspace.memory_start]
	add	edi,[edx+MatchedParameter.assembly_position]
	push	edx
      copy_whitespace:
	call	get_matched_token
	jc	no_match
	cmp	al,20h
	jne	copy_additional_symbol
	call	copy_matched_token
	jmp	copy_whitespace
      copy_additional_symbol:
	call	copy_matched_token
	pop	edx
	sub	edi,[assembly_workspace.memory_start]
	mov	[edx+MatchedParameter.assembly_position],edi
	pop	edi
	jmp	match_with_pattern
    match_whitespace:
	cmp	al,ah
	je	required_whitespace
	cmp	[whitespace_matching],0
	je	retract_match
	inc	esi
	jmp	match_tokens
    required_whitespace:
	inc	esi
    allow_whitespace:
	inc	edi
	or	[whitespace_matching],1
	jmp	match_with_pattern
    end_of_pattern:
	call	get_matched_token
	jc	pattern_matched
	cmp	al,20h
	jne	retract_match
	call	skip_token
	jmp	end_of_pattern
    pattern_matched:
	mov	ebx,[parameter_index]
	sub	ebx,1
	jc	matching_done
	assert	sizeof.MatchedParameter = 1 shl 3
	shl	ebx,3
	add	ebx,[expression_workspace.memory_start]
	mov	edi,[assembly_workspace.memory_start]
	add	edi,[ebx+MatchedParameter.assembly_position]
	call	close_matched_value
	sub	edi,[assembly_workspace.memory_start]
	mov	[ebx+MatchedParameter.assembly_position],edi
    matching_done:
	cmp	[number_of_parameters],0
	je	assembly_line
	mov	[symbol_class],SYMCLASS_PARAMETER
	and	[name_volatile],0
	and	[name_token],0
	or	[symbol_required],1
	or	[symbol_expected],1
	mov	[value_type],VALTYPE_SYMBOLIC
	xor	eax,eax
	mov	[value_index],eax
    define_matched_parameters:
	mov	[parameter_index],eax
	assert	sizeof.MatchedParameter = 1 shl 3
	shl	eax,3
	add	eax,[expression_workspace.memory_start]
	mov	edi,[eax+MatchedParameter.pattern]
	mov	esi,[edi+1]
	lodsd
	mov	ecx,eax
	add	edi,1+4
	call	get_pattern_token
	mov	edx,[esi+ecx]
	mov	al,NAME_CASESENSITIVE
	cmp	ah,'?'
	jne	matched_name_kind_ok
	mov	edx,[esi+ecx+4]
	mov	al,NAME_CASEINSENSITIVE
      matched_name_kind_ok:
	mov	[name_kind],al
	mov	ebx,[parameter_namespace]
	call	scan_namespace
	mov	eax,[parameter_index]
	assert	sizeof.MatchedParameter = 1 shl 3
	shl	eax,3
	add	eax,[expression_workspace.memory_start]
	mov	ecx,[eax+MatchedParameter.assembly_position]
	mov	esi,ecx
	xchg	esi,[value_index]
	sub	ecx,esi
	add	esi,[assembly_workspace.memory_start]
	mov	edi,[directive_block]
	call	add_block_parameter
	mov	eax,[parameter_index]
	inc	eax
	cmp	eax,[number_of_parameters]
	jb	define_matched_parameters
	jmp	assembly_line
    no_match:
	or	[assembly_mode],AMODE_SKIP
	jmp	assembly_line
    invalid_match:
	mov	edx,_invalid_argument
	call	register_error
	or	[assembly_mode],AMODE_SKIP
	jmp	assembly_line
    get_pattern_token:
	mov	ah,[edi]
	cmp	ah,40h
	je	skip_pattern_context
	retn
      skip_pattern_context:
	add	edi,1+sizeof.RecognitionContext
	jmp	get_pattern_token
    get_matched_token:
	cmp	ebx,[substitutions_end]
	je	token_in_line
	cmp	[substitution_active],0
	jne	token_in_substitution
	cmp	esi,[ebx+SymbolSubstitution.symbol_start]
	je	substitution_encountered
      token_in_line:
	cmp	esi,[line_end]
	je	no_more_matched_tokens
      check_for_matched_token:
	mov	al,[esi]
	cmp	al,40h
	je	matched_context_change
	clc
	retn
      no_more_matched_tokens:
	stc
	retn
      matched_context_change:
	inc	esi
	cmp	[unprocessed_matching],0
	jne	matched_context_change_done
	mov	[matched_context],esi
      matched_context_change_done:
	add	esi,sizeof.RecognitionContext
	jmp	get_matched_token
      substitution_encountered:
	mov	esi,[ebx+SymbolSubstitution.value_start]
	and	[matched_context],0
	or	[substitution_active],1
      token_in_substitution:
	cmp	esi,[ebx+SymbolSubstitution.value_end]
	jb	check_for_matched_token
	mov	esi,[ebx+SymbolSubstitution.symbol_end]
	mov	ecx,[ebx+SymbolSubstitution.leftover_context]
	mov	[matched_context],ecx
	add	ebx,sizeof.SymbolSubstitution
	and	[substitution_active],0
	jmp	get_matched_token
    skip_token:
	inc	esi
	cmp	al,1Ah
	je	skip_token_with_data
	cmp	al,22h
	je	skip_token_with_data
	cmp	al,27h
	je	skip_token_with_data
	cmp	al,30h
	je	skip_internal_token
	cmp	al,40h
	je	skip_context_token
	retn
      skip_token_with_data:
	add	esi,4
	retn
      skip_internal_token:
	lodsd
	add	esi,eax
	retn
      skip_context_token:
	add	esi,sizeof.RecognitionContext
	retn
    copy_matched_token:
	mov	edx,[matched_context]
	cmp	edx,[embedded_context]
	je	matched_context_ok
	test	edx,edx
	jz	copy_matched_context
	mov	eax,[edx]
	test	eax,eax
	jnz	copy_matched_context
	mov	[matched_context],eax
	cmp	eax,[embedded_context]
	je	matched_context_ok
	mov	edx,eax
      copy_matched_context:
	mov	[embedded_context],edx
	mov	ecx,1+sizeof.RecognitionContext
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	al,40h
	stosb
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	mov	edx,[matched_context]
	test	edx,edx
	jz	clear_matched_context
	xchg	esi,edx
	rep	movsd
	mov	esi,edx
	jmp	matched_context_ok
      clear_matched_context:
	xor	eax,eax
	rep	stosd
      matched_context_ok:
	mov	ecx,1+4
	mov	edx,assembly_workspace
	call	reserve_workspace
	lodsb
	stosb
	cmp	al,1Ah
	je	copy_token_with_data
	cmp	al,22h
	je	copy_token_with_data
	cmp	al,27h
	je	copy_token_with_data
	cmp	al,30h
	je	copy_internal_token
	retn
      copy_token_with_data:
	movsd
	retn
      copy_internal_token:
	mov	ecx,[esi]
	add	ecx,4
	mov	edx,assembly_workspace
	call	reserve_workspace
	lodsd
	stosd
	mov	ecx,eax
	rep	movsb
	retn
    close_matched_value:
	mov	eax,[embedded_context]
	test	eax,eax
	jz	matched_value_closed
	mov	ecx,1+sizeof.RecognitionContext
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	al,40h
	stosb
	xor	eax,eax
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	stosd
      matched_value_closed:
	retn

else_raw_match_block:
	mov	bl,CTRL_RMATCH
	jmp	begin_else_match_block
else_match_block:
	mov	bl,CTRL_MATCH
    begin_else_match_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
	mov	al,[edi+DirectiveBlock.flags]
	test	al,CTRLF_ALLOWS_ELSE
	jz	unexpected_instruction
	mov	ah,[assembly_mode]
	push	eax ebx
	call	close_control_directive_block
	mov	dl,DBLOCK_CONTROL
	xor	ecx,ecx
	call	add_directive_block
	mov	[directive_block],edi
	pop	ebx eax
	cmp	bl,CTRL_RMATCH
	sete	[unprocessed_matching]
	mov	[edi+DirectiveBlock.subtype],bl
	or	[edi+DirectiveBlock.flags],CTRLF_ALLOWS_ELSE
	test	al,CTRLF_INACTIVE
	jnz	skip_conditional_block
	test	ah,AMODE_SKIP
	jnz	evaluate_match
	jmp	skip_conditional_block

end_raw_match_block:
	mov	bl,CTRL_RMATCH
	jmp	close_match_block
end_match_block:
	mov	bl,CTRL_MATCH
    close_match_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	unexpected_instruction
	cmp	[edi+DirectiveBlock.subtype],bl
	jne	unexpected_instruction
	call	close_control_directive_block
	jmp	instruction_assembled

postponed_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	xor	ecx,ecx
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],CTRL_POSTPONE
	mov	[directive_block],edi
	or	[assembly_mode],AMODE_SKIP
	call	get_constituent_value
	jc	instruction_assembled
	cmp	al,'?'
	jne	invalid_argument
	mov	edi,[directive_block]
	or	[edi+DirectiveBlock.flags],CTRLF_SUSPENDED
	jmp	instruction_assembled

end_postponed_block:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	ignored_directive
	mov	dl,DBLOCK_CONTROL
	call	find_directive_block
	jc	end_postponed_execution
	cmp	[edi+DirectiveBlock.subtype],CTRL_POSTPONE
	jne	unexpected_instruction
	mov	al,[edi+DirectiveBlock.prior_assembly_mode]
	mov	[assembly_mode],al
	test	al,AMODE_SKIP
	jz	keep_postponed_block
	call	close_directive_block
	jmp	instruction_assembled
    keep_postponed_block:
	test	[edi+DirectiveBlock.flags],CTRLF_SUSPENDED
	jnz	keep_suspended_block
	mov	[edi+DirectiveBlock.type],DBLOCK_POSTPONED
	jmp	instruction_assembled
    keep_suspended_block:
	mov	[edi+DirectiveBlock.type],DBLOCK_SUSPENDED
	jmp	instruction_assembled
    end_postponed_execution:
	test	[assembly_mode],AMODE_POSTPONED
	jz	unexpected_instruction
	mov	eax,[source_context]
	mov	esi,eax
	call	release_source_context
	and	[esi+SourceContext.number_of_entries],0
	jmp	pass_done

define_struc:
	mov	bl,SYMCLASS_STRUCTURE
	jmp	begin_macro_definition
define_macro:
	mov	bl,SYMCLASS_INSTRUCTION
    begin_macro_definition:
	test	[assembly_mode],AMODE_DEFINITION
	jnz	nested_macro_definition
	mov	dl,DBLOCK_MACRO
	mov	ecx,sizeof.MacroData
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],bl
	mov	[edi-sizeof.MacroData+MacroData.nesting_counter],1
	xor	eax,eax
	mov	[macro_end_position],eax
	mov	[macro_definition_active],al
	or	[assembly_mode],AMODE_DEFINITION
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	invalid_macro_definition
	cmp	al,'('
	je	macro_with_label_parameter
    read_macro_name:
	mov	dl,bl
	call	get_macro_definition_symbol
	jc	invalid_identifier
	mov	[argument_start],esi
	mov	eax,[embedded_context]
	mov	[macro_parameters_context],eax
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	macro_parameters_correct
    check_macro_parameters:
	cmp	al,'#'
	jne	check_parameter_name
	call	check_concatenation
	jnc	check_macro_parameters
    invalid_macro_definition:
	mov	edx,_invalid_argument
	call	register_error
	jmp	assembly_line
    check_parameter_name:
	cmp	al,'?'
	je	parameter_starting_question_mark
	cmp	al,1Ah
	jne	invalid_macro_definition
	call	detect_numeric_symbol
	jc	invalid_macro_definition
      parameter_name_token:
	inc	esi
	lodsd
      parameter_name_present:
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	macro_parameters_correct
	cmp	al,'?'
	je	check_parameter_name_modifier
	cmp	al,'#'
	jne	check_parameter_modifiers
	call	check_concatenation
	jnc	check_parameter_name_concatenation
	call	move_to_next_symbol
	jc	macro_parameters_correct
	jmp	check_parameter_modifiers
      parameter_starting_question_mark:
	inc	esi
      parameter_name_freeform:
	call	move_to_next_symbol
	jc	macro_parameters_correct
	cmp	al,1Ah
	je	parameter_name_token
	cmp	al,'#'
	je	parameter_name_freeform_concatenation
	cmp	al,30h
	jne	invalid_macro_definition
      parameter_number_token:
	inc	esi
	lodsd
	add	esi,eax
	jmp	parameter_name_present
      parameter_name_freeform_concatenation:
	call	check_concatenation
	jc	invalid_macro_definition
	jmp	parameter_name_freeform
      check_parameter_name_concatenation:
	cmp	al,1Ah
	je	parameter_name_token
	cmp	al,30h
	je	parameter_number_token
	cmp	al,'?'
	jne	check_parameter_modifiers
      check_parameter_name_modifier:
	inc	esi
	call	move_to_next_symbol
	jc	macro_parameters_correct
      check_parameter_modifiers:
	cmp	al,':'
	je	check_default_value
	cmp	al,'='
	je	check_default_value
	cmp	al,'*'
	jne	check_next_parameter
	inc	esi
	call	move_to_next_symbol
	jnc	check_next_parameter
	jmp	macro_parameters_correct
    check_default_value:
	mov	edi,[expression_workspace.memory_start]
	inc	esi
	call	move_to_next_symbol
	jc	check_plain_default_value
	cmp	al,'<'
	je	check_enclosed_default_value
      check_plain_default_value:
	mov	dl,','
	xor	dh,dh
	mov	[breakpoint_token],'&'
	call	cut_piece_of_line
	jmp	check_next_parameter
      check_enclosed_default_value:
	inc	esi
	mov	dl,'>'
	mov	dh,'<'
	mov	[breakpoint_token],0
	call	cut_piece_of_line
	cmp	al,'>'
	jne	invalid_macro_definition
	inc	esi
	call	move_to_next_symbol
	jc	macro_parameters_correct
    check_next_parameter:
	test	al,al
	jz	macro_parameters_correct
	inc	esi
	cmp	al,','
	jne	check_final_parameter_modifiers
	call	move_to_next_symbol
	jc	invalid_macro_definition
	jmp	check_macro_parameters
    check_final_parameter_modifiers:
	cmp	al,'&'
	jne	invalid_macro_definition
	call	move_to_next_symbol
	jnc	invalid_macro_definition
    macro_parameters_correct:
	or	[macro_definition_active],1
	cmp	[macro_parameters_context],0
	je	store_macro_header
	xor	esi,esi
	mov	ecx,1+sizeof.RecognitionContext
	call	append_to_macro
	mov	al,40h
	stosb
	mov	esi,[macro_parameters_context]
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
      store_macro_header:
	mov	esi,[argument_start]
	mov	ecx,[line_end]
	sub	ecx,esi
	call	append_to_macro
	call	add_line_break_to_macro
	jmp	instruction_assembled
    nested_macro_definition:
	mov	dl,DBLOCK_MACRO
	call	find_directive_block
       ; jc	 internal_error
	cmp	bl,[edi+DirectiveBlock.subtype]
	jne	add_nested_macro_block
	inc	[edi-sizeof.MacroData+MacroData.nesting_counter]
	jmp	add_line_to_macro
    add_nested_macro_block:
	mov	ecx,sizeof.MacroData
	call	add_directive_block
	mov	[edi+DirectiveBlock.subtype],bl
	mov	[edi-sizeof.MacroData+MacroData.nesting_counter],1
	jmp	add_line_to_macro
    macro_with_label_parameter:
	cmp	bl,SYMCLASS_STRUCTURE
	jne	invalid_macro_definition
	mov	[argument_start],esi
	inc	esi
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	invalid_macro_definition
    check_label_parameter_initial_concatenation:
	cmp	al,'#'
	jne	check_label_parameter_name
	call	check_concatenation
	jnc	check_label_parameter_initial_concatenation
	jmp	invalid_macro_definition
    check_label_parameter_name:
	cmp	al,1Ah
	jne	invalid_macro_definition
	call	detect_numeric_symbol
	jc	invalid_macro_definition
    label_parameter_name_present:
	inc	esi
	lodsd
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	invalid_macro_definition
	cmp	al,'?'
	je	check_label_parameter_name_modifier
	cmp	al,'#'
	jne	label_parameter_name_end
	call	check_concatenation
	jnc	check_label_parameter_name_concatenation
	call	move_to_next_symbol
	jc	invalid_macro_definition
	jmp	label_parameter_name_end
    check_label_parameter_name_concatenation:
	cmp	al,1Ah
	je	label_parameter_name_present
	cmp	al,'?'
	jne	label_parameter_name_end
    check_label_parameter_name_modifier:
	inc	esi
	call	move_to_next_symbol
	jc	invalid_macro_definition
    label_parameter_name_end:
	inc	esi
	cmp	al,')'
	jne	invalid_macro_definition
	push	esi
	or	[macro_definition_active],1
	mov	ecx,esi
	mov	esi,[argument_start]
	sub	ecx,esi
	call	append_to_macro
	and	[macro_definition_active],0
	pop	esi
	xor	ecx,ecx
	call	move_to_next_symbol
	jc	invalid_macro_definition
	jmp	read_macro_name

end_struc:
	mov	bl,SYMCLASS_STRUCTURE
	jmp	close_macro_block
end_macro:
	mov	bl,SYMCLASS_INSTRUCTION
    close_macro_block:
	mov	dl,DBLOCK_MACRO
	call	find_directive_block
	jc	unexpected_instruction
	cmp	bl,[edi+DirectiveBlock.subtype]
	je	close_macro_definition
	; handle improper nesting, for partial backward-compatibility:
	cmp	bl,SYMCLASS_INSTRUCTION
	jne	add_line_to_macro
	call	close_directive_block
	mov	dl,DBLOCK_MACRO
	call	find_directive_block
	jc	unexpected_instruction
       ; cmp	 [edi+DirectiveBlock.subtype],SYMCLASS_INSTRUCTION
       ; jne	 internal_error
    close_macro_definition:
	dec	[edi-sizeof.MacroData+MacroData.nesting_counter]
	jnz	add_line_to_macro
	mov	al,[assembly_mode]
	and	al,not (AMODE_SKIP or AMODE_DEFINITION)
	or	al,[edi+DirectiveBlock.prior_assembly_mode]
	mov	[assembly_mode],al
	call	close_directive_block
	mov	al,[assembly_mode]
	test	al,AMODE_DEFINITION
	jne	add_line_to_macro
	test	al,AMODE_SKIP
	jnz	assembly_line
	push	esi
	cmp	[macro_definition_active],0
	je	macro_definition_done
	mov	ebx,[macro_leaf]
	call	create_value_definition
	test	edx,edx
	jz	macro_definition_done
	mov	[value],eax
	mov	[value_length],ecx
	mov	esi,[macro_buffer]
	mov	ecx,[macro_end_position]
	mov	[value_type],VALTYPE_SYMBOLIC
	call	assign_value
	mov	al,[macro_flags]
	or	[edx+ValueDefinition.flags],al
      macro_definition_done:
	pop	esi
	jmp	instruction_assembled

escape_directive:
	mov	dl,DBLOCK_MACRO
	call	find_directive_block
	jc	unexpected_instruction
	cmp	[edi-sizeof.MacroData+MacroData.nesting_counter],1
	ja	add_line_to_macro
	test	[edi+DirectiveBlock.prior_assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	cmp	[embedded_context],0
	je	escape_context_ok
	push	esi
	xor	esi,esi
	mov	ecx,1+sizeof.RecognitionContext
	call	append_to_macro
	mov	al,40h
	stosb
	mov	esi,[embedded_context]
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	pop	esi
    escape_context_ok:
	mov	ecx,[line_end]
	sub	ecx,esi
	call	append_to_macro
	call	add_line_break_to_macro
	jmp	assembly_line
add_line_to_macro:
	mov	esi,[line_start]
	mov	ecx,[line_end]
	sub	ecx,esi
	call	append_to_macro
	call	add_line_break_to_macro
	jmp	assembly_line
    append_to_macro:
	cmp	[macro_definition_active],0
	je	macro_appended
	mov	edi,[macro_buffer]
	mov	eax,[macro_end_position]
	add	edi,eax
	add	eax,ecx
	mov	[macro_end_position],eax
	cmp	eax,[macro_buffer_length]
	jbe	macro_block_ready
	push	ecx
	mov	ecx,eax
	mov	eax,[macro_buffer]
	sub	edi,eax
	call	realloc
	add	edi,eax
	mov	[macro_buffer],eax
	mov	[macro_buffer_length],ecx
	pop	ecx
      macro_block_ready:
	test	esi,esi
	jz	macro_appended
	mov	al,cl
	and	al,11b
	shr	ecx,2
	rep	movsd
	mov	cl,al
	rep	movsb
     macro_appended:
	retn
add_label_to_macro:
	mov	ecx,esi
	xchg	esi,[line_start]
	sub	ecx,esi
	call	append_to_macro
	mov	esi,[line_start]
	cmp	esi,[line_end]
	jne	assemble_instruction
	call	add_line_break_to_macro
	jmp	assemble_instruction
add_line_break_to_macro:
	mov	eax,[macro_buffer]
	mov	ecx,[macro_end_position]
	cmp	ecx,[macro_buffer_length]
	jb	macro_line_break
	add	ecx,100h
	call	realloc
	mov	[macro_buffer],eax
	mov	[macro_buffer_length],ecx
	mov	ecx,[macro_end_position]
      macro_line_break:
	lea	edi,[eax+ecx]
	xor	al,al
	stosb
	inc	[macro_end_position]
	retn

use_macro:
	and	[label_branch],0
use_struc:
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	mov	[instruction_value],edx
	mov	eax,[ebx+SymbolTree_Leaf.branch]
	mov	[instruction_branch],eax
	xor	ecx,ecx
	call	embed_symbolic_value
    measure_macro_header:
	lodsb
	test	al,al
	jz	macro_header_measured
	cmp	al,1Ah
	je	measure_token_with_data
	cmp	al,22h
	je	measure_token_with_data
	cmp	al,27h
	je	measure_token_with_data
	cmp	al,30h
	je	measure_internal_token
	cmp	al,40h
	jne	measure_macro_header
	add	esi,sizeof.RecognitionContext
	jmp	measure_macro_header
      measure_token_with_data:
	add	esi,4
	jmp	measure_macro_header
      measure_internal_token:
	lodsd
	add	esi,eax
	jmp	measure_macro_header
    macro_header_measured:
	mov	[instruction_body],esi
	dec	esi
	mov	[line_end],esi
	mov	eax,ebx
	mov	ecx,[parameter_namespace]
	call	get_local_anchor
	call	get_local_namespace
	mov	[new_local_namespace],ebx
	mov	eax,[parameter_namespace]
	mov	[ebx+SymbolTree_Root.chain],eax
	mov	edx,[instruction_value]
    local_namespace_ready:
	mov	esi,[edx+ValueDefinition.value]
	or	[symbol_definition],1
	mov	ebx,[new_local_namespace]
	mov	eax,[label_branch]
	mov	[ebx+SymbolTree_Root.current_label],eax
	cmp	[label_solid],0
	jne	struc_label_to_confirm
	mov	edx,[local_namespace]
	test	edx,edx
	jz	struc_label_ok
	cmp	eax,[edx+SymbolTree_Root.current_label]
	jne	struc_label_ok
	test	[edx+SymbolTree_Root.flags],NAMESPACE_CURRENT_LABEL_TO_CONFIRM
	jz	struc_label_ok
      struc_label_to_confirm:
	or	[ebx+SymbolTree_Root.flags],NAMESPACE_CURRENT_LABEL_TO_CONFIRM
      struc_label_ok:
	cmp	byte [esi],'('
	jne	prepare_macro_parameters
	inc	esi
	mov	ebx,[new_local_namespace]
	mov	dl,SYMCLASS_PARAMETER
	call	identify_symbol_in_namespace
	jc	invalid_argument
	test	ebx,ebx
	jz	invalid_argument
	cmp	edi,[new_local_namespace]
	jne	invalid_argument
	push	esi
	push	[line_end]
	push	[embedded_context]
	mov	eax,[line_context]
	mov	[embedded_context],eax
	mov	esi,[line_start]
	mov	ecx,[label_instruction_start]
	mov	[line_end],ecx
	sub	ecx,esi
	add	ecx,(1+sizeof.RecognitionContext)*2
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	call	reserve_workspace
	xor	edx,edx
	mov	[breakpoint_token],dl
	call	extract_piece_of_line
	call	finish_extracted_parameter
	pop	[embedded_context]
	pop	[line_end]
	or	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	call	update_value_definition
	mov	[value_type],VALTYPE_SYMBOLIC
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	call	assign_value
	pop	esi
	call	move_to_next_symbol
	jc	invalid_argument
	cmp	al,')'
	jne	invalid_argument
	inc	esi
    prepare_macro_parameters:
	mov	edi,[expression_workspace.memory_start]
	add	edi,sizeof.LineExcerpt
	xor	eax,eax
	mov	[macro_greedy],al
	mov	[number_of_parameters],eax
    macro_parameter_declaration:
	mov	ebx,[new_local_namespace]
	mov	dl,SYMCLASS_PARAMETER
	push	edi
	call	identify_symbol_in_namespace
	mov	eax,edi
	pop	edi
	jc	macro_parameters_declared
	test	ebx,ebx
	jz	invalid_argument
	cmp	eax,[new_local_namespace]
	jne	invalid_argument
	mov	eax,[number_of_parameters]
	shl	eax,2
	add	eax,[assembly_stack_base]
	lea	ecx,[eax+4]
	cmp	ecx,[assembly_stack_end]
	jbe	store_macro_parameter_leaf
	mov	eax,[assembly_stack_base]
	sub	ecx,eax
	call	grow_stack
	mov	[assembly_stack_base],eax
	add	ecx,eax
	mov	[assembly_stack_end],ecx
	mov	ecx,[number_of_parameters]
	shl	ecx,2
	add	eax,ecx
      store_macro_parameter_leaf:
	mov	[eax],ebx
	mov	edx,expression_workspace
	mov	ecx,sizeof.LineExcerpt
	call	reserve_workspace
	mov	[edi+LineExcerpt.data_start],esi
	mov	[edi+LineExcerpt.data_end],esi
	inc	[number_of_parameters]
	call	move_to_next_symbol
	jc	macro_parameters_declared
	cmp	al,':'
	je	macro_parameter_with_default_value
	cmp	al,'='
	je	macro_parameter_with_default_value
	cmp	al,'*'
	jne	next_parameter_declaration
	and	[edi+LineExcerpt.data_start],0
	inc	esi
	call	move_to_next_symbol
	jnc	next_parameter_declaration
	xor	al,al
	jmp	next_parameter_declaration
      macro_parameter_with_default_value:
	inc	esi
	call	move_to_next_symbol
	jc	get_plain_default_value
	cmp	al,'<'
	je	get_enclosed_default_value
      get_plain_default_value:
	mov	dl,','
	xor	dh,dh
	mov	[breakpoint_token],'&'
	call	cut_piece_of_line
	jmp	next_parameter_declaration
      get_enclosed_default_value:
	inc	esi
	mov	dl,'>'
	mov	dh,'<'
	mov	[breakpoint_token],0
	call	cut_piece_of_line
	cmp	al,'>'
	jne	invalid_argument
	inc	esi
	call	move_to_next_symbol
	jc	macro_parameters_declared
      next_parameter_declaration:
	test	al,al
	jz	macro_parameters_declared
	inc	esi
	add	edi,sizeof.LineExcerpt
	cmp	al,','
	je	macro_parameter_declaration
	cmp	al,'&'
	jne	invalid_argument
	or	[macro_greedy],1
	call	move_to_next_symbol
	jnc	invalid_argument
      macro_parameters_declared:
	call	warp_to_next_symbol
       ; cmp	 [number_of_line_embeddings],0
       ; jne	 internal_error
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	mov	ecx,[line_end]
	sub	ecx,esi
	add	ecx,(1+sizeof.RecognitionContext)*2
	call	reserve_workspace
	xor	eax,eax
	mov	[breakpoint_token],al
	mov	[parameter_index],eax
    macro_parameter_definition:
	mov	eax,[number_of_parameters]
	sub	eax,[parameter_index]
	jz	macro_parameters_ready
	jmp	get_macro_parameter
    macro_parameter_after_comma:
	mov	eax,[number_of_parameters]
	sub	eax,[parameter_index]
	jz	macro_parameters_ready
	inc	esi
      get_macro_parameter:
	mov	edi,[assembly_workspace.memory_start]
	cmp	[macro_greedy],0
	je	standard_parameter_value
	cmp	eax,1
	ja	standard_parameter_value
	xor	edx,edx
	call	extract_piece_of_line
	jmp	parameter_value_ready
      standard_parameter_value:
	call	extract_argument_value
      parameter_value_ready:
	mov	eax,[parameter_index]
	shl	eax,2
	add	eax,[assembly_stack_base]
	mov	ebx,[eax]
	or	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	call	update_value_definition
	mov	[value_type],VALTYPE_SYMBOLIC
	cmp	edi,[assembly_workspace.memory_start]
	je	empty_parameter_value
	call	finish_extracted_parameter
	push	esi
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
    assign_parameter_value:
	call	assign_value
	pop	esi
    parameter_value_assigned:
	inc	[parameter_index]
	call	move_to_next_symbol
	jc	macro_parameter_definition
	cmp	al,','
	je	macro_parameter_after_comma
    macro_parameters_ready:
	mov	edx,[instruction_value]
	mov	ecx,[instruction_body]
	sub	ecx,[edx+ValueDefinition.value]
	push	ecx
	call	create_source_entry
	pop	eax
	jc	stack_limit_exceeded
	mov	[ebx+SourceEntry.type],SOURCE_MACRO
	mov	[ebx+SourceEntry.text],edx
	mov	[ebx+SourceEntry.offset],eax
	or	[edx+ValueDefinition.flags],VAL_IN_USE
	inc	[edx+ValueDefinition.reference_count]
	mov	eax,[new_local_namespace]
	mov	[local_namespace],eax
	and	[eax+SymbolTree_Root.parameters],0
	mov	[ebx+SourceEntry.local_namespace],eax
	mov	ecx,[instruction_branch]
	test	ecx,ecx
	jz	instruction_assembled
	mov	[ebx+SourceEntry.name],ecx
	or	[ebx+SourceEntry.name_length],-1
	jmp	instruction_assembled
    stack_limit_exceeded:
	mov	edx,_stack_limit_exceeded
	call	register_error
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jz	assembly_line
	jmp	add_line_to_macro
    empty_parameter_value:
	mov	edi,[parameter_index]
	inc	edi
	assert	sizeof.LineExcerpt = 1 shl 4
	shl	edi,4
	add	edi,[expression_workspace.memory_start]
	mov	eax,[edi+LineExcerpt.data_start]
	test	eax,eax
	jz	missing_argument
	mov	ecx,[edi+LineExcerpt.data_end]
	sub	ecx,eax
	push	esi
	mov	esi,eax
	test	ecx,ecx
	jz	assign_parameter_value
	push	ebx edx
	mov	ebx,edi
	mov	edi,[assembly_workspace.memory_start]
	add	ecx,2*(1+sizeof.RecognitionContext)
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	al,40h
	stosb
	cmp	[ebx+LineExcerpt.recognition_context],0
	je	parameter_with_current_context
	mov	eax,esi
	mov	esi,[ebx+LineExcerpt.recognition_context]
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	movsd
	mov	[context_boundary],edi
	mov	esi,eax
	jmp	copy_parameter_contents
      parameter_with_current_context:
	call	store_current_context
      copy_parameter_contents:
	call	store_token
	cmp	esi,[ebx+LineExcerpt.data_end]
	jne	copy_parameter_contents
	call	store_context_reset_token
	pop	edx ebx
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	jmp	assign_parameter_value
    finish_extracted_parameter:
	test	[assembly_mode],AMODE_DEFINITION
	jz	store_context_reset_token
	cmp	[embedded_context],0
	jne	store_context_reset_token
	retn
    store_context_reset_token:
	cmp	edi,[context_boundary]
	je	context_reset_in_reused_token
	mov	al,40h
	stosb
      store_context_reset:
	xor	eax,eax
	assert	sizeof.RecognitionContext and 11b = 0
	mov	ecx,sizeof.RecognitionContext shr 2
	rep	stosd
	retn
      context_reset_in_reused_token:
	sub	edi,sizeof.RecognitionContext
	jmp	store_context_reset

local_directive:
	test	[assembly_mode],AMODE_SKIP
	jnz	assembly_line
	test	[assembly_mode],AMODE_DEFINITION
	jnz	add_line_to_macro
	mov	ebx,[local_namespace]
	test	ebx,ebx
	jz	unexpected_instruction
	or	[symbol_definition],1
	mov	dl,SYMCLASS_PARAMETER
	call	identify_symbol_in_namespace
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_argument
	cmp	edi,[local_namespace]
	jne	invalid_argument
	or	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	call	update_value_definition
	mov	eax,[current_pass]
	mov	[edx+ValueDefinition.pass],eax
	mov	[edx+ValueDefinition.type],VALTYPE_NATIVE_COMMAND
	mov	eax,local_symbol_name
	xchg	eax,[edx+ValueDefinition.value]
	cmp	[edx+ValueDefinition.block_length],0
	je	next_local_declaration
	and	[edx+ValueDefinition.block_length],0
	call	mfree
    next_local_declaration:
	call	move_to_next_symbol
	jc	assembly_line
	cmp	al,','
	jne	invalid_argument
	inc	esi
	jmp	local_directive
outscope_directive:
	test	[assembly_mode],AMODE_SKIP
	jnz	assemble_prefixed_instruction
	mov	ebx,[source_context]
	mov	eax,[ebx+SourceContext.number_of_entries]
	dec	eax
	imul	eax,sizeof.SourceEntry
	lea	ebx,[ebx+sizeof.SourceContext+eax]
	test	[assembly_mode],AMODE_DEFINITION
	jnz	define_outscope
	mov	ecx,[ebx+SourceEntry.local_namespace]
	test	[ecx+SymbolTree_Root.flags],NAMESPACE_UNATTACHED
	jnz	unexpected_instruction
	mov	eax,[ecx+SymbolTree_Root.chain]
	test	eax,eax
	jz	unexpected_instruction
    outscope_namespace_ok:
	mov	[parameter_namespace],eax
	test	[eax+SymbolTree_Root.flags],NAMESPACE_UNATTACHED
	jz	outscope_local_namespace_ok
	xor	eax,eax
    outscope_local_namespace_ok:
	mov	[local_namespace],eax
    assemble_prefixed_instruction:
	mov	[line_start],esi
	mov	eax,[embedded_context]
	mov	[line_context],eax
	jmp	assemble_instruction
    define_outscope:
	cmp	[ebx+SourceEntry.type],SOURCE_MACRO
	je	assemble_prefixed_instruction
	jmp	ignored_directive

define_data:
	and	[label_leaf],0
define_labeled_data:
	movzx	eax,[edx+ValueDefinition.attribute]
	mov	[data_unit_length],eax
	test	eax,eax
	jnz	data_unit_length_ok
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_argument
	mov	ecx,4
	mov	edi,data_unit_length
	call	fit_value
	jc	value_out_of_range
	js	value_out_of_range
	cmp	[data_unit_length],0
	je	value_out_of_range
	call	get_constituent_value
	jc	missing_argument
	cmp	al,':'
	je	data_unit_length_ok
	cmp	al,','
	jne	invalid_argument
    data_unit_length_ok:
	cmp	[label_leaf],0
	je	data_definition
	call	define_data_label
    data_definition:
	call	peek_at_constituent_value
	jc	missing_argument
	cmp	al,'?'
	je	single_uninitialized_unit
	call	get_constant_value
	cmp	al,30h
	je	numeric_data
	cmp	al,2Eh
	je	float_data
	cmp	al,22h
	jne	invalid_argument
	call	output_string
    data_outputted:
	call	get_constituent_value
	jc	instruction_assembled
	cmp	al,','
	jne	invalid_argument
	jmp	data_definition
    float_data:
	push	esi
	push	edx
	mov	ecx,[data_unit_length]
	call	initialize_output
	pop	esi
	mov	ecx,[data_unit_length]
	call	fit_float
	pop	esi
	jmp	data_outputted
    numeric_data:
	call	keep_value
	call	peek_at_constituent_value
	jc	single_data_unit
	cmp	al,1Ah
	jne	single_data_unit
	test	edx,edx
	jz	single_data_unit
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	single_data_unit
	cmp	[edx+ValueDefinition.value],PREPOSITION_DUP
	je	duplication_operator
    single_data_unit:
	mov	ecx,[data_unit_length]
	call	initialize_output
	call	get_kept_value
	mov	ecx,[data_unit_length]
	call	fit_value
	jc	value_out_of_range
	jmp	data_outputted
    single_uninitialized_unit:
	and	[current_constituent],0
	mov	ecx,[data_unit_length]
	call	uninitialized_output
	jmp	data_outputted
    duplication_operator:
	call	get_kept_value
	mov	edi,number_of_iterations
	mov	ecx,4
	call	fit_value
	jc	value_out_of_range
	js	value_out_of_range
	xor	eax,eax
	mov	[current_constituent],al
	mov	[initial_parentheses],eax
	call	peek_at_constituent_value
	jc	invalid_argument
	cmp	al,'?'
	je	reserve_through_duplication
	mov	edi,[expression_workspace.memory_start]
	mov	[expression_sequence_end],edi
	or	[leave_opening_parentheses],1
    parse_duplicated_expression:
	call	parse_expression
	add	[initial_parentheses],ecx
	jecxz	duplicated_value_parsed
	mov	eax,[expression_workspace.memory_start]
	mov	eax,[eax]
	test	eax,eax
	jnz	duplicated_value_parsed
	call	get_constituent_value
	jc	invalid_argument
	cmp	al,'?'
	jne	invalid_argument
	mov	edi,[expression_workspace.memory_start]
	or	eax,-1
	stosd
    duplicated_value_parsed:
	mov	[expression_sequence_end],edi
	and	[leave_opening_parentheses],0
	cmp	[initial_parentheses],1
	jb	duplicated_data_parsed
	ja	invalid_argument
	call	get_constituent_value
	jc	missing_closing_parenthesis
	cmp	al,')'
	je	duplicated_data_parsed
	cmp	al,','
	jne	invalid_argument
	mov	edi,[expression_sequence_end]
	call	peek_at_constituent_value
	jc	invalid_argument
	mov	edi,[expression_sequence_end]
	cmp	al,'?'
	jne	parse_duplicated_expression
	and	[current_constituent],0
	mov	ecx,4
	mov	edx,expression_workspace
	call	reserve_workspace
	or	eax,-1
	stosd
	jmp	duplicated_value_parsed
    duplicated_data_parsed:
	cmp	[number_of_iterations],0
	je	data_outputted
    duplicated_data:
	mov	eax,[expression_workspace.memory_start]
	mov	[expression_sequence_cursor],eax
    generate_duplicate:
	mov	eax,[expression_sequence_cursor]
	cmp	eax,[expression_sequence_end]
	je	duplicate_outputted
	cmp	dword [eax],-1
	je	duplicated_uninitialized_unit
	push	esi
	mov	esi,eax
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	mov	[expression_sequence_cursor],esi
	pop	esi
	jc	duplicate_outputted
	call	pop_terms
	jc	invalid_argument
	call	get_calculated_constant_value
	cmp	al,30h
	je	duplicated_data_unit
	cmp	al,2Eh
	je	duplicated_data_unit
	cmp	al,22h
	jne	invalid_argument
	call	output_string
	jmp	generate_duplicate
    duplicated_data_unit:
	push	eax edx
	mov	ecx,[data_unit_length]
	call	initialize_output
	pop	edx eax
	mov	ecx,[data_unit_length]
	cmp	al,2Eh
	je	duplicated_float
	call	fit_value
	jc	value_out_of_range
	jmp	generate_duplicate
    duplicated_float:
	push	esi
	mov	esi,edx
	call	fit_float
	pop	esi
	jmp	generate_duplicate
    duplicated_uninitialized_unit:
	add	eax,4
	mov	[expression_sequence_cursor],eax
	mov	ecx,[data_unit_length]
	call	uninitialized_output
	jmp	generate_duplicate
    duplicate_outputted:
	dec	[number_of_iterations]
	jnz	duplicated_data
	jmp	data_outputted
    reserve_through_duplication:
	and	[current_constituent],0
	mov	eax,[number_of_iterations]
	mul	[data_unit_length]
	test	edx,edx
	jnz	duplication_overflow
	mov	ecx,eax
	call	uninitialized_output
	jmp	data_outputted
    duplication_overflow:
	mov	edx,_area_overflow
	call	register_error
	jmp	data_outputted
    output_string:
	push	esi
	mov	esi,edx
	mov	ecx,[esi]
	call	initialize_output
	lodsd
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	pop	esi
	mov	ecx,[data_unit_length]
	xor	edx,edx
	div	ecx
	test	edx,edx
	jz	string_outputted
	sub	ecx,edx
	push	ecx
	call	initialize_output
	pop	ecx
	xor	al,al
	rep	stosb
      string_outputted:
	retn
    define_data_label:
	push	esi
	call	get_current_address_value
	lea	edi,[esi+ecx]
	mov	ecx,4+4+4
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	edx,[data_unit_length]
	bsr	eax,edx
	inc	al
	shr	al,3
	inc	al
	stosd
	mov	[edi],edx
	add	edi,eax
	xor	eax,eax
	stosd
	mov	ebx,[label_leaf]
	call	create_constant_value_definition
	test	edx,edx
	jz	data_label_defined
	mov	ecx,edi
	sub	ecx,esi
	call	assign_shiftable_value
      data_label_defined:
	call	update_current_label
	pop	esi
	retn

reserve_labeled_data:
	movzx	eax,[edx+ValueDefinition.attribute]
	mov	[data_unit_length],eax
	call	define_data_label
	jmp	data_reservation
reserve_data:
	movzx	eax,[edx+ValueDefinition.attribute]
	mov	[data_unit_length],eax
    data_reservation:
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_argument
	mov	ecx,4
	mov	edi,[assembly_workspace.memory_start]
	call	fit_value
	jc	reservation_out_of_range
	js	reservation_out_of_range
	mov	eax,[edi]
	mul	[data_unit_length]
	test	edx,edx
	jnz	reservation_overflow
	mov	ecx,eax
	call	uninitialized_output
	jmp	instruction_assembled
    reservation_out_of_range:
	mov	edx,_value_out_of_range
	call	register_error
	jmp	instruction_assembled
    reservation_overflow:
	mov	edx,_area_overflow
	call	register_error
	jmp	instruction_assembled

include_labeled_data:
	mov	[data_unit_length],1
	call	define_data_label
include_data:
	xor	eax,eax
	mov	[data_offset],eax
	mov	[data_offset+4],eax
	call	get_constant_value
	cmp	al,22h
	jne	invalid_argument
	push	esi
	call	prepare_file_path
	pop	esi
	call	get_constituent_value
	jc	include_all_available_data
	cmp	al,':'
	jne	get_data_length
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_argument
	mov	edi,file_offset
	mov	ecx,8
	call	fit_value
	jc	value_out_of_range
	js	value_out_of_range
	call	get_constituent_value
	jc	include_all_available_data
    get_data_length:
	cmp	al,','
	jne	invalid_argument
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_argument
	mov	edi,data_length
	mov	ecx,4
	call	fit_value
	jc	value_out_of_range
	js	value_out_of_range
	push	esi
	mov	ecx,[data_length]
	call	initialize_output
	mov	esi,[assembly_workspace.memory_start]
	call	get_file_data
	test	ebx,ebx
	jnz	read_from_file
	mov	esi,[local_path]
	call	get_file_data
	test	ebx,ebx
	jz	data_file_not_found
      read_from_file:
	call	read_file_data
	jc	file_data_not_read
	pop	esi
	jmp	instruction_assembled
    data_file_not_found:
	mov	ebx,esi
	mov	edx,_source_file_not_found
	call	register_error
	pop	esi
	jmp	instruction_assembled
    include_all_available_data:
	push	esi
	mov	esi,[assembly_workspace.memory_start]
	call	get_file_data
	test	ebx,ebx
	jnz	measure_available_data
	mov	esi,[local_path]
	call	get_file_data
	test	ebx,ebx
	jz	data_file_not_found
      measure_available_data:
	mov	ecx,dword [ebx+FileData.length]
	mov	edx,dword [ebx+FileData.length+4]
	sub	ecx,dword [file_offset]
	sbb	edx,dword [file_offset+4]
	jc	file_data_not_read
	jnz	out_of_memory
	mov	[data_length],ecx
	push	ebx
	call	initialize_output
	pop	ebx
	call	read_file_data
	jc	file_data_not_read
	pop	esi
	jmp	instruction_assembled
    file_data_not_read:
	mov	ebx,esi
	mov	edx,_error_reading_file
	call	register_error
	pop	esi
	jmp	instruction_assembled

load_value:
	or	[symbol_definition],1
	mov	dl,SYMCLASS_EXPRESSION
	call	identify_symbol
	jc	missing_argument
	test	ebx,ebx
	jz	invalid_identifier
	mov	[label_leaf],ebx
	mov	[size_specified],0
	call	peek_at_constituent_value
	jc	invalid_load_syntax
	cmp	al,':'
	je	size_after_separator
	cmp	al,1Ah
	jne	get_load_size
	test	edx,edx
	jz	get_load_size
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	get_load_size
	cmp	[edx+ValueDefinition.value],PREPOSITION_FROM
	jne	invalid_load_syntax
	and	[current_constituent],0
	jmp	get_source_address
    size_after_separator:
	and	[current_constituent],0
    get_load_size:
	call	get_numeric_constant_value
	test	edx,edx
	jz	invalid_load_syntax
	or	[size_specified],1
	mov	edi,value_length
	mov	ecx,4
	call	fit_value
	jc	value_out_of_range
	js	value_out_of_range
	call	get_constituent_value
	jc	invalid_load_syntax
	cmp	al,1Ah
	jne	invalid_load_syntax
	test	edx,edx
	jz	invalid_load_syntax
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	invalid_load_syntax
	cmp	[edx+ValueDefinition.value],PREPOSITION_FROM
	jne	invalid_load_syntax
    get_source_address:
	call	peek_at_constituent_value
	jc	invalid_argument
	cmp	al,':'
	je	load_from_output_offset
	mov	eax,[current_area]
	mov	[data_area],eax
	and	[data_area_symbol],0
	and	[leave_opening_parentheses],0
	mov	edi,[expression_workspace.memory_start]
	call	parse_expression
	call	peek_at_constituent_value
	jc	source_address_parsed
	cmp	al,':'
	jne	source_address_parsed
	and	[current_constituent],0
	mov	edi,[expression_workspace.memory_start]
	call	get_area_value
	jc	invalid_area
	mov	[data_area],edx
	mov	edi,[expression_workspace.memory_start]
	call	parse_expression
    source_address_parsed:
	call	calculate_relative_address
	jc	invalid_argument
	mov	edi,data_offset
	mov	ecx,4
	call	fit_value
	jc	data_out_of_area
	js	data_out_of_area
	mov	edi,[assembly_workspace.memory_start]
	mov	eax,[value_length]
	stosd
	mov	ecx,eax
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	edx,[data_area]
    load_from_area:
	mov	eax,[edx+ValueDefinition.value]
	mov	ecx,[edx+ValueDefinition.pass]
	cmp	ecx,[current_pass]
	je	area_ok
	test	[eax+AreaHeader.flags],AREA_VARIABLE
	jnz	source_area_inaccessible
      area_ok:
	mov	ebx,[data_offset]
	add	ebx,[eax+AreaHeader.base_address_length]
	jc	data_out_of_area
	add	ebx,sizeof.AreaHeader
	jc	data_out_of_area
	mov	ecx,[edx+ValueDefinition.value_length]
	sub	ecx,ebx
	jbe	load_out_of_initialized_data
	call	prepare_load_length
	jc	data_out_of_area
	xchg	esi,ebx
	add	esi,eax
	mov	dl,cl
	shr	ecx,2
	rep	movsd
	mov	cl,dl
	and	cl,11b
	rep	movsb
	mov	esi,ebx
	cmp	[value_length],0
	je	value_loaded
	mov	ecx,[eax+AreaHeader.uninitialized_length]
	jmp	load_uninitialized_data
      load_out_of_initialized_data:
	add	ecx,[eax+AreaHeader.uninitialized_length]
	jc	load_uninitialized_data
	xor	ecx,ecx
      load_uninitialized_data:
	call	prepare_load_length
	jc	data_out_of_area
	xor	eax,eax
	mov	dl,cl
	shr	ecx,2
	rep	stosd
	mov	cl,dl
	and	cl,11b
	rep	stosb
	cmp	[value_length],0
	je	value_loaded
	xor	ebx,ebx
	xchg	ebx,[data_area_symbol]
	test	ebx,ebx
	jz	data_out_of_area
	mov	edx,[ebx+SymbolTree_Leaf.retired_definition]
	test	edx,edx
	jz	area_with_no_history
	mov	ecx,[current_pass]
	sub	ecx,[edx+ValueDefinition.pass]
	cmp	ecx,1
	ja	area_with_no_history
	or	[ebx+SymbolTree_Leaf.flags],SYM_LINK_PREDICTED
	mov	eax,[edx+ValueDefinition.value]
	jmp	load_from_area
    invalid_source_area:
	call	parse_expression
    source_area_inaccessible:
	mov	edx,_invalid_area
	call	register_error
	jmp	loaded_no_value
    area_with_no_history:
	or	[next_pass_needed],1
    data_out_of_area:
	mov	edx,_address_out_of_range
	call	register_error
	jmp	loaded_no_value
    invalid_load_syntax:
	mov	edx,_invalid_argument
	call	register_error
    loaded_no_value:
	mov	edi,[assembly_workspace.memory_start]
	xor	eax,eax
	stosd
    value_loaded:
	mov	ebx,[label_leaf]
	call	update_value_definition
	test	edx,edx
	jz	instruction_assembled
	push	esi
	mov	esi,[assembly_workspace.memory_start]
	mov	ecx,[esi]
	add	ecx,4
	mov	[value_type],VALTYPE_STRING
	call	assign_value
	pop	esi
	jmp	instruction_assembled
    prepare_load_length:
	cmp	ecx,[value_length]
	jbe	value_length_ok
	mov	ecx,[value_length]
      value_length_ok:
	add	[data_offset],ecx
	jc	load_length_ready
	sub	[value_length],ecx
      load_length_ready:
	retn
    load_from_output_offset:
	call	read_output_offset
	jc	value_out_of_range
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	mov	eax,[value_length]
	stosd
	mov	ecx,eax
	call	reserve_workspace
	push	esi
	call	read_from_output
	pop	esi
	cmp	[value_length],0
	jne	data_out_of_area
	jmp	value_loaded

store_value:
	mov	[size_specified],0
	call	get_constant_value
	cmp	al,22h
	je	store_string_value
	cmp	al,30h
	jne	invalid_argument
	call	keep_value
	call	peek_at_constituent_value
	jc	invalid_argument
	cmp	al,':'
	je	size_after_value
	cmp	al,1Ah
	jne	size_before_value
	test	edx,edx
	jz	invalid_argument
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	je	get_destination_address
    size_before_value:
	or	[size_specified],1
	call	get_kept_value
	mov	edi,value_length
	mov	ecx,4
	call	fit_value
	jc	value_out_of_range
	js	value_out_of_range
	call	get_constant_value
	cmp	al,22h
	je	value_after_size_ok
	cmp	al,30h
	jne	invalid_argument
    value_after_size_ok:
	call	keep_value
	jmp	value_with_size_ready
    store_string_value:
	mov	eax,[edx]
	mov	[value_length],eax
	call	keep_value
	call	peek_at_constituent_value
	jc	invalid_argument
	cmp	al,':'
	jne	check_symbol_after_value_with_size
    size_after_value:
	and	[current_constituent],0
	call	get_constant_value
	cmp	al,30h
	jne	invalid_argument
	or	[size_specified],1
	mov	edi,value_length
	mov	ecx,4
	call	fit_value
	jc	value_out_of_range
	js	value_out_of_range
    value_with_size_ready:
	call	peek_at_constituent_value
	jc	invalid_argument
    check_symbol_after_value_with_size:
	cmp	al,1Ah
	jne	invalid_argument
	test	edx,edx
	jz	invalid_argument
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	invalid_argument
    get_destination_address:
	cmp	[edx+ValueDefinition.value],PREPOSITION_AT
	jne	invalid_argument
	and	[current_constituent],0
	call	peek_at_constituent_value
	jc	invalid_argument
	cmp	al,':'
	je	store_at_output_offset
	mov	eax,[current_area]
	mov	[data_area],eax
	and	[leave_opening_parentheses],0
	mov	edi,[expression_workspace.memory_start]
	call	parse_expression
	call	peek_at_constituent_value
	jc	destination_address_parsed
	cmp	al,':'
	jne	destination_address_parsed
	and	[current_constituent],0
	mov	edi,[expression_workspace.memory_start]
	call	get_area_value
	jc	invalid_area
	mov	ecx,[current_pass]
	cmp	[edx+ValueDefinition.pass],ecx
	jne	invalid_area
	mov	[data_area],edx
	mov	edi,[expression_workspace.memory_start]
	call	parse_expression
    destination_address_parsed:
	call	calculate_relative_address
	jc	invalid_argument
	mov	edi,data_offset
	mov	ecx,4
	call	fit_value
	jc	store_out_of_area
	js	store_out_of_area
    prepare_store:
	mov	edx,[data_area]
	mov	eax,[edx+ValueDefinition.value]
	or	[eax+AreaHeader.flags],AREA_VARIABLE
	mov	ebx,[data_offset]
	xor	ecx,ecx
	add	ebx,[eax+AreaHeader.base_address_length]
	adc	ecx,0
	add	ebx,sizeof.AreaHeader
	adc	ecx,0
	add	ebx,[value_length]
	adc	ecx,0
	jnz	store_outside_initialized_area
	cmp	ebx,[edx+ValueDefinition.value_length]
	ja	store_outside_initialized_area
	lea	edi,[eax+ebx]
	mov	ecx,[value_length]
	sub	edi,ecx
	call	get_kept_value
	call	fit_value
	jc	value_out_of_range
	jmp	instruction_assembled
    store_outside_initialized_area:
	sub	ebx,[edx+ValueDefinition.value_length]
	sbb	ecx,0
	jnz	store_out_of_area
	sub	[eax+AreaHeader.uninitialized_length],ebx
	jnc	extend_area
	add	[eax+AreaHeader.uninitialized_length],ebx
	test	[eax+AreaHeader.flags],AREA_VIRTUAL
	jz	store_out_of_area
	test	[edx+ValueDefinition.flags],VAL_IN_USE
	jnz	store_out_of_area
	and	[eax+AreaHeader.uninitialized_length],0
	push	edx
	mov	edx,_address_out_of_range
	call	register_error
	pop	edx
    extend_virtual_area:
	call	expand_value
	jmp	prepare_store
    extend_area:
	test	[eax+AreaHeader.flags],AREA_VIRTUAL
	jnz	extend_virtual_area
	call	expand_value
	call	update_output_offsets
	jmp	prepare_store
    store_out_of_area:
	mov	edx,_address_out_of_range
	call	register_error
	jmp	instruction_assembled
    invalid_area:
	mov	edx,_invalid_area
	call	register_error
	jmp	assembly_line
    calculate_relative_address:
	push	esi
	mov	esi,[expression_workspace.memory_start]
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	jc	invalid_relative_address
	mov	ebx,edi
	mov	edi,[expression_workspace.memory_start]
	mov	esi,edi
	mov	eax,EXPR_AREA_ADDRESS
	stosd
	mov	eax,[data_area]
	stosd
	mov	eax,EXPR_OPERATOR
	stosd
	mov	eax,calculate_sub
	stosd
	xor	eax,eax
	stosd
	mov	edi,ebx
	call	calculate_parsed_expression
	jc	invalid_relative_address
	call	pop_terms
	jc	invalid_relative_address
	cmp	[size_specified],0
	jne	addressed_length_ok
	and	[value_length],0
	mov	edx,[edi+ExpressionTerm.metadata]
	test	edx,edx
	jz	addressed_length_ok
	mov	esi,edi
	mov	ecx,4
	mov	edi,value_length
	call	fit_value
	jc	invalid_relative_address
	js	invalid_relative_address
	mov	ecx,[edx]
	cmp	dword [edx+4+ecx],0
	jne	invalid_relative_address
	mov	edi,esi
      addressed_length_ok:
	pop	esi
	mov	eax,edi
      check_for_uncanceled_terms:
	add	eax,sizeof.ExpressionTerm
	cmp	[eax+ExpressionTerm.attributes],0
	je	relative_address_ok
	cmp	[eax+ExpressionTerm.metadata],0
	je	check_for_uncanceled_terms
	mov	edx,_address_out_of_range
	call	register_error
      relative_address_ok:
	call	get_term_value
	clc
	retn
      invalid_relative_address:
	pop	esi
	stc
	retn
    read_output_offset:
	and	[current_constituent],0
	call	get_numeric_constant_value
	test	edx,edx
	jz	output_offset_out_of_range
	push	edi
	mov	edi,file_offset
	mov	ecx,8
	call	fit_value
	pop	edi
	jc	output_offset_out_of_range
	js	output_offset_out_of_range
	cmp	[size_specified],0
	jne	output_offset_ok
	and	[value_length],0
	mov	edx,[edi+ExpressionTerm.metadata]
	test	edx,edx
	jz	output_offset_ok
	mov	ecx,4
	mov	edi,value_length
	call	fit_value
	jc	output_offset_out_of_range
	js	output_offset_out_of_range
      output_offset_ok:
	clc
	retn
      output_offset_out_of_range:
	stc
	retn
    store_at_output_offset:
	call	read_output_offset
	jc	value_out_of_range
	mov	ecx,[value_length]
	mov	edx,assembly_workspace
	mov	edi,[edx+Workspace.memory_start]
	call	reserve_workspace
	call	get_kept_value
	mov	ecx,[value_length]
	call	fit_value
	jc	value_out_of_range
	push	esi
	mov	esi,[assembly_workspace.memory_start]
	call	rewrite_output
	pop	esi
	cmp	[value_length],0
	jne	store_out_of_area
	jmp	instruction_assembled

display_data:
	call	get_constant_value
	cmp	al,30h
	je	display_byte
	cmp	al,22h
	jne	invalid_argument
	test	[trace_mode],TRACE_DISPLAY
	jnz	bypass_display_buffer
	cmp	[next_pass_needed],0
	jne	display_next_value
	mov	ecx,[edx]
	call	reserve_display_buffer
	mov	ebx,esi
	mov	esi,edx
	lodsd
	mov	ecx,eax
	rep	movsb
	mov	esi,ebx
    display_next_value:
	call	get_constituent_value
	jc	assembly_line
	cmp	al,','
	jne	invalid_argument
	jmp	display_data
    bypass_display_buffer:
	mov	ebx,esi
	mov	esi,edx
	lodsd
	mov	ecx,eax
	jecxz	display_next_bypassing_value
	call	display_string
      display_next_bypassing_value:
	mov	esi,ebx
	jmp	display_next_value
    display_byte:
	test	[trace_mode],TRACE_DISPLAY
	jnz	byte_bypassing_display_buffer
	cmp	[next_pass_needed],0
	jne	display_next_value
	mov	ecx,1
	call	reserve_display_buffer
	mov	ecx,1
	call	fit_value
	jc	value_out_of_range
	jmp	display_next_value
    byte_bypassing_display_buffer:
	mov	edi,displayed_byte
	mov	ecx,1
	call	fit_value
	jc	value_out_of_range
	mov	ebx,esi
	mov	esi,edi
	mov	ecx,1
	call	display_string
	mov	esi,ebx
	jmp	display_next_value
    reserve_display_buffer:
	mov	edi,[display_data_length]
	add	ecx,edi
	add	edi,[display_buffer]
	mov	[display_data_length],ecx
	cmp	ecx,[display_buffer_length]
	jbe	display_buffer_reserve_ok
	mov	eax,[display_buffer]
	sub	edi,eax
	push	edx
	call	grow_stack
	pop	edx
	add	edi,eax
	mov	[display_buffer],eax
	mov	[display_buffer_length],ecx
    display_buffer_reserve_ok:
	retn

format_directive:
	mov	edx,[ebx+SymbolTree_Leaf.branch]
	call	get_symbol_namespace
	and	[symbol_definition],0
	mov	dl,SYMCLASS_INSTRUCTION
	call	identify_symbol_in_namespace
	jc	invalid_argument
	test	ebx,ebx
	jz	invalid_argument
	mov	al,[ebx+SymbolTree_Leaf.class]
	cmp	al,SYMCLASS_INSTRUCTION
	jne	invalid_argument
	call	get_available_value
	jc	invalid_argument
	jmp	prefixed_directive
format_binary:
	call	get_constituent_value
	jc	instruction_assembled
	cmp	al,1Ah
	jne	invalid_argument
	test	edx,edx
	jz	invalid_argument
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_PREPOSITION
	jne	invalid_argument
	cmp	[edx+ValueDefinition.value],PREPOSITION_AS
	jne	invalid_argument
	cmp	[output_extension],0
	jne	repeated_declaration
	call	get_constant_value
	cmp	al,22h
	jne	invalid_argument
	mov	edi,esi
	mov	ecx,[edx]
	lea	esi,[edx+4]
	mov	[output_extension_length],ecx
	mov	ebx,[auxiliary_output_areas]
	call	get_from_map
	jnc	extension_stored
	xor	eax,eax
	mov	ecx,[output_extension_length]
	call	put_into_map
    extension_stored:
	mov	[output_extension],esi
    validate_extension:
	mov	ebx,characters
    scan_extension:
	test	ecx,ecx
	jz	extension_valid
	lodsb
	xlatb
	test	al,al
	jz	invalid_argument
	dec	ecx
	jmp	scan_extension
    extension_valid:
	mov	esi,edi
	jmp	instruction_assembled

custom_error:
	mov	edi,[assembly_workspace.memory_start]
	add	edi,4
    create_custom_error_message:
	push	edi
	call	get_constant_value
	pop	edi
	cmp	al,30h
	je	attach_byte_to_error_message
	cmp	al,22h
	jne	invalid_argument
	mov	ebx,edx
	mov	ecx,[edx]
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	edx,esi
	lea	esi,[ebx+4]
	mov	ecx,[ebx]
	rep	movsb
	mov	esi,edx
	jmp	collect_next_part_of_message
    attach_byte_to_error_message:
	mov	ebx,edx
	mov	ecx,1
	mov	edx,assembly_workspace
	call	reserve_workspace
	mov	edx,ebx
	mov	ecx,1
	call	fit_value
	jc	value_out_of_range
	inc	edi
    collect_next_part_of_message:
	push	edi
	call	get_constituent_value
	pop	edi
	jc	custom_error_message_ready
	cmp	al,','
	jne	invalid_argument
	jmp	create_custom_error_message
    custom_error_message_ready:
	mov	edx,[assembly_workspace.memory_start]
	sub	edi,edx
	sub	edi,4
	mov	[edx],edi
	call	register_volatile_error
	test	edx,edx
	jz	instruction_assembled
	or	[edx+Error.flags],ERR_CUSTOM
	jmp	instruction_assembled

choose_to_remove_comments:
	and	[preprocessing_mode],not PMODE_RETAIN_COMMENTS
	jmp	instruction_assembled
choose_to_retain_comments:
	or	[preprocessing_mode],PMODE_RETAIN_COMMENTS
	jmp	instruction_assembled
choose_to_combine_lines:
	and	[preprocessing_mode],not PMODE_ISOLATE_LINES
	jmp	instruction_assembled
choose_to_isolate_lines:
	or	[preprocessing_mode],PMODE_ISOLATE_LINES
	jmp	instruction_assembled

missing_argument:
	mov	edx,_missing_argument
	call	register_error
	jmp	assembly_line

invalid_argument:
	mov	edx,_invalid_argument
	call	register_error
	jmp	assembly_line

invalid_identifier:
	mov	edx,_invalid_identifier
	call	register_error
	jmp	assembly_line

value_out_of_range:
	mov	edx,_value_out_of_range
	call	register_error
	jmp	assembly_line

missing_closing_parenthesis:
	mov	edx,_missing_closing_parenthesis
	call	register_error
	jmp	assembly_line

unexpected_instruction:
	mov	edx,_unexpected_instruction
	call	register_error
	jmp	assembly_line

repeated_declaration:
	mov	edx,_repeated_declaration
	call	register_error
	jmp	assembly_line
