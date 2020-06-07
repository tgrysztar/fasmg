
COND_NEGATED = 1			; only with COND_EVALUATE
COND_TRUE = 1				; only with COND_DETERMINED
COND_EVALUATE = 2
COND_DETERMINED = 4

get_condition_value:
; in:
;  esi = pointer into preprocessed line or the last embedded line
; out:
;  esi = pointer advanced past the processed line
;  al = logical value
	mov	ebx,[condition_stack_base]
	mov	[condition_stack],ebx
	mov	byte [ebx],COND_EVALUATE
    get_logical_value:
	call	peek_at_constituent_value
	cmp	al,'~'
	jne	negation_registered
	and	[current_constituent],0
	mov	ebx,[condition_stack]
	mov	al,[ebx]
	test	byte [ebx],COND_EVALUATE
	jz	get_logical_value
	xor	byte [ebx],COND_NEGATED
	jmp	get_logical_value
    negation_registered:
	call	parse_logical_value
	setnc	dh
	mov	dl,al
	jecxz	get_logical_operator
	mov	ebx,[condition_stack]
	lea	eax,[ebx+ecx+1]
	cmp	eax,[condition_stack_end]
	jbe	condition_stack_ready
	push	ecx edx
	mov	ecx,eax
	mov	eax,[condition_stack_base]
	sub	ecx,eax
	sub	ebx,eax
	call	grow_stack
	mov	[condition_stack_base],eax
	add	ebx,eax
	mov	[condition_stack],ebx
	add	eax,ecx
	mov	[condition_stack_end],eax
	pop	edx ecx
    condition_stack_ready:
	xor	al,al
	test	byte [ebx],COND_EVALUATE
	jz	store_opening_parentheses
	or	al,COND_EVALUATE
    store_opening_parentheses:
	inc	ebx
	mov	[ebx],al
	loop	store_opening_parentheses
	mov	[condition_stack],ebx
	cmp	dx,'~'
	je	get_logical_value
    get_logical_operator:
	call	peek_at_constituent_value
	jc	end_of_logical_expression
	cmp	al,')'
	je	close_logical_subexpression
	cmp	al,'|'
	je	logical_or
	cmp	al,'&'
	jne	end_of_logical_expression
    logical_and:
	and	[current_constituent],0
	mov	ebx,[condition_stack]
	mov	al,[ebx]
	test	al,COND_EVALUATE
	jnz	evaluate_logical_and
	test	al,COND_DETERMINED
	jz	get_logical_value
	test	al,COND_TRUE
	jz	determined_false
	jmp	continue_evaluation
    evaluate_logical_and:
	test	al,COND_NEGATED
	jnz	evaluate_negated_logical_and
	call	evaluate_logical_value
	mov	ebx,[condition_stack]
	test	al,al
	jnz	continue_evaluation
    determined_false:
	mov	byte [ebx],COND_DETERMINED
	jmp	get_logical_value
    evaluate_negated_logical_and:
	call	evaluate_logical_value
	mov	ebx,[condition_stack]
	test	al,al
	jnz	determined_false
    continue_evaluation:
	mov	byte [ebx],COND_EVALUATE
	jmp	get_logical_value
    logical_or:
	and	[current_constituent],0
	mov	ebx,[condition_stack]
	mov	al,[ebx]
	test	al,COND_EVALUATE
	jnz	evaluate_logical_or
	test	al,COND_DETERMINED
	jz	get_logical_value
	test	al,COND_TRUE
	jnz	determined_true
	jmp	continue_evaluation
    evaluate_logical_or:
	test	al,COND_NEGATED
	jnz	evaluate_negated_logical_or
	call	evaluate_logical_value
	mov	ebx,[condition_stack]
	test	al,al
	jnz	determined_true
	jmp	continue_evaluation
    evaluate_negated_logical_or:
	call	evaluate_logical_value
	mov	ebx,[condition_stack]
	test	al,al
	jnz	continue_evaluation
    determined_true:
	mov	byte [ebx],COND_DETERMINED + COND_TRUE
	jmp	get_logical_value
    close_logical_subexpression:
	and	[current_constituent],0
	mov	ebx,[condition_stack]
	cmp	ebx,[condition_stack_base]
	je	excess_parenthesis
	mov	al,[ebx]
	dec	ebx
	mov	[condition_stack],ebx
	test	al,COND_DETERMINED
	jnz	subexpression_determined
	cmp	al,COND_EVALUATE + COND_NEGATED
	jne	get_logical_operator
	test	byte [ebx],COND_EVALUATE
	jz	get_logical_operator
	xor	byte [ebx],COND_NEGATED
	jmp	get_logical_operator
    subexpression_determined:
	test	al,COND_TRUE
	jnz	subexpression_determined_true
	mov	[comparator],evaluated_false
	jmp	get_logical_operator
    subexpression_determined_true:
	mov	[comparator],evaluated_true
	jmp	get_logical_operator
    end_of_logical_expression:
	mov	ebx,[condition_stack]
	cmp	ebx,[condition_stack_base]
	jne	missing_parenthesis
	mov	al,[ebx]
	test	al,COND_DETERMINED
	jnz	condition_determined
	test	al,COND_NEGATED
	jz	evaluate_logical_value
	call	evaluate_logical_value
	test	al,al
	setz	al
	retn
    condition_determined:
	and	al,COND_TRUE
	retn
    excess_parenthesis:
	mov	edx,_excess_closing_parenthesis
	call	register_error
	jmp	unknown_condition
    missing_parenthesis:
	mov	edx,_missing_closing_parenthesis
	call	register_error
    unknown_condition:
	xor	al,al
	retn

parse_logical_value:
; in:
;  esi = pointer into preprocessed line or the last embedded line
; out:
;  [comparator] - evaluating routine
;  [expression_workspace.memory_start] - parsed argument sequences
;  [expression_end] - end of the parsed argument sequences
;  esi = pointer advanced past the parsed value
;  al = special character that follows parsed value, zero if no more symbols in line
;  ecx = number of parentheses opened before the value that did not get closed
;  cf set if value was empty
	mov	edi,[expression_workspace.memory_start]
	xor	eax,eax
	mov	[comparator],eax
	or	[leave_opening_parentheses],1
	call	parse_expression
	mov	[initial_parentheses],ecx
	mov	[expression_end],edi
	call	peek_at_constituent_value
	jc	end_of_line
	cmp	al,'&'
	je	end_of_logical_value
	cmp	al,'|'
	je	end_of_logical_value
	cmp	al,'~'
	je	end_of_logical_value
	cmp	al,')'
	je	end_of_logical_value
	cmp	al,1Ah
	je	identify_comparator
	cmp	al,'='
	je	parse_equal
	cmp	al,'<'
	je	parse_less
	cmp	al,'>'
	je	parse_greater
	jmp	end_of_logical_value
    identify_comparator:
	test	edx,edx
	jz	end_of_logical_value
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_COMPARATOR
	jne	end_of_logical_value
	and	[current_constituent],0
	mov	eax,[edx+ValueDefinition.value]
	jmp	set_comparator
    parse_less:
	and	[current_constituent],0
	call	warp_to_next_symbol
	jc	parse_less_than
	test	ecx,ecx
	jnz	parse_less_than
	call	peek_at_constituent_value
	jc	parse_less_than
	cmp	al,'='
	je	parse_less_or_equal
	cmp	al,'>'
	je	parse_not_equal
    parse_less_than:
	mov	eax,check_if_less
	jmp	set_comparator
    parse_less_or_equal:
	and	[current_constituent],0
	mov	eax,check_if_not_greater
	jmp	set_comparator
    parse_not_equal:
	and	[current_constituent],0
	mov	eax,check_if_not_equal
	jmp	set_comparator
    parse_greater:
	and	[current_constituent],0
	call	warp_to_next_symbol
	jc	parse_greater_than
	test	ecx,ecx
	jnz	parse_greater_than
	call	peek_at_constituent_value
	jc	parse_greater_than
	cmp	al,'='
	je	parse_greater_or_equal
    parse_greater_than:
	mov	eax,check_if_greater
	jmp	set_comparator
    parse_greater_or_equal:
	and	[current_constituent],0
	mov	eax,check_if_not_less
	jmp	set_comparator
    parse_equal:
	and	[current_constituent],0
	mov	eax,check_if_equal
    set_comparator:
	mov	edi,[expression_end]
	mov	[comparator],eax
	and	[leave_opening_parentheses],0
	call	parse_expression
	mov	[expression_end],edi
	call	peek_at_constituent_value
	jnc	end_of_logical_value
    end_of_line:
	xor	al,al
    end_of_logical_value:
	mov	ecx,[initial_parentheses]
	cmp	[comparator],0
	jnz	logical_value_not_empty
	mov	[comparator],check_if_not_zero
	mov	ebx,[expression_workspace.memory_start]
	cmp	dword [ebx],0
	jne	logical_value_not_empty
	stc
	retn
    logical_value_not_empty:
	clc
	retn

evaluate_logical_value:
; in:
;  [comparator] - evaluating routine
;  [expression_workspace.memory_start] - parsed argument sequences
; out: al = logical value
; note: evaluates value prepared by previous call to parse_logical_value
; preserves: esi
	push	esi
	mov	esi,[expression_workspace.memory_start]
	jmp	[comparator]
    evaluated_false:
	pop	esi
	xor	al,al
	retn
    evaluated_true:
	pop	esi
	mov	al,1
	retn

evaluate_stored_logical_value:
; in: esi - pointer to evaluating routine followed by parsed argument sequences
; out: al = logical value
; preserves: esi
	push	esi
	lodsd
	jmp	eax

invalid_logical_value:
	mov	edx,_invalid_expression
	call	register_error
	jmp	evaluated_false

check_if_equal:
	call	get_difference_signum
	test	al,al
	jz	evaluated_true
	jmp	evaluated_false
check_if_not_equal:
	call	get_difference_signum
	test	al,al
	jnz	evaluated_true
	jmp	evaluated_false
check_if_less:
	call	get_difference_signum
	cmp	al,0
	jl	evaluated_true
	jmp	evaluated_false
check_if_not_less:
	call	get_difference_signum
	cmp	al,0
	jnl	evaluated_true
	jmp	evaluated_false
check_if_greater:
	call	get_difference_signum
	cmp	al,0
	jg	evaluated_true
	jmp	evaluated_false
check_if_not_greater:
	call	get_difference_signum
	cmp	al,0
	jng	evaluated_true
	jmp	evaluated_false
    get_difference_signum:
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	jc	signum_error
	call	calculate_parsed_expression
	jc	signum_error
	mov	esi,subtraction_operator
	call	calculate_parsed_expression
	call	pop_terms
	mov	eax,edi
	jnc	check_difference_for_variable_terms
      signum_error:
	xor	al,al
	retn
      check_difference_for_variable_terms:
	add	eax,sizeof.ExpressionTerm
	cmp	[eax+ExpressionTerm.attributes],0
	je	difference_terms_ok
	cmp	[eax+ExpressionTerm.metadata],0
	je	check_difference_for_variable_terms
	mov	edx,_values_not_comparable
	call	register_error
      difference_terms_ok:
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	get_float_signum
	call	get_numeric_term_value
	mov	ecx,1
	call	fit_value
	js	signum_negative
	jc	signum_positive
	xor	al,al
	cmp	[edi],al
	jne	signum_positive
	retn
      get_float_signum:
	call	get_term_value
	mov	esi,edx
	call	get_float_exponent
	jz	signum_zero
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	jnz	signum_negative
      signum_positive:
	mov	al,1
	retn
      signum_negative:
	or	al,-1
	retn
      signum_zero:
	xor	al,al
	retn

check_if_not_zero:
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	jc	evaluated_false
	call	pop_terms
	jc	evaluated_false
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	check_if_not_zero_float
	call	get_numeric_term_value
	mov	ecx,1
	call	fit_value
	js	evaluated_true
	jc	evaluated_true
	cmp	byte [edi],0
	jne	evaluated_true
    check_if_has_variable_terms:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	evaluated_false
	cmp	[edi+ExpressionTerm.metadata],0
	je	check_if_has_variable_terms
	jmp	evaluated_true
    check_if_not_zero_float:
	call	get_term_value
	mov	esi,edx
	call	get_float_exponent
	jz	check_if_has_variable_terms
	jmp	evaluated_true

check_if_relative:
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	jc	evaluated_false
	call	calculate_parsed_expression
	jc	evaluated_false
	mov	ebx,edi
	call	pop_terms
	jc	evaluated_false
	mov	edx,edi
	call	pop_terms
	jc	evaluated_false
	cmp	[edx+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	je	check_difference_terms
	xchg	edi,edx
	cmp	[edx+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	je	check_difference_terms
	mov	esi,subtraction_operator
	mov	edi,ebx
	call	calculate_parsed_expression
	call	pop_terms
	jc	evaluated_false
    check_difference_terms:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	evaluated_true
	cmp	[edi+ExpressionTerm.metadata],0
	je	check_difference_terms
	jmp	evaluated_false
    undefined_condition:
	mov	edx,_invalid_value
	call	register_error
	jmp	evaluated_false

check_if_type_equal:
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	jc	evaluated_false
	call	pop_terms
	jc	evaluated_false
	mov	eax,[edi+ExpressionTerm.attributes]
	mov	[result_type],al
	call	calculate_parsed_expression
	jc	evaluated_false
	call	pop_terms
	jc	evaluated_false
	mov	eax,[edi+ExpressionTerm.attributes]
	cmp	al,[result_type]
	je	evaluated_true
	jmp	evaluated_false

check_if_value_equal:
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	jc	evaluated_false
	mov	ebx,edi
	call	pop_terms
	jc	evaluated_false
	mov	eax,[edi+ExpressionTerm.attributes]
	mov	[result_type],al
	mov	edi,ebx
	call	calculate_parsed_expression
	jc	evaluated_false
	mov	ebx,edi
	call	pop_terms
	jc	evaluated_false
	mov	eax,[edi+ExpressionTerm.attributes]
	cmp	al,[result_type]
	jne	evaluated_false
	cmp	al,EXPR_STRING
	jne	compare_values_numerically
	call	get_term_value
	mov	ecx,[edx]
	call	pop_terms
	call	get_term_value
	cmp	ecx,[edx]
	jne	evaluated_false
      compare_values_numerically:
	mov	esi,subtraction_operator
	mov	edi,ebx
	call	calculate_parsed_expression
	call	pop_terms
	jc	evaluated_false
	mov	eax,edi
      check_if_terms_equal:
	add	eax,sizeof.ExpressionTerm
	cmp	[eax+ExpressionTerm.attributes],0
	je	check_if_constant_term_equal
	cmp	[eax+ExpressionTerm.metadata],0
	je	check_if_terms_equal
	jmp	evaluated_false
      check_if_constant_term_equal:
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	check_if_float_equal
	call	get_numeric_term_value
	xor	ecx,ecx
	xor	edi,edi
	call	fit_value
	jc	evaluated_false
	jmp	evaluated_true
      check_if_float_equal:
	call	get_term_value
	mov	esi,edx
	call	get_float_exponent
	jnz	evaluated_false
	jmp	evaluated_true

check_if_defined:
	xor	ecx,ecx
	jmp	check_if_expression_defined
check_if_defined_earlier:
	mov	ecx,[current_pass]
    check_if_expression_defined:
	and	[outer_expression],0
	and	[defined_element],0
	lodsd
	test	eax,eax
	jnz	checked_expression_invalid
    check_expression_element:
	lodsd
	test	eax,eax
	jz	check_expression_end
	cmp	al,EXPR_SYMBOL
	je	check_if_subexpression_defined
	cmp	al,EXPR_SYMBOL_VALUE
	je	check_if_symbol_defined
	or	[defined_element],1
	add	esi,4
	cmp	al,EXPR_NUMBER
	je	check_expression_element
	cmp	al,EXPR_STRING
	je	check_expression_element
	cmp	al,EXPR_FLOAT
	je	check_expression_element
	cmp	al,EXPR_OPERATOR
	je	check_expression_element
    checked_expression_invalid:
	jmp	invalid_logical_value
    check_expression_end:
	xor	esi,esi
	xchg	esi,[outer_expression]
	test	esi,esi
	jnz	check_expression_element
	jecxz	checked_expression_defined
	cmp	[defined_element],0
	je	checked_expression_invalid
    checked_expression_defined:
	jmp	evaluated_true
    check_if_symbol_defined:
	lodsd
	test	eax,eax
	jz	evaluated_false
	lodsd
	test	eax,eax
	jz	evaluated_false
    check_symbol_value:
	or	[defined_element],1
	jecxz	check_expression_element
	test	[eax+ValueDefinition.flags],VAL_INTERNAL
	jnz	check_expression_element
	cmp	[eax+ValueDefinition.pass],ecx
	jne	evaluated_false
	jmp	check_expression_element
    check_if_subexpression_defined:
	mov	ebx,[esi]
	test	ebx,ebx
	jz	evaluated_false
	add	esi,4
	call	get_available_value
	mov	eax,edx
	test	eax,eax
	jz	evaluated_false
	cmp	[eax+ValueDefinition.type],VALTYPE_SYMBOLIC
	jne	check_symbol_value
	mov	[outer_expression],esi
	push	ecx
	call	get_subexpression
	pop	ecx
	jnc	invalid_logical_value
	jmp	check_expression_element
    get_subexpression:
	call	clear_line_embeddings
	xor	esi,esi
	xor	ecx,ecx
	call	embed_symbolic_value
	mov	edi,[expression_workspace.memory_start]
	and	[leave_opening_parentheses],0
	call	parse_expression
	call	get_constituent_value
	mov	esi,[expression_workspace.memory_start]
	retn

check_if_used:
	lodsd
	test	eax,eax
	jnz	invalid_logical_value
   check_if_expression_is_used_symbol:
	lodsd
	cmp	al,EXPR_SYMBOL
	je	check_for_indirect_symbol
	cmp	al,EXPR_SYMBOL_VALUE
	jne	invalid_logical_value
	lodsd
	mov	ebx,eax
	lodsd
   check_if_used_symbol:
	lodsd
	test	eax,eax
	jnz	invalid_logical_value
	test	ebx,ebx
	jz	evaluated_false
	mov	ecx,[ebx+SymbolTree_Leaf.last_use_pass]
	jecxz	symbol_predicted_unused
	mov	eax,[current_pass]
	sub	eax,ecx
	jz	evaluated_true
	cmp	eax,1
	ja	symbol_predicted_unused
   symbol_predicted_used:
	or	[ebx+SymbolTree_Leaf.flags],SYM_USAGE_PREDICTED + SYM_PREDICTED_USED
	jmp	evaluated_true
   symbol_predicted_unused:
	mov	al,[ebx+SymbolTree_Leaf.flags]
	or	al,SYM_USAGE_PREDICTED
	and	al,not SYM_PREDICTED_USED
	mov	[ebx+SymbolTree_Leaf.flags],al
	jmp	evaluated_false
   check_for_indirect_symbol:
	lodsd
	mov	ebx,eax
	call	get_available_value
	test	edx,edx
	jz	check_if_used_symbol
	cmp	[edx+ValueDefinition.type],VALTYPE_SYMBOLIC
	jne	check_if_used_symbol
	call	get_subexpression
	jnc	invalid_logical_value
	jmp	check_if_expression_is_used_symbol
