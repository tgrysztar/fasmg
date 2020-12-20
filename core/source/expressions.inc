
struct ExpressionTerm
	attributes dd ? 		; EXPR_# in low byte, plus any EXPRF_#
	metadata dd ?
	value dd ?
ends

EXPR_NUMBER = 30h
EXPR_STRING = 22h
EXPR_FLOAT = 2Eh

EXPR_SYMBOL_VALUE = 10h
EXPR_AREA_ADDRESS = 11h
EXPR_OPERATOR = 20h
EXPR_SYMBOL = 40h

EXPR_MISSING_ARGUMENT = 29h
EXPR_MISSING_PARENTHESIS = 28h

EXPRF_VALUE_IN_WORKSPACE = 100h
EXPRF_CALM_LITERAL = 200h

convert_number:
; in:
;  edx - 32-bit length followed by a string of that length
; out:
;  cf set when number has no known prefix or suffix but is not a plain decimal number
;  when cf = 0:
;   edx - 32-bit length followed by binary data of that length, null when string did not represent a valid number
;  when cf = 1:
;   edx preserved
; preserves: esi
	mov	edi,[value_workspace.memory_start]
	add	edi,[value_position]
	mov	ecx,[edx]
	add	edx,4
	cmp	byte [edx],'$'
	je	pascal_hexadecimal_number
	cmp	ecx,2
	jb	check_number_suffix
	cmp	word [edx],'0x'
	je	c_hexadecimal_number
    check_number_suffix:
	movzx	eax,byte [edx+ecx-1]
	mov	al,[characters+eax]
	cmp	al,'h'
	je	suffixed_hexadecimal_number
	cmp	al,'b'
	je	suffixed_binary_number
	cmp	al,'o'
	je	suffixed_octal_number
	cmp	al,'q'
	je	suffixed_octal_number
	cmp	al,'d'
	jne	decimal_number
	or	ah,1
	dec	ecx
	jz	invalid_number
    decimal_number:
	mov	ebx,ecx
      check_decimal_digits:
	mov	al,[edx+ebx-1]
	cmp	al,27h
	je	check_next_decimal_digit
	cmp	al,'_'
	je	check_next_decimal_digit
	cmp	al,'0'
	jb	unknown_number
	cmp	al,'9'
	ja	unknown_number
      check_next_decimal_digit:
	dec	ebx
	jnz	check_decimal_digits
	push	ecx edx
	add	ecx,12
	mov	edx,value_workspace
	call	reserve_workspace
	xor	eax,eax
	mov	dword [edi+4],eax
	add	eax,4
	mov	dword [edi],eax
	pop	edx ebx
    convert_decimal_number:
	mov	al,[edx]
	cmp	al,27h
	je	skip_decimal_digit
	cmp	al,'_'
	je	skip_decimal_digit
	sub	al,'0'
	mov	ecx,[edi]
	xor	ecx,ecx
      add_decimal_digit:
	add	ecx,4
	movzx	eax,al
	add	[edi+ecx],eax
	setc	al
	sets	ah
	cmp	ecx,[edi]
	jb	add_decimal_digit
	or	ah,al
	jz	decimal_digit_added
	xor	ah,ah
	add	ecx,4
	mov	[edi+ecx],eax
	mov	[edi],ecx
      decimal_digit_added:
	dec	ebx
	jz	decimal_number_converted
      next_decimal_digit:
	inc	edx
	push	ebx edx
	mov	ebx,10
	xor	ecx,ecx
	xor	edx,edx
      multiply_decimal_number:
	add	ecx,4
	mov	eax,edx
	xchg	eax,[edi+ecx]
	mul	ebx
	add	[edi+ecx],eax
	cmp	ecx,[edi]
	jb	multiply_decimal_number
	test	edx,edx
	jz	decimal_number_multiplied
	add	ecx,4
	mov	[edi+ecx],edx
	mov	[edi],ecx
      decimal_number_multiplied:
	pop	edx ebx
	xor	eax,eax
	jmp	convert_decimal_number
      skip_decimal_digit:
	inc	edx
	dec	ebx
	jnz	convert_decimal_number
      decimal_number_converted:
	lea	edx,[edi+4]
	mov	ecx,[edi]
	dec	ecx
    optimize_number:
	movsx	eax,byte [edx+ecx-1]
	cmp	ah,[edx+ecx]
	jne	number_optimized
	loop	optimize_number
	cmp	byte [edx],0
	je	number_finished
      number_optimized:
	inc	ecx
      number_finished:
	sub	edx,4
	mov	[edx],ecx
	lea	edi,[edx+4+ecx]
	sub	edi,[value_workspace.memory_start]
	mov	[value_position],edi
       ; clc
	retn
    c_hexadecimal_number:
	sub	ecx,3
	jc	invalid_number
	add	edx,2
	jmp	hexadecimal_number
    pascal_hexadecimal_number:
	inc	edx
    suffixed_hexadecimal_number:
	sub	ecx,2
	jc	invalid_number
    hexadecimal_number:
	push	edx ecx
	inc	ecx
	shr	ecx,1
	add	ecx,5
	mov	edx,value_workspace
	call	reserve_workspace
	pop	ebx edx
	push	edi
	xor	eax,eax
	stosd
	mov	[edi],al
	xor	cl,cl
      hexadecimal_digit:
	movzx	eax,byte [edx+ebx]
	cmp	al,27h
	je	skip_hexadecimal_digit
	cmp	al,'_'
	je	skip_hexadecimal_digit
	mov	al,[characters+eax]
	sub	al,'0'
	jc	invalid_digit
	cmp	al,10
	jb	hexadecimal_digit_ok
	sub	al,'a'-'0'
	jc	invalid_digit
	add	al,10
	cmp	al,16
	jae	invalid_digit
      hexadecimal_digit_ok:
	shl	al,cl
	or	[edi],al
	sub	ebx,1
	jc	number_converted
	xor	cl,4
	jnz	hexadecimal_digit
	inc	edi
	mov	[edi],cl
	jmp	hexadecimal_digit
      skip_hexadecimal_digit:
	sub	ebx,1
	jnc	hexadecimal_digit
    number_converted:
	pop	edx
	inc	edi
	and	byte [edi],0
	add	edx,4
	mov	ecx,edi
	sub	ecx,edx
	jmp	optimize_number
    suffixed_binary_number:
	sub	ecx,2
	jc	invalid_number
	push	edx ecx
	add	ecx,7
	shr	ecx,3
	add	ecx,5
	mov	edx,value_workspace
	call	reserve_workspace
	pop	ebx edx
	push	edi
	xor	eax,eax
	stosd
	mov	[edi],al
	xor	cl,cl
      binary_digit:
	mov	al,[edx+ebx]
	cmp	al,27h
	je	skip_binary_digit
	cmp	al,'_'
	je	skip_binary_digit
	sub	al,'0'
	jc	invalid_digit
	cmp	al,2
	jae	invalid_digit
	shl	al,cl
	or	[edi],al
	sub	ebx,1
	jc	number_converted
	inc	cl
	and	cl,111b
	jnz	binary_digit
	inc	edi
	mov	[edi],cl
	jmp	binary_digit
      skip_binary_digit:
	sub	ebx,1
	jnc	binary_digit
	jmp	number_converted
    suffixed_octal_number:
	sub	ecx,2
	jc	invalid_number
	push	edx ecx
	inc	ecx
	shr	ecx,1
	add	ecx,5
	mov	edx,value_workspace
	call	reserve_workspace
	pop	ebx edx
	push	edi
	xor	eax,eax
	stosd
	mov	[edi],al
	xor	cl,cl
      octal_digit:
	mov	al,[edx+ebx]
	cmp	al,27h
	je	skip_octal_digit
	cmp	al,'_'
	je	skip_octal_digit
	sub	al,'0'
	jc	invalid_digit
	cmp	al,8
	jae	invalid_digit
	shl	eax,cl
	or	[edi],al
	sub	ebx,1
	jc	number_converted
	add	cl,3
	cmp	cl,8
	jb	octal_digit
	sub	cl,8
	inc	edi
	mov	[edi],ah
	xor	eax,eax
	jmp	octal_digit
      skip_octal_digit:
	sub	ebx,1
	jnc	octal_digit
	jmp	number_converted
    unknown_number:
	test	ah,ah
	jnz	invalid_number
	sub	edx,4
	stc
	retn
    invalid_digit:
	pop	edi
    invalid_number:
	xor	edx,edx
	mov	al,30h
	clc
	retn

convert_number_back:
; in:
;  edx - 32-bit length followed by binary data of that length
; out:
;  edx - 32-bit length followed by a string of that length
; note:
;  the number is treated as unsigned
;  returned string is in temporary storage and should be copied out of it immediately
	mov	edi,[value_workspace.memory_start]
	add	edi,[value_position]
	mov	ecx,[edx]
	add	ecx,2
	shl	ecx,2
	mov	esi,edx
	mov	edx,value_workspace
	call	reserve_workspace
	lodsd
	lea	ebx,[edi+4+(eax+1)*4]
	mov	edx,edi
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	xchg	eax,ecx
	stosd
	jecxz	highest_dword_offset_ok
	dec	ecx
	and	ecx,not 11b
    highest_dword_offset_ok:
	mov	esi,edx
	mov	edi,ebx
	push	edi
    obtain_digit:
	xor	edx,edx
    divide_highest_dwords:
	mov	eax,[esi+ecx]
	call	div10
	test	eax,eax
	jnz	more_digits_to_come
	sub	ecx,4
	jnc	divide_highest_dwords
	pop	ecx
	cmp	ecx,edi
	je	store_final_digit
	test	dl,dl
	jz	finish_number
    store_final_digit:
	add	dl,'0'
	dec	edi
	mov	[edi],dl
    finish_number:
	sub	ecx,edi
	sub	edi,4
	mov	[edi],ecx
	mov	edx,edi
	retn
    more_digits_to_come:
	mov	ebx,ecx
    divide_remaining_dwords:
	mov	[esi+ebx],eax
	sub	ebx,4
	jc	store_digit
	mov	eax,[esi+ebx]
	call	div10
	jmp	divide_remaining_dwords
    store_digit:
	add	dl,'0'
	dec	edi
	mov	[edi],dl
	jmp	obtain_digit
    div10:
       ; cmp	 edx,10
       ; jae	 internal_error
	push	ebx ecx
	push	eax
	mov	ebx,eax
	mov	ecx,edx
	shld	edx,eax,2
	sub	ebx,edx
	sbb	ecx,0
	mov	eax,ebx
	mov	ebx,1999999Ah
	mul	ebx
	mov	eax,ecx
	imul	eax,ebx
	add	eax,edx
	pop	edx
	imul	ecx,eax,10
	sub	edx,ecx
	cmp	edx,10
	jb	div10_done
	sub	edx,10
	inc	eax
      div10_done:
	pop	ecx ebx
	retn

fit_value:
; in:
;  edx - 32-bit length followed by numeric data
;  ecx = number of bytes that the value has to fit into
;  edi - buffer for the specified amount of bytes, null when just checking whether value fits in range
; out:
;  sf set when value is negative
;  cf set if value does not fit into extended range for specified size
;  when cf = 0:
;   edi - value fit into specified amount of bytes
; preserves: edx, esi, edi
; note: when value is zero, it fits even into zero byte range
	mov	ebx,ecx
	cmp	[edx],ecx
	jbe	term_in_range
	xor	al,al
	cmp	[edx+4+ecx],al
	je	check_redundant_bytes
	dec	al
	cmp	[edx+4+ecx],al
	jne	term_out_of_range
	jecxz	term_out_of_range
    check_redundant_bytes:
	inc	ecx
	cmp	ecx,[edx]
	je	term_in_range
	cmp	[edx+4+ecx],al
	je	check_redundant_bytes
    term_out_of_range:
	mov	ecx,[edx]
	mov	al,[edx+4+ecx-1]
	test	al,al
	stc
	retn
    term_in_range:
	test	edi,edi
	jz	value_fit
	mov	ecx,[edx]
	cmp	ecx,ebx
	jbe	copy_value
	mov	ecx,ebx
    copy_value:
	lea	eax,[edx+4]
	xchg	eax,esi
	rep	movsb
	mov	esi,eax
	mov	ecx,[edx]
	movsx	eax,byte [edx+4+ecx-1]
	mov	al,ah
	mov	ecx,ebx
	sub	ecx,[edx]
	jbe	value_ready
	rep	stosb
    value_ready:
	sub	edi,ebx
    value_fit:
	test	al,al
	clc
	retn

start_decimal_converter:
; preserves: ebx
	mov	edx,value_workspace
	mov	edi,[value_position]
	mov	[converter_position],edi
	add	edi,[edx+Workspace.memory_start]
	mov	ecx,2*sizeof.FloatData
	call	reserve_workspace
	lea	ecx,[edi+2*sizeof.FloatData]
	sub	ecx,[edx+Workspace.memory_start]
	mov	[value_position],ecx
	mov	esi,edi
	xor	eax,eax
	assert	sizeof.FloatData and 11b = 0
	mov	ecx,sizeof.FloatData shr 2
	rep	stosd
	mov	edx,edi
	mov	ecx,sizeof.FloatData shr 2
	rep	stosd
	mov	[edx+FloatData.exponent],7
	retn

convert_decimal_digit:
; in:
;  al = value of subsequent digit
;  ecx = number of zero digits to insert before
; out:
;  edi - FloatData holding the value corresponding to digits converted so far
; note: start_decimal_converter should be called before converting the first digit
	mov	esi,[value_workspace.memory_start]
	add	esi,[converter_position]
	mov	edi,esi
	shl	eax,24
	mov	[esi+sizeof.FloatData+FloatData.mantissa],eax
	jecxz	multiply_by_ten
	inc	ecx
	call	multiply_float_by_power_of_ten
	jmp	add_subsequent_digit
      multiply_by_ten:
	mov	ecx,10
	call	multiply_float_by_unsigned_int
      add_subsequent_digit:
	mov	esi,edi
	lea	ebx,[esi+sizeof.FloatData]
	call	add_floats
	retn

finish_decimal_conversion:
; out:
;  edi - FloatData holding the converted value
; preserves: ebx, ecx, esi, edi
; note: this function should not be called before all the digits of a value have been processed by convert_decimal_digit
	mov	eax,[converter_position]
	mov	edi,[value_workspace.memory_start]
	add	edi,eax
	add	eax,2*sizeof.FloatData
	cmp	eax,[value_position]
	jne	decimal_conversion_finished
	sub	eax,sizeof.FloatData
	mov	[value_position],eax
      decimal_conversion_finished:
	retn

keep_value:
; in:
;  edx - numeric or string data returned by source parsing or expression evaluating functions
; preserves: ebx, esi, edi
; note:
;  this function should be used in conjunction with get_kept_value to retain access to data
;  while calling other parsing or expression evaluating functions
	xor	al,al
	cmp	edx,[value_workspace.memory_end]
	jae	keep_value_pointer
	mov	ecx,edx
	sub	ecx,[value_workspace.memory_start]
	jc	keep_value_pointer
	mov	edx,ecx
	inc	al
    keep_value_pointer:
	mov	[kept_value],edx
	mov	[kept_value_in_workspace],al
	retn

get_kept_value:
; out:
;  edx - numeric or string data previously passed to keep_value
; preserves: eax, ebx, ecx, esi, edi
	mov	edx,[kept_value]
	cmp	[kept_value_in_workspace],0
	je	kept_value_ok
	add	edx,[value_workspace.memory_start]
    kept_value_ok:
	retn

get_constant_value:
; in:
;  esi = pointer into preprocessed line or the last embedded line
; out:
;  esi = pointer advanced past the processed part of line
;  al = type of value, zero when expression gave no result
;  when al = 22h:
;   edx - 32-bit length followed by string data
;  when al = 30h:
;   edx - 32-bit length followed by numeric data
;  when al = 2Eh:
;   edx - FloatData
	call	get_expression_value
	jc	return_empty_type
    get_calculated_constant_value:
    ; in:
    ;  edi - list of ExpressionTerm elements as returned by get_expression_value
    ; out:
    ;  same as get_constant_value
    ; preserves: edi
	call	forbid_variable_terms
	call	get_term_value
	mov	eax,[edi+ExpressionTerm.attributes]
	assert	EXPR_STRING = 22h
	assert	EXPR_NUMBER = 30h
	assert	EXPR_FLOAT = 2Eh
	retn
    return_empty_type:
	xor	al,al
	retn
    forbid_variable_terms:
	mov	eax,edi
      detect_variable_terms:
	add	eax,sizeof.ExpressionTerm
	cmp	[eax+ExpressionTerm.attributes],0
	je	expression_terms_ok
	cmp	[eax+ExpressionTerm.metadata],0
	je	detect_variable_terms
	mov	edx,_misused_variable_term
	call	register_error
      expression_terms_ok:
	retn

get_numeric_constant_value:
; in:
;  esi = pointer into preprocessed line or the last embedded line
; out:
;  esi = pointer advanced past the processed part of line
;  edx - 32-bit length followed by numeric data, null when expression gave no result
;  edi - ExpressionTerm, null when expression gave no result
	call	get_expression_value
	jc	empty_numeric_constant
	call	forbid_variable_terms
	call	get_numeric_term_value
	retn
    empty_numeric_constant:
	xor	edx,edx
	xor	edi,edi
	retn

get_expression_value:
; in:
;  esi = pointer into preprocessed line or the last embedded line
; out:
;  cf set if expression gave no result
;  esi = pointer advanced past the processed part of line
;  when cf = 0:
;   edi - list of ExpressionTerm elements, the closing element has attributes set to zero
	mov	edi,[expression_workspace.memory_start]
	and	[leave_opening_parentheses],0
	call	parse_expression
	push	esi
	mov	esi,[expression_workspace.memory_start]
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	call	pop_terms
	pop	esi
	retn

convert_terms_to_numeric_value:
; in:
;  edi - list of ExpressionTerm elements as returned by get_expression_value
; out:
;  esi - value in VALTYPE_NUMERIC format
;  ecx = length of value
; note: the returned value is placed in assembly workspace
	mov	edx,assembly_workspace
    convert_terms_to_numeric_value_in_workspace:
    ; in:
    ;  edx - Workspace
    ;  edi - list of ExpressionTerm elements as returned by get_expression_value
    ; out:
    ;  esi - value in VALTYPE_NUMERIC format (same [edx+Workspace.memory_start])
    ;  ecx = length of value
    ; preserves: edx
	mov	ebx,[edx+Workspace.memory_start]
      copy_numeric_term_values:
	mov	esi,edx
	call	get_numeric_term_value
	xchg	esi,edx
	xchg	ebx,edi
	mov	ecx,[esi]
	add	ecx,8
	call	reserve_workspace
	lodsd
	stosd
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
      next_numeric_term:
	add	ebx,sizeof.ExpressionTerm
	cmp	[ebx+ExpressionTerm.attributes],0
	je	numeric_term_values_copied
	mov	eax,[ebx+ExpressionTerm.metadata]
	test	eax,eax
	jz	next_numeric_term
	stosd
	xchg	ebx,edi
	jmp	copy_numeric_term_values
      numeric_term_values_copied:
	xor	eax,eax
	stosd
	mov	esi,[edx+Workspace.memory_start]
	mov	ecx,edi
	sub	ecx,esi
	retn

update_predicted_shift:
; in:
;  edx - ValueDefinition that is going to be updated
;  esi - new value in VALTYPE_NUMERIC fomat
; preserves: ebx, ecx, edx, esi
	test	edx,edx
	jz	zero_shift
	cmp	[edx+ValueDefinition.type],VALTYPE_NUMERIC
	jne	zero_shift
	mov	eax,[current_pass]
	dec	eax
	cmp	eax,[edx+ValueDefinition.pass]
	jne	zero_shift
	call	get_low_dword
	mov	[predicted_shift],eax
	mov	edi,[edx+ValueDefinition.value]
	xchg	esi,edi
	call	get_low_dword
	sub	[predicted_shift],eax
	test	byte [predicted_shift+3],11000000b
	mov	esi,edi
	jnp	zero_shift
	retn
    zero_shift:
	mov	[predicted_shift],0
	retn
    get_low_dword:
	mov	eax,[esi]
	cmp	eax,1
	jb	low_dword_ready
	je	dword_from_single_byte
	cmp	eax,3
	jb	dword_from_two_bytes
	je	dword_from_three_bytes
	mov	eax,[esi+4]
      low_dword_ready:
	retn
      dword_from_single_byte:
	movsx	eax,byte [esi+4]
	retn
      dword_from_two_bytes:
	movsx	eax,word [esi+4]
	retn
      dword_from_three_bytes:
	mov	eax,[esi+4-1]
	sar	eax,8
	retn

get_area_value:
; in:
;  edi - command sequence created by parse_expression
; out:
;  cf set if expression does not yield a valid and accessible VALTYPE_AREA value
;  when cf = 0:
;   edx - ValueDefinition
; preserves: esi
	cmp	byte [edi],EXPR_SYMBOL_VALUE
	jne	no_plain_area_symbol
	cmp	dword [edi+3*4],0
	jne	no_plain_area_symbol
	mov	edx,[edi+2*4]
	test	edx,edx
	jz	no_plain_area_symbol
	cmp	[edx+ValueDefinition.type],VALTYPE_AREA
	jne	no_plain_area_symbol
	mov	ebx,[edi+4]
	call	mark_symbol_as_used
	clc
	retn
      no_plain_area_symbol:
	push	esi
	mov	esi,edi
	mov	edi,[calculation_workspace.memory_start]
	call	calculate_parsed_expression
	jc	invalid_area_expression
	call	pop_terms
	jc	invalid_area_expression
	mov	eax,edi
	xor	esi,esi
      extract_variable_term:
	add	eax,sizeof.ExpressionTerm
	cmp	[eax+ExpressionTerm.attributes],0
	je	variable_term_extracted
	cmp	[eax+ExpressionTerm.metadata],0
	je	extract_variable_term
	test	esi,esi
	jnz	invalid_area_expression
	mov	esi,eax
	jmp	extract_variable_term
      variable_term_extracted:
	test	esi,esi
	jz	invalid_area_expression
	call	get_term_value
	xor	ecx,ecx
	mov	edi,ecx
	call	fit_value
	jc	invalid_area_expression
	mov	edi,esi
	call	get_term_value
	mov	ecx,1
	mov	edi,value
	call	fit_value
	jc	invalid_area_expression
	cmp	byte [value],1
	jne	invalid_area_expression
	mov	ebx,[esi+ExpressionTerm.metadata]
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	test	edx,edx
	jz	invalid_area_expression
	cmp	[edx+ValueDefinition.type],VALTYPE_AREA
	jne	invalid_area_expression
	pop	esi
	clc
	retn
      invalid_area_expression:
	pop	esi
      invalid_area_symbol:
	stc
	retn

get_constituent_value:
; same as get_processed_value
; note: after any expression evaluation functions have been used, only this function should be used to retrieve constituents of line
	xor	al,al
	xchg	al,[current_constituent]
	test	al,al
	jz	get_processed_value
    get_constituent_components:
	mov	ebx,[constituent_symbol]
	mov	edx,[constituent_value]
	mov	ecx,[constituent_whitespace]
	clc
	retn

peek_at_constituent_value:
; same as get_constituent_value, but returned values are also kept in constituent variables
; note:
;  the retrieved value is still going to be available to expression evaluation functions or to the next call of get_constituent_value
;  to consume the value it is enough to set [current_constituent] to zero
	cmp	[current_constituent],0
	jne	get_current_constituent
	call	get_processed_value
	jc	constituent_value_ok
	mov	[current_constituent],al
	mov	[constituent_symbol],ebx
	mov	[constituent_value],edx
	mov	[constituent_whitespace],ecx
    constituent_value_ok:
	retn
    get_current_constituent:
	mov	al,[current_constituent]
	jmp	get_constituent_components

parse_expression:
; in:
;  esi = pointer into preprocessed line or the last embedded line
;  edi = pointer to a place in expression workspace where the parsed command sequence should be stored
;  [leave_opening_parentheses] = non-zero when parentheses opened in the beginning of expression that did not get closed should be returned to caller instead of being registered in parsed expression
; out:
;  esi = pointer advanced past the processed part of line
;  edi = pointer advanced past the created sequence in expression workspace
;  ecx = number of parentheses opened in the beginning of expression that did not get closed (zero when [leave_opening_parentheses] was zero)
	mov	[expression_end],edi
	sub	edi,[expression_workspace.memory_start]
	mov	[expression_position],edi
	mov	eax,[operator_stack_base]
	mov	[operator_stack],eax
	mov	[operator_stack_position],eax
	mov	[operator_argument_expected],1
    get_expression_element:
	mov	edi,[expression_end]
	mov	edx,expression_workspace
	mov	ecx,[operator_stack_position]
	sub	ecx,[operator_stack_base]
	add	ecx,16
	call	reserve_workspace
	mov	[expression_end],edi
	call	peek_at_constituent_value
	mov	edi,[expression_end]
	jc	expression_line_end
	cmp	al,'('
	je	open_subexpression
	cmp	al,')'
	je	close_subexpression
	cmp	al,30h
	je	store_expression_number
	cmp	al,22h
	je	store_expression_string
	cmp	al,2Eh
	je	store_expression_float
	cmp	al,1Ah
	je	expression_symbol
	movzx	eax,al
	shl	eax,2
	add	eax,[operator_table]
	mov	edx,[eax]
	test	edx,edx
	jz	terminate_expression
	xor	ebx,ebx
	mov	ecx,edx
	jmp	identify_operator
    expression_symbol:
	test	edx,edx
	jz	store_expression_symbol
	mov	ecx,edx
    identify_operator:
	cmp	[ecx+ValueDefinition.type],VALTYPE_NATIVE_COMMAND
	jne	inspect_expression_symbol
	test	[ecx+ValueDefinition.flags],VAL_UNARY
	setnz	al
	xor	al,[operator_argument_expected]
	jz	store_expression_operator
	mov	ecx,[ecx+ValueDefinition.previous]
	test	ecx,ecx
	jnz	identify_operator
    inspect_expression_symbol:
	cmp	[edx+ValueDefinition.type],VALTYPE_NATIVE_COMPARATOR
	je	terminate_expression
    store_expression_symbol:
	cmp	[operator_argument_expected],0
	je	terminate_expression
	mov	eax,EXPR_SYMBOL_VALUE
	stosd
	mov	eax,ebx
	stosd
	mov	eax,edx
	stosd
    expression_symbol_stored:
	mov	[expression_end],edi
	and	[operator_argument_expected],0
	and	[current_constituent],0
	jmp	get_expression_element
    store_expression_number:
	cmp	[operator_argument_expected],0
	je	terminate_expression
	mov	eax,EXPR_NUMBER
    value_pointer_in_expression:
	cmp	edx,[value_workspace.memory_end]
	jae	value_pointer_in_expression_ok
	mov	ebx,edx
	sub	ebx,[value_workspace.memory_start]
	jc	value_pointer_in_expression_ok
	mov	edx,ebx
	or	eax,EXPRF_VALUE_IN_WORKSPACE
    value_pointer_in_expression_ok:
	stosd
	mov	eax,edx
	stosd
	jmp	expression_symbol_stored
    store_expression_string:
	cmp	[operator_argument_expected],0
	je	terminate_expression
	mov	eax,EXPR_STRING
	jmp	value_pointer_in_expression
    store_expression_float:
	cmp	[operator_argument_expected],0
	je	terminate_expression
	mov	eax,EXPR_FLOAT
	jmp	value_pointer_in_expression
    store_expression_operator:
	mov	edx,[ecx+ValueDefinition.value]
	mov	cl,[ecx+ValueDefinition.attribute]
	mov	ebx,[operator_stack_position]
	cmp	[operator_argument_expected],0
	jne	push_operator
    establish_operator_precedence:
	cmp	ebx,[operator_stack]
	je	push_operator
	mov	ch,cl
	and	cl,OPERATOR_PRECEDENCE_MASK
	cmp	cl,[ebx-8]
	ja	push_operator
	jb	store_pending_operator
	test	ch,OPERATOR_RIGHT_ASSOCIATIVE
	jnz	push_operator
    store_pending_operator:
	sub	ebx,8
	mov	eax,EXPR_OPERATOR
	stosd
	mov	eax,[ebx+4]
	stosd
	jmp	establish_operator_precedence
    push_operator:
	call	reserve_operator_stack
	mov	[ebx],cl
	mov	[ebx+4],edx
	add	ebx,8
	mov	[operator_stack_position],ebx
	mov	[expression_end],edi
	or	[operator_argument_expected],1
	and	[current_constituent],0
	jmp	get_expression_element
    open_subexpression:
	cmp	[operator_argument_expected],0
	je	terminate_expression
	mov	ebx,[operator_stack_position]
	call	reserve_operator_stack
	mov	eax,[operator_stack]
	sub	eax,[operator_stack_base]
	mov	[ebx],eax
	add	ebx,4
	mov	[operator_stack],ebx
	mov	[operator_stack_position],ebx
	and	[current_constituent],0
	jmp	get_expression_element
    close_subexpression:
	mov	eax,[operator_stack]
	cmp	eax,[operator_stack_base]
	je	terminate_expression
	cmp	[operator_argument_expected],0
	je	subexpression_closed
	mov	eax,EXPR_MISSING_ARGUMENT
	stosd
    subexpression_closed:
	call	store_remaining_operators
	mov	[expression_end],edi
	sub	ebx,4
	mov	eax,[ebx]
	add	eax,[operator_stack_base]
	mov	[operator_stack],eax
	mov	[operator_stack_position],ebx
	and	[current_constituent],0
	jmp	get_expression_element
    expression_line_end:
	and	[current_constituent],0
    terminate_expression:
	cmp	[operator_argument_expected],0
	je	close_expression
	mov	eax,[operator_stack_position]
	cmp	eax,[operator_stack]
	jne	expression_terminated_prematurely
	mov	eax,edi
	sub	eax,[expression_workspace.memory_start]
	cmp	eax,[expression_position]
	je	close_expression
    expression_terminated_prematurely:
	mov	eax,EXPR_MISSING_ARGUMENT
	stosd
    close_expression:
	call	store_remaining_operators
	xor	ecx,ecx
	cmp	ebx,[operator_stack_base]
	jne	forcibly_close_subexpressions
    expression_parsed:
	xor	eax,eax
	stosd
	retn
    forcibly_close_subexpressions:
	sub	ebx,4
	mov	eax,[ebx]
	add	eax,[operator_stack_base]
	mov	[operator_stack],eax
	cmp	[leave_opening_parentheses],0
	je	internal_parenthesis
	cmp	eax,ebx
	je	external_parenthesis
    internal_parenthesis:
	mov	eax,EXPR_MISSING_PARENTHESIS
	inc	ecx
	rep	stosd
	mov	[operator_stack_position],ebx
	call	store_remaining_operators
	cmp	ebx,[operator_stack_base]
	jne	forcibly_close_subexpressions
	jmp	expression_parsed
    external_parenthesis:
	inc	ecx
	cmp	ebx,[operator_stack_base]
	jne	forcibly_close_subexpressions
	jmp	expression_parsed
    reserve_operator_stack:
	mov	eax,[operator_stack_end]
	sub	eax,8
	cmp	ebx,eax
	jbe	operator_stack_ready
	push	edx
	mov	eax,[operator_stack_base]
	sub	ebx,eax
	sub	[operator_stack],eax
	mov	ecx,[operator_stack_end]
	sub	ecx,eax
	add	ecx,8
	call	grow_stack
	add	ebx,eax
	add	[operator_stack],eax
	mov	[operator_stack_base],eax
	add	eax,ecx
	mov	[operator_stack_end],eax
	pop	edx
    operator_stack_ready:
	retn
    store_remaining_operators:
	mov	ebx,[operator_stack_position]
    store_operator:
	cmp	ebx,[operator_stack]
	je	remaining_operators_stored
	sub	ebx,8
	mov	eax,EXPR_OPERATOR
	stosd
	mov	eax,[ebx+4]
	stosd
	jmp	store_operator
    remaining_operators_stored:
	retn

calculate_parsed_expression:
; in:
;  esi - command sequence created by parse_expression
;  edi - top of the stack in the calculation workspace
; out:
;  cf set if expression had structural errors
;  esi = pointer moved past the processed sequence
;  edi - top of the updated stack
;  when cf = 0:
;   eax = non-zero if result is considered not yet known for resolving purposes
	mov	[expression_end],esi
	mov	eax,edi
	sub	eax,[calculation_workspace.memory_start]
	mov	[calculation_position],eax
	cmp	dword [esi],0
	je	invalid_expression
    calculation_loop:
	mov	esi,[expression_end]
	lodsd
	cmp	al,EXPR_OPERATOR
	je	execute_operator
	cmp	al,EXPR_NUMBER
	je	push_literal
	cmp	al,EXPR_STRING
	je	push_literal
	cmp	al,EXPR_FLOAT
	je	push_literal
	cmp	al,EXPR_SYMBOL_VALUE
	je	push_symbol_value
	cmp	al,EXPR_SYMBOL
	je	push_symbol_current_value
	cmp	al,EXPR_AREA_ADDRESS
	je	push_area_address
	test	eax,eax
	jnz	structural_error
       ; clc
	retn
    execute_operator:
	lodsd
	mov	[expression_end],esi
	jmp	eax
    push_literal:
	mov	ebx,eax
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	lodsd
	test	ebx,EXPRF_CALM_LITERAL
	jnz	calm_literal_value
	test	ebx,EXPRF_VALUE_IN_WORKSPACE
	jnz	valid_literal_value
	test	eax,eax
	jnz	valid_literal_value
	jmp	invalid_number_in_expression
    calm_literal_value:
	add	eax,[calm_literals]
    valid_literal_value:
	mov	[edi+ExpressionTerm.attributes],ebx
	mov	[edi+ExpressionTerm.value],eax
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	add	edi,2*sizeof.ExpressionTerm
	mov	[expression_end],esi
	jmp	calculation_loop
    push_symbol_current_value:
    ; occurring only in CALM expressions, entire expression workspace should be free to use
	lodsd
	test	eax,eax
	jz	invalid_identifier_in_expression
	mov	ebx,eax
	mov	[expression_end],esi
	call	use_available_value
	jc	undefined_symbol_in_expression
	mov	al,[edx+ValueDefinition.type]
	cmp	al,VALTYPE_SYMBOLIC
	jne	identify_value_to_push
	push	esi edi
	call	clear_line_embeddings
	xor	esi,esi
	xor	ecx,ecx
	call	embed_symbolic_value
	mov	edi,[expression_workspace.memory_start]
	and	[leave_opening_parentheses],0
	call	parse_expression
	pop	edi
	call	get_constituent_value
	jnc	invalid_subexpression
	mov	esi,[expression_workspace.memory_start]
	push	[calculation_position]
	call	calculate_parsed_expression
	pop	[calculation_position]
	jc	invalid_subexpression
	pop	[expression_end]
	test	eax,eax
	jnz	unknown_result
	jmp	calculation_loop
    invalid_subexpression:
	pop	[expression_end]
	mov	edx,_invalid_expression
	call	register_error
	jmp	unknown_result
    push_symbol_value:
	lodsd
	test	eax,eax
	jz	invalid_identifier_in_expression
	mov	ebx,eax
	lodsd
	mov	edx,eax
	call	mark_symbol_as_used
	mov	[expression_end],esi
	test	edx,edx
	jz	undefined_symbol_in_expression
	mov	al,[edx+ValueDefinition.type]
    identify_value_to_push:
	cmp	al,VALTYPE_NUMERIC
	je	push_numeric_symbol
	cmp	al,VALTYPE_ELEMENT
	je	push_element
	cmp	al,VALTYPE_AREA
	je	push_element
	cmp	al,VALTYPE_STRING
	je	push_string
	cmp	al,VALTYPE_FLOAT
	je	push_float
	cmp	al,VALTYPE_PLAIN
	je	push_plain_symbol
	cmp	al,VALTYPE_NATIVE_FUNCTION
	jne	invalid_symbol_in_expression
	jmp	[edx+ValueDefinition.value]
    push_numeric_symbol:
	xor	esi,esi
	test	[edx+ValueDefinition.flags],VAL_SHIFTABLE
	jz	predicted_shift_ok
	mov	eax,[edx+ValueDefinition.pass]
	cmp	eax,[current_pass]
	je	predicted_shift_ok
	mov	esi,[predicted_shift]
      predicted_shift_ok:
	mov	ebx,[edx+ValueDefinition.value]
	mov	eax,ebx
	add	eax,[edx+ValueDefinition.value_length]
	push	eax edi
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[edi+ExpressionTerm.value],ebx
	test	esi,esi
	jz	push_numeric_terms
	or	[next_pass_needed],1
	mov	ecx,temporary_value
	mov	dword [ecx],4
	mov	dword [ecx+4],esi
	lea	esi,[edi+sizeof.ExpressionTerm]
	mov	[esi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[esi+ExpressionTerm.value],ecx
	push	ebx
	call	add_term_values
	pop	ebx
      push_numeric_terms:
	add	edi,sizeof.ExpressionTerm
	mov	edx,calculation_workspace
	mov	ecx,sizeof.ExpressionTerm
	call	reserve_workspace
	mov	eax,[ebx]
	lea	ebx,[ebx+4+eax]
	mov	eax,[ebx]
	test	eax,eax
	jz	numeric_terms_pushed
	mov	eax,[ebx]
	mov	[edi+ExpressionTerm.metadata],eax
	add	ebx,4
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[edi+ExpressionTerm.value],ebx
	jmp	push_numeric_terms
      numeric_terms_pushed:
	add	ebx,4
	pop	edx ecx
	cmp	ebx,ecx
	jne	symbol_metadata_ok
	xor	ebx,ebx
      symbol_metadata_ok:
	mov	[edx+ExpressionTerm.metadata],ebx
	xor	eax,eax
	mov	[edi+ExpressionTerm.attributes],eax
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    push_plain_symbol:
	mov	ebx,[edx+ValueDefinition.value]
    push_plain_value:
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	mov	esi,edi
	mov	edx,value_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	edi,[value_position]
	mov	ecx,4+4
	call	reserve_workspace
	xor	eax,eax
	test	ebx,ebx
	jz	number_length_ok
	bsr	eax,ebx
	inc	al
	shr	al,3
	inc	al
      number_length_ok:
	stosd
	mov	[edi],ebx
	add	eax,edi
	sub	eax,[value_workspace.memory_start]
	xchg	eax,[value_position]
	mov	edi,esi
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	mov	[edi+ExpressionTerm.value],eax
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	add	edi,2*sizeof.ExpressionTerm
	jmp	calculation_loop
    push_string:
	mov	al,EXPR_STRING
    push_value:
	mov	[term_type],al
	mov	ebx,[edx+ValueDefinition.value]
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	mov	[edi+ExpressionTerm.value],ebx
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	mov	al,[term_type]
	mov	[edi+ExpressionTerm.attributes],eax
	add	edi,2*sizeof.ExpressionTerm
	jmp	calculation_loop
    push_float:
	mov	al,EXPR_FLOAT
	jmp	push_value
    push_element:
	mov	edx,calculation_workspace
	mov	ecx,3*sizeof.ExpressionTerm
	call	reserve_workspace
	mov	eax,EXPR_NUMBER
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],zero_value
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.value],singular_value
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.metadata],ebx
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+2*sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	add	edi,3*sizeof.ExpressionTerm
	jmp	calculation_loop
    push_area_address:
	lodsd
	mov	[expression_end],esi
	jmp	area_address_value
    base_address_value:
	mov	eax,[current_area]
    area_address_value:
	mov	ebx,[eax+ValueDefinition.value]
	add	ebx,sizeof.AreaHeader
    push_numeric_value:
	mov	edx,calculation_workspace
	mov	ecx,sizeof.ExpressionTerm
	call	reserve_workspace
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[edi+ExpressionTerm.value],ebx
	and	[edi+ExpressionTerm.metadata],0
	jmp	push_variable_terms
    truncated_address_value:
	xor	esi,esi
	jmp	push_current_address
    current_address_value:
	or	esi,-1
      push_current_address:
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	mov	eax,[current_area]
	mov	ebx,[eax+ValueDefinition.value]
	mov	ecx,[eax+ValueDefinition.value_length]
	sub	ecx,[ebx+AreaHeader.base_address_length]
	sub	ecx,sizeof.AreaHeader
	and	esi,[ebx+AreaHeader.uninitialized_length]
	add	ecx,esi
	add	ebx,sizeof.AreaHeader
	mov	esi,temporary_value
	mov	dword [esi],4
	mov	[esi+4],ecx
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[edi+ExpressionTerm.value],esi
	and	[edi+ExpressionTerm.metadata],0
	lea	esi,[edi+sizeof.ExpressionTerm]
	mov	[esi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[esi+ExpressionTerm.value],ebx
	call	add_term_values
	mov	ebx,[esi+ExpressionTerm.value]
      push_variable_terms:
	mov	eax,[ebx]
	lea	ebx,[ebx+4+eax]
      convert_variable_term:
	add	edi,sizeof.ExpressionTerm
	mov	edx,calculation_workspace
	mov	ecx,sizeof.ExpressionTerm
	call	reserve_workspace
	mov	eax,[ebx]
	test	eax,eax
	jz	variable_terms_converted
	mov	[edi+ExpressionTerm.metadata],eax
	add	ebx,4
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[edi+ExpressionTerm.value],ebx
	mov	eax,[ebx]
	lea	ebx,[ebx+4+eax]
	jmp	convert_variable_term
      variable_terms_converted:
	mov	[edi+ExpressionTerm.attributes],eax
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    truncated_position_value:
	call	prepare_to_push_computed_value
	call	get_output_length
	jmp	push_output_position
      prepare_to_push_computed_value:
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	mov	esi,edi
	mov	edx,value_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	edi,[value_position]
	mov	ecx,4+8
	call	reserve_workspace
	retn
    current_position_value:
	call	prepare_to_push_computed_value
	call	get_output_position
      push_output_position:
	mov	edi,[value_workspace.memory_start]
	add	edi,[value_position]
      push_computed_value:
	mov	ecx,eax
	or	ecx,edx
	jz	computed_value_length_ok
	bsr	ecx,edx
	jnz	long_computed_value
	bsr	ecx,eax
	inc	cl
	shr	cl,3
	inc	cl
	jmp	computed_value_length_ok
      long_computed_value:
	inc	cl
	shr	cl,3
	add	cl,1+4
      computed_value_length_ok:
	mov	[edi],ecx
	add	edi,4
	stosd
	mov	eax,edx
	stosd
	add	ecx,edi
	sub	ecx,[value_workspace.memory_start]
	xchg	ecx,[value_position]
	mov	edi,esi
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	mov	[edi+ExpressionTerm.value],ecx
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	add	edi,2*sizeof.ExpressionTerm
	jmp	calculation_loop
    current_time_value:
	call	prepare_to_push_computed_value
	call	get_timestamp
	jmp	push_computed_value
    current_line_number_value:
	call	prepare_to_push_computed_value
	call	get_file_source_entry
	mov	eax,[ebx+SourceEntry.line_number]
	xor	edx,edx
	jmp	push_computed_value
    current_file_name_value:
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	push	edi
	call	get_file_source_entry
	jmp	push_file_name_value
    main_file_name_value:
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	push	edi
	mov	ebx,[source_context]
	lea	ebx,[ebx+sizeof.SourceContext]
      push_file_name_value:
	mov	esi,[ebx+SourceEntry.name]
	xor	ecx,ecx
	cmp	[ebx+SourceEntry.type],SOURCE_FILE
	jne	file_name_length_ok
	mov	ecx,[ebx+SourceEntry.name_length]
	test	ecx,ecx
	jnz	file_name_length_ok
	mov	edi,esi
	xor	al,al
	dec	ecx
	repne	scasb
	add	ecx,2
	neg	ecx
      file_name_length_ok:
	mov	ebx,ecx
	add	ecx,4
	mov	edx,value_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	edi,[value_position]
	call	reserve_workspace
	mov	eax,ebx
	stosd
	mov	ecx,ebx
	shr	ecx,2
	rep	movsd
	mov	ecx,ebx
	and	ecx,11b
	rep	movsb
	mov	eax,edi
	sub	eax,[value_workspace.memory_start]
	xchg	eax,[value_position]
	pop	edi
	mov	[edi+ExpressionTerm.attributes],EXPR_STRING + EXPRF_VALUE_IN_WORKSPACE
	mov	[edi+ExpressionTerm.value],eax
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	add	edi,2*sizeof.ExpressionTerm
	jmp	calculation_loop
    structural_error:
	cmp	eax,EXPR_MISSING_PARENTHESIS
	je	expression_missing_parenthesis
    invalid_expression:
	mov	edx,_invalid_expression
	call	register_error
	stc
	retn
    expression_missing_parenthesis:
	mov	edx,_missing_closing_parenthesis
	call	register_error
	stc
	retn
    undefined_symbol_in_expression:
	cmp	[next_pass_needed],0
	jne	unknown_result
	mov	edx,_undefined_symbol
	jmp	error_causing_unknown_result
    invalid_identifier_in_expression:
	mov	edx,_invalid_identifier
	jmp	error_causing_unknown_result
    invalid_number_in_expression:
	mov	edx,_invalid_number
	jmp	error_causing_unknown_result
    invalid_symbol_in_expression:
	mov	edx,_invalid_symbol_value
    error_causing_unknown_result:
	call	register_error
    unknown_result:
	mov	eax,edi
	mov	edi,[calculation_position]
	add	edi,[calculation_workspace.memory_start]
	cmp	edi,eax
	jb	unknown_result_terms_ready
	mov	edi,eax
	mov	edx,calculation_workspace
	mov	ecx,2*sizeof.ExpressionTerm
	call	reserve_workspace
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
      unknown_result_terms_ready:
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[edi+ExpressionTerm.value],zero_value
	add	edi,2*sizeof.ExpressionTerm
	mov	esi,[expression_end]
    skip_expression:
	lodsd
	cmp	al,EXPR_OPERATOR
	je	skip_dword
	cmp	al,EXPR_NUMBER
	je	skip_dword
	cmp	al,EXPR_STRING
	je	skip_dword
	cmp	al,EXPR_FLOAT
	je	skip_dword
	cmp	al,EXPR_SYMBOL_VALUE
	je	skip_two_dwords
	test	eax,eax
	jnz	invalid_expression
	inc	eax
       ; clc
	retn
    skip_two_dwords:
	add	esi,4
    skip_dword:
	add	esi,4
	jmp	skip_expression

pop_terms:
; in:
;  edi = top of the stack
; out:
;  edi = new top of the stack, at the same time a pointer to the first term of retrieved linear polynomial
;  cf set when there were no more entries on the stack
; preserves: eax, ebx, ecx, edx, esi
	cmp	edi,[calculation_workspace.memory_start]
	je	nothing_to_pop
	sub	edi,sizeof.ExpressionTerm
      find_first_term:
	cmp	edi,[calculation_workspace.memory_start]
	je	first_term_found
	cmp	[edi-sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	je	first_term_found
	sub	edi,sizeof.ExpressionTerm
	jmp	find_first_term
      first_term_found:
	clc
	retn
      nothing_to_pop:
	and	[edi+ExpressionTerm.metadata],0
	and	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	mov	[edi+ExpressionTerm.value],zero_value
	stc
	retn

calculate_to_number:
	call	pop_terms
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	single_term_result
    make_terms_numeric:
	mov	eax,[edi+ExpressionTerm.attributes]
	test	eax,eax
	jz	all_terms_numeric
	call	get_numeric_term_value
	add	edi,sizeof.ExpressionTerm
	jmp	make_terms_numeric
    all_terms_numeric:
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop

calculate_to_string:
	call	pop_terms
	mov	al,EXPR_STRING
	xchg	al,byte [edi+ExpressionTerm.attributes]
	cmp	al,EXPR_FLOAT
	jne	single_term_result
	mov	edx,_invalid_value
	call	register_error
	mov	[edi+ExpressionTerm.value],zero_value
	mov	[edi+ExpressionTerm.attributes],EXPR_STRING
	and	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	add	edi,2*sizeof.ExpressionTerm
	jmp	calculation_loop

calculate_to_float:
	call	pop_terms
	call	get_float_term_value
	jmp	single_term_result

extract_integer_part:
	call	pop_terms
	call	truncate_term_value
	jmp	single_term_result

calculate_neg:
	call	pop_terms
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	negate_float_term
	call	negate_term_value
	add	edi,sizeof.ExpressionTerm
    negate_element_terms:
	cmp	[edi+ExpressionTerm.attributes],0
	je	all_terms_negated
	cmp	[edi+ExpressionTerm.metadata],0
	je	negate_next_term
	call	negate_term_value
    negate_next_term:
	add	edi,sizeof.ExpressionTerm
	jmp	negate_element_terms
    all_terms_negated:
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    negate_float_term:
	call	prepare_altered_float_term
	xor	[ebx+FloatData.attributes],FLOAT_NEGATIVE
    single_term_result:
	add	edi,sizeof.ExpressionTerm
	mov	eax,edi
	call	check_for_excess_terms
	jnc	single_term_result_ok
	mov	edx,_misused_variable_term
	call	register_error
      single_term_result_ok:
	and	[edi+ExpressionTerm.attributes],0
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    check_for_excess_terms:
	cmp	[eax+ExpressionTerm.attributes],0
	je	no_excess_terms
	cmp	[eax+ExpressionTerm.metadata],0
	jne	excess_terms
	add	eax,sizeof.ExpressionTerm
	jne	check_for_excess_terms
      no_excess_terms:
	clc
	retn
      excess_terms:
	stc
	retn
    prepare_altered_float_term:
	mov	ebx,edi
	mov	edx,value_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	edi,[value_position]
	mov	ecx,sizeof.FloatData
	call	reserve_workspace
	xchg	edi,ebx
	call	get_term_value
	xchg	edi,ebx
	mov	esi,edx
	assert	sizeof.FloatData and 11b = 0
	mov	ecx,sizeof.FloatData shr 2
	rep	movsd
	mov	eax,edi
	sub	eax,[value_workspace.memory_start]
	xchg	eax,[value_position]
	sub	edi,sizeof.FloatData
	xchg	edi,ebx
	mov	[edi+ExpressionTerm.value],eax
	or	[edi+ExpressionTerm.attributes],EXPRF_VALUE_IN_WORKSPACE
	retn

calculate_add:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	cmp	byte [esi+ExpressionTerm.attributes],EXPR_FLOAT
	je	add_float_terms
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	add_float_terms
	cmp	[edi+ExpressionTerm.metadata],0
	jne	add_constant_terms
	mov	eax,[esi+ExpressionTerm.metadata]
	mov	[edi+ExpressionTerm.metadata],eax
    add_constant_terms:
	call	add_term_values
	add	esi,sizeof.ExpressionTerm
    add_variable_terms:
	cmp	[esi+ExpressionTerm.attributes],0
	je	all_terms_added
	mov	eax,[esi+ExpressionTerm.metadata]
	test	eax,eax
	jz	add_next_term
	push	edi
    find_element_to_add_to:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	attach_new_element
	cmp	eax,[edi+ExpressionTerm.metadata]
	jne	find_element_to_add_to
	push	esi
	call	add_term_values
	jnz	variable_term_added
	and	[edi+ExpressionTerm.metadata],0
    variable_term_added:
	pop	esi edi
    add_next_term:
	add	esi,sizeof.ExpressionTerm
	jmp	add_variable_terms
    attach_new_element:
	assert	sizeof.ExpressionTerm and 11b = 0
	mov	ecx,sizeof.ExpressionTerm shr 2
	rep	movsd
	and	[edi+ExpressionTerm.attributes],0
	pop	edi
	jmp	add_variable_terms
    all_terms_added:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	jne	all_terms_added
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    add_float_terms:
	mov	eax,add_floats
    operation_on_float_terms:
	push	esi edi eax
	mov	ebx,edi
	mov	edx,value_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	edi,[value_position]
	mov	ecx,sizeof.FloatData
	call	reserve_workspace
	lea	ecx,[edi+sizeof.FloatData]
	sub	ecx,[edx+Workspace.memory_start]
	mov	[value_position],ecx
	xchg	edi,ebx
	call	get_float_term_value
	xchg	edi,esi
	call	get_float_term_value
	mov	edi,esi
	mov	esi,edx
	call	get_term_value
	mov	edi,ebx
	mov	ebx,edx
	pop	eax
	call	eax
	mov	ebx,edi
	sub	ebx,[value_workspace.memory_start]
	test	[edi+FloatData.attributes],FLOAT_INFINITE or FLOAT_INDETERMINATE or FLOAT_UNDERFLOW
	jz	operation_on_float_terms_ok
	mov	edx,_indeterminate_result
	call	register_error
	xor	eax,eax
	mov	[edi+FloatData.attributes],eax
	add	edi,FloatData.mantissa
	mov	ecx,MANTISSA_SEGMENTS
	rep	stosd
      operation_on_float_terms_ok:
	pop	edi esi
	mov	[edi+ExpressionTerm.value],ebx
	mov	[edi+ExpressionTerm.attributes],EXPR_FLOAT + EXPRF_VALUE_IN_WORKSPACE
	and	[edi+ExpressionTerm.metadata],0
    finish_calculation_on_single_terms:
	add	edi,sizeof.ExpressionTerm
	mov	eax,edi
	call	check_for_excess_terms
	jc	single_terms_violation
	lea	eax,[esi+sizeof.ExpressionTerm]
	call	check_for_excess_terms
	jnc	single_terms_ok
      single_terms_violation:
	mov	edx,_misused_variable_term
	call	register_error
      single_terms_ok:
	and	[edi+ExpressionTerm.attributes],0
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop

calculate_sub:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	cmp	byte [esi+ExpressionTerm.attributes],EXPR_FLOAT
	je	subtract_float_terms
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	subtract_float_terms
	cmp	[edi+ExpressionTerm.metadata],0
	jne	subtract_constant_terms
	mov	eax,[esi+ExpressionTerm.metadata]
	mov	[edi+ExpressionTerm.metadata],eax
    subtract_constant_terms:
	call	subtract_term_values
	add	esi,sizeof.ExpressionTerm
    subtract_variable_terms:
	cmp	[esi+ExpressionTerm.attributes],0
	je	all_terms_subtracted
	mov	eax,[esi+ExpressionTerm.metadata]
	test	eax,eax
	jz	subtract_next_term
	push	edi
    find_element_to_subtract_from:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	attach_negated_element
	cmp	eax,[edi+ExpressionTerm.metadata]
	jne	find_element_to_subtract_from
	push	esi
	call	subtract_term_values
	jnz	variable_term_subtracted
	and	[edi+ExpressionTerm.metadata],0
    variable_term_subtracted:
	pop	esi edi
    subtract_next_term:
	add	esi,sizeof.ExpressionTerm
	jmp	subtract_variable_terms
    attach_negated_element:
	assert	sizeof.ExpressionTerm and 11b = 0
	mov	ecx,sizeof.ExpressionTerm shr 2
	rep	movsd
	sub	edi,sizeof.ExpressionTerm
	call	negate_term_value
	add	edi,sizeof.ExpressionTerm
	and	[edi+ExpressionTerm.attributes],0
	pop	edi
	jmp	subtract_variable_terms
    all_terms_subtracted:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	jne	all_terms_subtracted
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    subtract_float_terms:
	mov	eax,subtract_floats
	jmp	operation_on_float_terms

calculate_mul:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	cmp	byte [esi+ExpressionTerm.attributes],EXPR_FLOAT
	je	multiply_float_terms
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	multiply_float_terms
	cmp	[esi+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	jne	multiply_by_destination
    multiply_terms:
	call	multiply_term_values
	jnz	multiply_next_term
	and	[edi+ExpressionTerm.metadata],0
    multiply_next_term:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	terms_multiplied
	jmp	multiply_terms
    terms_multiplied:
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    multiply_by_destination:
	and	[esi+ExpressionTerm.metadata],0
	cmp	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	je	duplicate_destination_constant_term
	mov	edx,_nonlinear_polynomial
	call	register_error
    duplicate_destination_constant_term:
	mov	eax,[edi+ExpressionTerm.attributes]
	mov	edx,[edi+ExpressionTerm.value]
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.value],edx
    multiply_source_terms:
	mov	eax,[esi+ExpressionTerm.metadata]
	mov	[edi+ExpressionTerm.metadata],eax
	call	multiply_term_values
	jnz	multiply_next_source_term
	and	[edi+ExpressionTerm.metadata],0
    multiply_next_source_term:
	add	esi,sizeof.ExpressionTerm
	add	edi,sizeof.ExpressionTerm
	cmp	[esi+ExpressionTerm.attributes],0
	je	source_terms_multiplied
	cmp	[esi+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	je	multiply_source_terms
	jmp	duplicate_destination_constant_term
    source_terms_multiplied:
	and	[edi+ExpressionTerm.attributes],0
	jmp	terms_multiplied
    multiply_float_terms:
	mov	eax,multiply_floats
	jmp	operation_on_float_terms

calculate_div:
	call	pop_terms
	mov	eax,edi
	call	check_for_excess_terms
	jnc	divisor_ok
	mov	edx,_misused_variable_term
	call	register_error
    divisor_ok:
	mov	esi,edi
	call	pop_terms
	cmp	byte [esi+ExpressionTerm.attributes],EXPR_FLOAT
	je	divide_float_terms
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	divide_float_terms
	and	[edi+ExpressionTerm.metadata],0
	mov	eax,[esi+ExpressionTerm.attributes]
	mov	edx,[esi+ExpressionTerm.value]
	mov	[esi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	mov	[esi+sizeof.ExpressionTerm+ExpressionTerm.value],edx
	call	divide_term_values
    divide_variable_term:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	division_done
	cmp	[edi+ExpressionTerm.metadata],0
	je	divide_variable_term
	mov	eax,[esi+sizeof.ExpressionTerm+ExpressionTerm.attributes]
	mov	edx,[esi+sizeof.ExpressionTerm+ExpressionTerm.value]
	mov	[esi+ExpressionTerm.attributes],eax
	mov	[esi+ExpressionTerm.value],edx
	call	divide_term_values
	jz	divide_variable_term
	mov	edx,_subdivided_variable_term
	call	register_error
	jmp	divide_variable_term
    division_done:
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    divide_float_terms:
	mov	eax,divide_floats
	jmp	operation_on_float_terms

calculate_mod:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	call	divide_term_values
	and	[esi+ExpressionTerm.metadata],0
	assert	sizeof.ExpressionTerm and 11b = 0
	mov	ecx,sizeof.ExpressionTerm shr 2
	rep	movsd
	mov	eax,edi
	call	check_for_excess_terms
	jc	mod_variable_term
	mov	eax,esi
	call	check_for_excess_terms
	jnc	mod_terms_ok
      mod_variable_term:
	mov	edx,_misused_variable_term
	call	register_error
      mod_terms_ok:
	and	[edi+ExpressionTerm.attributes],0
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop

calculate_not:
	call	pop_terms
	call	invert_term_value_bits
	jmp	single_term_result

calculate_xor:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	call	bitwise_add_term_values
    finish_bitwise_calculation:
	cmp	[edi+ExpressionTerm.metadata],0
	jne	finish_calculation_on_single_terms
	mov	eax,[esi+ExpressionTerm.metadata]
	mov	[edi+ExpressionTerm.metadata],eax
	jmp	finish_calculation_on_single_terms

calculate_and:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	call	bitwise_multiply_term_values
	jmp	finish_bitwise_calculation

calculate_or:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	call	bitwise_inclusive_or_of_term_values
	jmp	finish_bitwise_calculation

calculate_shl:
	call	pop_terms
	call	get_numeric_term_value
	mov	ecx,5
	call	fit_value
	js	negative_shift_left
	jc	huge_shift_left
    shift_left:
	mov	esi,edi
    check_shift_left_argument:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	shift_left_argument_ok
	cmp	[edi+ExpressionTerm.metadata],0
	je	check_shift_left_argument
	mov	edx,_misused_variable_term
	call	register_error
    shift_left_argument_ok:
	mov	edi,esi
	call	pop_terms
	and	[edi+ExpressionTerm.metadata],0
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	shift_floating_point_left
    shift_term_left:
	mov	eax,[esi]
	mov	dl,[esi+4]
	call	shift_term_value_left
    shift_variable_term_left:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	shift_done
	cmp	[edi+ExpressionTerm.metadata],0
	je	shift_variable_term_left
	jmp	shift_term_left
    shift_done:
	add	edi,sizeof.ExpressionTerm
	jmp	calculation_loop
    negative_shift_left:
	jc	huge_shift_right
	not	byte [edi+4]
	neg	dword [edi]
	jc	shift_right
	inc	byte [edi+4]
	jnz	shift_right
    huge_shift_right:
	or	eax,-1
	mov	[edi],eax
	mov	[edi+4],al
	jmp	shift_right
    shift_floating_point_left:
	push	esi
	call	prepare_altered_float_term
	pop	esi
	xchg	esi,ebx
	call	get_float_unnormalization
	jc	single_term_result
	mov	eax,[esi+FloatData.exponent]
	cdq
	add	eax,[ebx]
	movzx	ebx,byte [ebx+4]
	adc	ebx,edx
	mov	[esi+FloatData.exponent],eax
	cdq
	cmp	ebx,edx
	jne	floating_point_over_boundary
	test	ecx,ecx
	jz	single_term_result
	push	edi
	mov	edi,esi
	and	[mantissa_tail],0
	call	normalize_float
	pop	edi
	jmp	single_term_result
    floating_point_over_boundary:
	jecxz	floating_point_out_of_range
	test	ebx,ebx
	jnz	floating_point_out_of_range
	mov	ecx,7FFFFFFFh
	sub	eax,ecx
	mov	[esi+FloatData.exponent],ecx
	push	eax edi
	mov	edi,esi
	and	[mantissa_tail],0
	call	normalize_float
	pop	edi ecx
	mov	eax,[esi+FloatData.exponent]
	cdq
	add	eax,ecx
	js	floating_point_out_of_range
	adc	edx,0
	jnz	floating_point_out_of_range
	mov	[esi+FloatData.exponent],eax
	jmp	single_term_result
    floating_point_out_of_range:
	mov	edx,_value_out_of_range
	call	register_error
	jmp	single_term_result

calculate_shr:
	call	pop_terms
	call	get_numeric_term_value
	mov	ecx,5
	call	fit_value
	js	negative_shift_right
	jc	huge_shift_right
    shift_right:
	mov	esi,edi
    check_shift_right_argument:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	shift_right_argument_ok
	cmp	[edi+ExpressionTerm.metadata],0
	je	check_shift_right_argument
	mov	edx,_misused_variable_term
	call	register_error
    shift_right_argument_ok:
	mov	edi,esi
	call	pop_terms
	and	[edi+ExpressionTerm.metadata],0
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	je	shift_floating_point_right
    shift_term_right:
	mov	eax,[esi]
	mov	dl,[esi+4]
	call	shift_term_value_right
    shift_variable_term_right:
	add	edi,sizeof.ExpressionTerm
	cmp	[edi+ExpressionTerm.attributes],0
	je	shift_done
	cmp	[edi+ExpressionTerm.metadata],0
	je	shift_variable_term_right
	call	get_term_value
	mov	ecx,[esi]
	mov	bl,cl
	mov	al,[esi+4]
	shrd	ecx,eax,3
	shr	al,3
	jnz	not_exact_shift
	cmp	ecx,[edx]
	jae	not_exact_shift
	xchg	edi,edx
	add	edi,4
	xor	al,al
	repe	scasb
	xchg	edi,edx
	jne	not_exact_shift
	mov	cl,bl
	and	cl,8-1
	jz	shift_term_right
	mov	al,1
	shl	al,cl
	dec	al
	test	[edx],al
	jz	shift_term_right
    not_exact_shift:
	mov	edx,_subdivided_variable_term
	call	register_error
	jmp	shift_variable_term_right
    negative_shift_right:
	jc	huge_shift_left
	not	byte [edi+4]
	neg	dword [edi]
	jc	shift_left
	inc	byte [edi+4]
	jnz	shift_left
    huge_shift_left:
	or	eax,-1
	mov	[edi],eax
	mov	[edi+4],al
	jmp	shift_left
    shift_floating_point_right:
	push	esi
	call	prepare_altered_float_term
	pop	esi
	xchg	esi,ebx
	call	get_float_unnormalization
	jc	single_term_result
	mov	eax,[esi+FloatData.exponent]
	cdq
	sub	eax,[ebx]
	movzx	ecx,byte [ebx+4]
	sbb	edx,ecx
	mov	ecx,edx
	mov	[esi+FloatData.exponent],eax
	cdq
	cmp	ecx,edx
	je	single_term_result
	cmp	ecx,-1
	jne	floating_point_out_of_range
	mov	[esi+FloatData.exponent],-80000000h
	mov	ecx,eax
	neg	ecx
	push	edi
	mov	edi,esi
	and	[mantissa_tail],0
	call	unnormalize_float
	pop	edi
	mov	[esi+FloatData.exponent],-80000000h
	call	get_float_unnormalization
	jc	floating_point_out_of_range
	jmp	single_term_result

calculate_bsf:
	call	pop_terms
	mov	esi,edi
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,10
	mov	edx,value_workspace
	call	reserve_workspace
	call	find_first_set_bit_in_term_value
	jz	bit_scan_result_undefined
	movzx	edx,dl
    bit_scan_result_ready:
	mov	ebx,edi
	and	dword [edi],0
	add	edi,4
	stosd
	mov	ax,dx
	stosw
	mov	ecx,6
      optimize_result_value:
	cmp	[edi-1],dh
	jne	result_value_ready
	movsx	eax,byte [edi-2]
	cmp	ah,dh
	jne	result_value_ready
	dec	edi
	loop	optimize_result_value
      result_value_ready:
	mov	[ebx],ecx
	sub	edi,[value_workspace.memory_start]
	mov	[value_position],edi
	sub	ebx,[value_workspace.memory_start]
	mov	edi,esi
	mov	[edi+ExpressionTerm.value],ebx
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	and	[edi+ExpressionTerm.metadata],0
	jmp	single_term_result

calculate_bsr:
	call	pop_terms
	mov	esi,edi
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,10
	mov	edx,value_workspace
	call	reserve_workspace
	cmp	byte [esi+ExpressionTerm.attributes],EXPR_FLOAT
	je	extract_float_exponent
	call	find_last_set_bit_in_term_value
	jc	bit_scan_result_undefined
	movzx	edx,dl
	jmp	bit_scan_result_ready
    bit_scan_result_undefined:
	mov	edi,esi
	mov	edx,_indeterminate_result
	call	register_error
	mov	[edi+ExpressionTerm.value],zero_value
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	and	[edi+ExpressionTerm.metadata],0
	jmp	single_term_result
    extract_float_exponent:
	mov	edi,esi
	mov	[destination_term],edi
	call	get_term_value
	mov	esi,edx
	call	get_float_unnormalization
	jc	exponent_indeterminate
	mov	eax,[esi+FloatData.exponent]
	cdq
	sub	eax,ecx
	sbb	edx,0
      exponent_extracted:
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	esi,[destination_term]
	jmp	bit_scan_result_ready
      exponent_indeterminate:
	mov	edx,_indeterminate_result
	call	register_error
	xor	eax,eax
	xor	edx,edx
	jmp	exponent_extracted

calculate_bswap:
	call	pop_terms
	call	get_numeric_term_value
	mov	ecx,4
	call	fit_value
	js	bswap_argument_out_of_range
	jc	bswap_argument_out_of_range
	mov	esi,edi
	mov	eax,edi
	call	check_for_excess_terms
	jnc	bswap_argument_ok
	mov	edx,_misused_variable_term
	call	register_error
      bswap_argument_ok:
	call	pop_terms
	and	[edi+ExpressionTerm.metadata],0
	mov	ebx,[esi]
	call	reverse_term_value_bytes
	jmp	single_term_result
      bswap_argument_out_of_range:
	mov	edx,_value_out_of_range
	call	register_error
	call	pop_terms
	jmp	single_term_result

count_bytes:
	call	pop_terms
	call	get_string_term_value
	mov	esi,edi
	add	esi,sizeof.ExpressionTerm
	mov	eax,esi
	call	check_for_excess_terms
	mov	ebx,[edx]
	jnc	push_plain_value
	mov	edx,_misused_variable_term
	call	register_error
	jmp	push_plain_value

count_elements:
	call	pop_terms
	mov	edx,[edi+ExpressionTerm.metadata]
	xor	ebx,ebx
	mov	edx,edi
      count_variable_terms:
	add	edx,sizeof.ExpressionTerm
	cmp	[edx+ExpressionTerm.attributes],0
	je	push_plain_value
	cmp	[edx+ExpressionTerm.metadata],0
	je	count_variable_terms
	inc	ebx
	jmp	count_variable_terms

extract_element_reverse:
	xor	eax,eax
extract_element:
	call	get_indexed_term
	jc	replace_with_zero
	jecxz	replace_with_constant_unit
	mov	ebx,[ebx+ExpressionTerm.metadata]
	jmp	push_element
    get_indexed_term:
	call	pop_terms
	mov	esi,edi
	call	pop_terms
	mov	ebx,edi
	test	eax,eax
	jnz	process_term_indexing
	xchg	ebx,esi
      process_term_indexing:
	mov	edx,esi
      check_term_index:
	add	edx,sizeof.ExpressionTerm
	cmp	[edx+ExpressionTerm.attributes],0
	je	term_index_ok
	cmp	[edx+ExpressionTerm.metadata],0
	je	check_term_index
	mov	edx,_misused_variable_term
	call	register_error
      term_index_ok:
	push	ebx
	xchg	edi,esi
	call	get_numeric_term_value
	mov	ecx,4
	call	fit_value
	xchg	esi,edi
	pop	ebx
	js	no_such_term
	jc	no_such_term
	xor	ecx,ecx
      find_indexed_term:
	cmp	ecx,[esi]
	je	term_found
      find_next_term:
	add	ebx,sizeof.ExpressionTerm
	cmp	[ebx+ExpressionTerm.attributes],0
	je	no_such_term
	cmp	[ebx+ExpressionTerm.metadata],0
	je	find_next_term
	inc	ecx
	jmp	find_indexed_term
      term_found:
	clc
	retn
      no_such_term:
	stc
	retn
    replace_with_area_metadata:
	mov	edx,area_metadata
	mov	eax,EXPR_STRING
	jmp	replace_with_simple_constant
    replace_with_constant_unit:
	mov	edx,singular_value
	jmp	replace_with_simple_number
    replace_with_zero:
	mov	edx,zero_value
    replace_with_simple_number:
	mov	eax,EXPR_NUMBER
    replace_with_simple_constant:
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],edx
	xor	eax,eax
	mov	[edi+ExpressionTerm.metadata],eax
	mov	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],eax
	add	edi,2*sizeof.ExpressionTerm
	jmp	calculation_loop

extract_scale_reverse:
	xor	eax,eax
extract_scale:
	call	get_indexed_term
	jc	replace_with_zero
	mov	eax,[ebx+ExpressionTerm.attributes]
	mov	edx,[ebx+ExpressionTerm.value]
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],edx
	and	[edi+sizeof.ExpressionTerm+ExpressionTerm.attributes],0
	add	edi,2*sizeof.ExpressionTerm
	jmp	calculation_loop

extract_metadata_reverse:
	xor	eax,eax
extract_metadata:
	call	get_indexed_term
	jc	replace_with_zero
	mov	ebx,[ebx+ExpressionTerm.metadata]
	test	ebx,ebx
	jz	replace_with_zero
	test	ecx,ecx
	jz	push_numeric_value
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	test	edx,edx
	jz	replace_with_zero
	cmp	[edx+ValueDefinition.type],VALTYPE_AREA
	je	replace_with_area_metadata
	cmp	[edx+ValueDefinition.type],VALTYPE_ELEMENT
	jne	replace_with_zero
	mov	ecx,[edx+ValueDefinition.value_length]
	jecxz	replace_with_zero
	jmp	push_numeric_symbol

extract_size:
	call	pop_terms
	mov	ebx,[edi+ExpressionTerm.metadata]
	test	ebx,ebx
	jz	replace_with_zero
	jmp	push_numeric_value

get_term_value:
; in: edi - ExpressionTerm
; out: edx - 32-bit length followed by data
; preserves: eax, ebx, ecx, esi, edi
	mov	edx,[edi+ExpressionTerm.value]
	test	[edi+ExpressionTerm.attributes],EXPRF_VALUE_IN_WORKSPACE
	jz	value_pointer_ready
	add	edx,[value_workspace.memory_start]
    value_pointer_ready:
	retn

get_numeric_term_value:
; in: edi - ExpressionTerm
; out: edx - 32-bit length followed by numeric data
; preserves: ebx, esi, edi
; note:
;  term is converted to numeric and the value workspace may be modified in process
;  other pointers to term values may become invalidated
	mov	eax,[edi+ExpressionTerm.attributes]
	mov	edx,[edi+ExpressionTerm.value]
	test	eax,EXPRF_VALUE_IN_WORKSPACE
	jz	convert_to_numeric
	add	edx,[value_workspace.memory_start]
      convert_to_numeric:
	cmp	al,EXPR_FLOAT
	je	wrong_numeric_type
	cmp	al,EXPR_NUMBER
	je	value_pointer_ready
	mov	eax,[edx]
	test	byte [edx+4+eax-1],80h
	jnz	extend_to_unsigned_value
	mov	byte [edi+ExpressionTerm.attributes],EXPR_NUMBER
	retn
      wrong_numeric_type:
	mov	edx,_invalid_value
	call	register_error
	mov	edx,zero_value
	mov	[edi+ExpressionTerm.value],edx
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	retn
      extend_to_unsigned_value:
	push	esi edi
	mov	esi,edi
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,[edx]
	add	ecx,5
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointer_for_unsigned_value_ready
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointer_for_unsigned_value_ready:
	mov	eax,EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	xchg	eax,[esi+ExpressionTerm.attributes]
	mov	edx,[value_position]
	xchg	edx,[esi+ExpressionTerm.value]
	mov	esi,edx
	test	eax,EXPRF_VALUE_IN_WORKSPACE
	jz	make_unsigned_value
	add	esi,[value_workspace.memory_start]
      make_unsigned_value:
	mov	edx,edi
	lodsd
	stosd
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	xor	al,al
	stosb
	sub	edi,[value_workspace.memory_start]
	mov	[value_position],edi
	inc	dword [edx]
	pop	edi esi
	retn

get_string_term_value:
; in: edi - ExpressionTerm
; out: edx - 32-bit length followed by string data
; preserves: ebx, esi, edi
	mov	eax,[edi+ExpressionTerm.attributes]
	cmp	al,EXPR_STRING
	je	string_value_available
	mov	edx,_invalid_value
	call	register_error
	mov	[edi+ExpressionTerm.attributes],EXPR_STRING
	cmp	al,EXPR_NUMBER
	je	string_value_available
	mov	edx,zero_value
	mov	[edi+ExpressionTerm.value],edx
	retn
      string_value_available:
	mov	edx,[edi+ExpressionTerm.value]
	test	eax,EXPRF_VALUE_IN_WORKSPACE
	jz	string_value_pointer_ready
	add	edx,[value_workspace.memory_start]
      string_value_pointer_ready:
	retn

get_float_term_value:
; in: edi - ExpressionTerm
; out: edx - FloatData
; preserves: ebx, esi, edi
; note:
;  term is converted to float and the value workspace may be modified in process
;  other pointers to term values may become invalidated
	mov	eax,[edi+ExpressionTerm.attributes]
	mov	edx,[edi+ExpressionTerm.value]
	test	eax,EXPRF_VALUE_IN_WORKSPACE
	jz	convert_to_float
	add	edx,[value_workspace.memory_start]
      convert_to_float:
	cmp	al,EXPR_FLOAT
	je	value_pointer_ready
	cmp	al,EXPR_STRING
	je	convert_positive_to_float
	mov	eax,[edx]
	test	byte [edx+4+eax-1],80h
	jnz	convert_negative_to_float
      convert_positive_to_float:
	push	ebx esi
	mov	ebx,edi
	mov	edx,value_workspace
	mov	edi,[edx+Workspace.memory_start]
	add	edi,[value_position]
	mov	ecx,sizeof.FloatData
	call	reserve_workspace
	xchg	edi,ebx
	call	get_term_value
	mov	esi,edx
	lea	eax,[ebx+sizeof.FloatData]
	sub	eax,[value_workspace.memory_start]
	xchg	eax,[value_position]
	mov	[edi+ExpressionTerm.value],eax
	mov	[edi+ExpressionTerm.attributes],EXPR_FLOAT + EXPRF_VALUE_IN_WORKSPACE
	push	edi
	mov	edi,ebx
	call	number_to_float
	mov	edx,edi
	pop	edi esi ebx
	retn
      convert_negative_to_float:
	push	ebx
	call	negate_term_value
	pop	ebx
	call	convert_positive_to_float
	or	[edx+FloatData.attributes],FLOAT_NEGATIVE
	retn

negate_term_value:
; in:
;  edi - ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	push	esi
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,[esi]
	add	ecx,5
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointers_ready_for_negation
	mov	edi,[destination_term]
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointers_ready_for_negation:
	mov	edx,[esi]
	and	edx,not 11b
	mov	eax,1
	xor	ecx,ecx
	add	esi,4
	add	edi,4
      negate_dwords:
	cmp	ecx,edx
	je	dwords_negated
	mov	ebx,[esi+ecx]
	not	ebx
	add	eax,ebx
	mov	[edi+ecx],eax
	setc	al
	movzx	eax,al
	add	ecx,4
	jmp	negate_dwords
      dwords_negated:
	mov	edx,[esi-4]
      negate_bytes:
	cmp	ecx,edx
	je	bytes_negated
	mov	bl,[esi+ecx]
	not	bl
	add	al,bl
	mov	[edi+ecx],al
	setc	al
	inc	ecx
	jmp	negate_bytes
      bytes_negated:
	movsx	ebx,byte [esi+ecx-1]
	not	bh
	add	al,bh
	mov	[edi+ecx],al
	pop	esi
    value_calculated:
	jecxz	check_for_zero
      optimize_value:
	movsx	edx,byte [edi+ecx-1]
	cmp	dh,[edi+ecx]
	jne	value_optimized
	loop	optimize_value
      check_for_zero:
	cmp	byte [edi],0
	je	finish_value
      value_optimized:
	inc	ecx
      finish_value:
	sub	edi,4
	mov	[edi],ecx
    value_finished:
	mov	edx,edi
	sub	edx,[value_workspace.memory_start]
	lea	eax,[edx+4+ecx]
	mov	[value_position],eax
	mov	edi,[destination_term]
	mov	[edi+ExpressionTerm.value],edx
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	test	ecx,ecx
	retn

add_term_values:
; in:
;  esi - source ExpressionTerm
;  edi - destination ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	call	prepare_pointers_for_summation
	mov	edx,[ebx]
	cmp	edx,[esi]
	jb	shorter_addend_selected
	mov	edx,[esi]
	xchg	ebx,esi
      shorter_addend_selected:
	and	edx,not 11b
	add	ebx,4
	add	esi,4
	add	edi,4
	xor	al,al
	xor	ecx,ecx
      add_dwords:
	cmp	ecx,edx
	je	dwords_added
	neg	al
	mov	eax,[esi+ecx]
	adc	eax,[ebx+ecx]
	mov	[edi+ecx],eax
	setc	al
	add	ecx,4
	jmp	add_dwords
      dwords_added:
	mov	edx,[ebx-4]
      add_bytes:
	cmp	ecx,edx
	je	bytes_added
	neg	al
	mov	al,[esi+ecx]
	adc	al,[ebx+ecx]
	mov	[edi+ecx],al
	setc	al
	inc	ecx
	jmp	add_bytes
      bytes_added:
	movsx	ebx,byte [ebx+ecx-1]
	sar	ebx,8
	mov	edx,[esi-4]
	sub	edx,ecx
	and	edx,not 11b
	add	edx,ecx
      carry_dwords:
	cmp	ecx,edx
	je	dwords_carried
	neg	al
	mov	eax,[esi+ecx]
	adc	eax,ebx
	mov	[edi+ecx],eax
	setc	al
	add	ecx,4
	jmp	carry_dwords
      dwords_carried:
	mov	edx,[esi-4]
      carry_bytes:
	cmp	ecx,edx
	je	bytes_carried
	neg	al
	mov	al,[esi+ecx]
	adc	al,bl
	mov	[edi+ecx],al
	setc	al
	inc	ecx
	jmp	carry_bytes
      bytes_carried:
	movsx	edx,byte [esi+ecx-1]
	neg	al
	mov	al,dh
	adc	al,bl
	mov	[edi+ecx],al
	mov	esi,[source_term]
	jmp	value_calculated
    prepare_pointers_for_summation:
	mov	[source_term],esi
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	ebx,[edx]
	mov	edi,esi
	call	get_numeric_term_value
	mov	ecx,[edx]
	cmp	ecx,ebx
	jae	sum_length_estimated
	mov	ecx,ebx
      sum_length_estimated:
	add	ecx,5
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	edx,value_workspace
	call	reserve_workspace
	mov	edi,[destination_term]
	call	get_term_value
	mov	ebx,edx
	mov	edi,esi
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	retn

subtract_term_values:
; in:
;  esi - source ExpressionTerm
;  edi - destination ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	call	prepare_pointers_for_summation
	mov	edx,[esi]
	cmp	edx,[ebx]
	jb	shorter_length_selected
	mov	edx,[ebx]
      shorter_length_selected:
	push	edx
	and	edx,not 11b
	add	ebx,4
	add	esi,4
	add	edi,4
	xor	al,al
	xor	ecx,ecx
      subtract_dwords:
	cmp	ecx,edx
	je	dwords_subtracted
	neg	al
	mov	eax,[ebx+ecx]
	sbb	eax,[esi+ecx]
	mov	[edi+ecx],eax
	setc	al
	add	ecx,4
	jmp	subtract_dwords
      dwords_subtracted:
	pop	edx
      subtract_bytes:
	cmp	ecx,edx
	je	bytes_subtracted
	neg	al
	mov	al,[ebx+ecx]
	sbb	al,[esi+ecx]
	mov	[edi+ecx],al
	setc	al
	inc	ecx
	jmp	subtract_bytes
      bytes_subtracted:
	cmp	ecx,[ebx-4]
	je	minuend_ended
	xchg	esi,ebx
	movsx	ebx,byte [ebx+ecx-1]
	sar	ebx,8
	mov	edx,[esi-4]
	sub	edx,ecx
	and	edx,not 11b
	add	edx,ecx
      borrow_dwords:
	cmp	ecx,edx
	je	dwords_borrowed
	neg	al
	mov	eax,[esi+ecx]
	sbb	eax,ebx
	mov	[edi+ecx],eax
	setc	al
	add	ecx,4
	jmp	borrow_dwords
      dwords_borrowed:
	mov	edx,[esi-4]
      borrow_bytes:
	cmp	ecx,edx
	je	bytes_borrowed
	neg	al
	mov	al,[esi+ecx]
	sbb	al,bl
	mov	[edi+ecx],al
	setc	al
	inc	ecx
	jmp	borrow_bytes
      bytes_borrowed:
	movsx	edx,byte [esi+ecx-1]
	neg	al
	mov	al,dh
	sbb	al,bl
	mov	[edi+ecx],al
	mov	esi,[source_term]
	jmp	value_calculated
      minuend_ended:
	movsx	ebx,byte [ebx+ecx-1]
	sar	ebx,8
	mov	edx,[esi-4]
	sub	edx,ecx
	and	edx,not 11b
	add	edx,ecx
      subtract_supernumerary_dwords:
	cmp	ecx,edx
	je	supernumerary_dwords_subtracted
	neg	al
	mov	eax,ebx
	sbb	eax,[esi+ecx]
	mov	[edi+ecx],eax
	setc	al
	add	ecx,4
	jmp	subtract_supernumerary_dwords
      supernumerary_dwords_subtracted:
	mov	edx,[esi-4]
      subtract_supernumerary_bytes:
	cmp	ecx,edx
	je	supernumerary_bytes_subtracted
	neg	al
	mov	al,bl
	sbb	al,[esi+ecx]
	mov	[edi+ecx],al
	setc	al
	inc	ecx
	jmp	subtract_supernumerary_bytes
      supernumerary_bytes_subtracted:
	movsx	edx,byte [esi+ecx-1]
	neg	al
	mov	al,bl
	sbb	al,dh
	mov	[edi+ecx],al
	mov	esi,[source_term]
	jmp	value_calculated

multiply_term_values:
; in:
;  esi - source ExpressionTerm
;  edi - destination ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	mov	[source_term],esi
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	edi,multiplier
	mov	ecx,4
	call	fit_value
	jnc	multiply_by_dword
	mov	edi,esi
	call	get_numeric_term_value
	mov	esi,[destination_term]
	mov	edi,multiplier
	mov	ecx,4
	call	fit_value
	jnc	multiply_by_dword
	mov	edi,[source_term]
	call	get_term_value
	mov	ebx,[edx]
	mov	edi,[destination_term]
	call	get_term_value
	mov	eax,[edx]
	mov	ecx,ebx
	add	ecx,eax
	cmp	ebx,eax
	jae	split_length_selected
	xchg	ebx,eax
      split_length_selected:
	shr	ebx,1
	add	ecx,18
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	edx,value_workspace
	call	reserve_workspace
	mov	eax,edi
	mov	edi,[source_term]
	push	edi
	call	get_term_value
	mov	edi,eax
	mov	esi,edx
	call	split_value
	mov	eax,edi
	mov	edi,[destination_term]
	push	edi
	call	get_term_value
	mov	edi,eax
	mov	esi,edx
	call	split_value
	sub	edi,[value_workspace.memory_start]
	mov	edx,edi
	mov	edi,[free_temporary_terms]
	add	[free_temporary_terms],6*sizeof.ExpressionTerm
	mov	eax,EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	xchg	edx,[value_position]
	push	edx
	mov	esi,[value_workspace.memory_start]
	mov	ecx,4
      create_temporary_terms:
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],edx
	add	edx,[esi+edx]
	add	edx,4
	add	edi,sizeof.ExpressionTerm
	loop	create_temporary_terms
	mov	ecx,2
      duplicate_temporary_terms:
	mov	[edi+ExpressionTerm.attributes],eax
	mov	edx,[edi-4*sizeof.ExpressionTerm+ExpressionTerm.value]
	mov	[edi+ExpressionTerm.value],edx
	add	edi,sizeof.ExpressionTerm
	loop	duplicate_temporary_terms
	push	ebx
	sub	edi,6*sizeof.ExpressionTerm
	lea	esi,[edi+2*sizeof.ExpressionTerm]
	call	multiply_term_values
	add	esi,sizeof.ExpressionTerm
	add	edi,sizeof.ExpressionTerm
	call	multiply_term_values
	add	edi,sizeof.ExpressionTerm
	call	add_term_values
	add	esi,2*sizeof.ExpressionTerm
	add	edi,2*sizeof.ExpressionTerm
	call	add_term_values
	mov	esi,edi
	sub	edi,2*sizeof.ExpressionTerm
	call	multiply_term_values
	add	edi,sizeof.ExpressionTerm
	mov	eax,[esi-3*sizeof.ExpressionTerm+ExpressionTerm.attributes]
	mov	edx,[esi-3*sizeof.ExpressionTerm+ExpressionTerm.value]
	mov	[esi+ExpressionTerm.attributes],eax
	mov	[esi+ExpressionTerm.value],edx
	mov	eax,[edi-3*sizeof.ExpressionTerm+ExpressionTerm.attributes]
	mov	edx,[edi-3*sizeof.ExpressionTerm+ExpressionTerm.value]
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],edx
	call	add_term_values
	sub	edi,sizeof.ExpressionTerm
	sub	esi,sizeof.ExpressionTerm
	call	subtract_term_values
	mov	eax,[esp]
	xor	dl,dl
	shld	edx,eax,3
	shl	eax,3
	call	shift_term_value_left
	mov	esi,edi
	sub	edi,2*sizeof.ExpressionTerm
	call	add_term_values
	mov	esi,edi
	add	edi,sizeof.ExpressionTerm
	pop	eax
	xor	dl,dl
	shld	edx,eax,4
	shl	eax,4
	call	shift_term_value_left
	pop	[value_position]
	mov	eax,[edi+ExpressionTerm.attributes]
	mov	edx,[edi+ExpressionTerm.value]
	pop	edi
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],edx
	call	add_term_values
	mov	[free_temporary_terms],esi
	pop	esi
	test	edi,edi
	retn
    split_value:
	cmp	ebx,[esi]
	jae	too_small_to_split
	mov	eax,ebx
	inc	eax
	stosd
	mov	ecx,ebx
	lodsd
	shr	ecx,2
	rep	movsd
	mov	ecx,ebx
	and	ecx,11b
	rep	movsb
	mov	ecx,eax
	xor	al,al
	stosb
	sub	ecx,ebx
	mov	eax,ecx
	stosd
	shr	ecx,2
	rep	movsd
	mov	ecx,eax
	and	ecx,11b
	rep	movsb
	retn
      too_small_to_split:
	lodsd
	mov	ecx,eax
	stosd
	shr	ecx,2
	rep	movsd
	mov	ecx,eax
	and	ecx,11b
	rep	movsb
	xor	eax,eax
	stosd
	retn
    multiply_by_dword:
	sets	al
	mov	[multiplier_sign],al
	movzx	eax,al
	or	eax,[multiplier]
	jz	zero_product
	mov	ebx,esi
	mov	edi,esi
	call	get_numeric_term_value
	mov	esi,edx
	mov	ecx,[esi]
	add	ecx,16
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointers_ready_for_multiplication_by_dword
	mov	edi,ebx
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointers_ready_for_multiplication_by_dword:
	mov	eax,[esi]
	and	eax,not 11b
	mov	[edi],eax
	add	esi,4
	add	edi,4
	xor	ebx,ebx
	xor	ecx,ecx
      multiply_dwords_by_dword:
	cmp	ecx,[edi-4]
	je	dwords_multiplied_by_dword
	mov	eax,[esi+ecx]
	mul	[multiplier]
	add	eax,ebx
	adc	edx,0
	mov	ebx,edx
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	multiply_dwords_by_dword
      dwords_multiplied_by_dword:
	mov	edx,[esi-4]
	and	edx,11b
	jz	only_sign_in_last_dword
	cmp	edx,2
	jb	single_byte_in_last_dword
	je	two_bytes_in_last_dword
	movsx	eax,byte [esi+ecx+2]
	shl	eax,16
	mov	ax,[esi+ecx]
	jmp	last_dword_ready
      two_bytes_in_last_dword:
	movsx	eax,word [esi+ecx]
	jmp	last_dword_ready
      single_byte_in_last_dword:
	movsx	eax,byte [esi+ecx]
	jmp	last_dword_ready
      only_sign_in_last_dword:
	movsx	eax,byte [esi+ecx-1]
	sar	eax,8
      last_dword_ready:
	mov	[edi-4],eax
	mul	[multiplier]
	add	eax,ebx
	adc	edx,0
	mov	[edi+ecx],eax
	xor	eax,eax
	mov	ebx,[esi-4]
	test	byte [esi+ebx-1],80h
	jz	product_extension
	sub	edx,[multiplier]
	sbb	eax,0
      product_extension:
	mov	[edi+ecx+4],edx
	add	ecx,8
	mov	[edi+ecx],eax
	cmp	[multiplier_sign],0
	je	product_ready
	mov	edx,[esi-4]
	and	edx,not 11b
	xor	ecx,ecx
	xor	al,al
      adjust_product:
	cmp	ecx,edx
	je	finish_product_adjustment
	neg	al
	mov	eax,[esi+ecx]
	sbb	[edi+ecx+4],eax
	setc	al
	add	ecx,4
	jmp	adjust_product
      finish_product_adjustment:
	neg	al
	mov	eax,[edi-4]
	cdq
	sbb	[edi+ecx+4],eax
	sbb	[edi+ecx+8],edx
	add	ecx,8
      product_ready:
	mov	esi,[source_term]
	jmp	value_calculated
    zero_product:
    ; zf already set
	mov	esi,[source_term]
	mov	edi,[destination_term]
	mov	[edi+ExpressionTerm.value],zero_value
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	retn

divide_term_values:
; in:
;  esi - divisor ExpressionTerm, to be replaced with remainder
;  edi - dividend ExpressionTerm, to be replaced with quotient
; out:
;  zf set if remainder is zero
; preserves: esi, edi
	mov	[source_term],esi
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	eax,[edx]
	test	byte [edx+4+eax-1],80h
	jnz	negative_dividend
	xchg	edi,esi
	call	get_numeric_term_value
	mov	eax,[edx]
	test	byte [edx+4+eax-1],80h
	jnz	negative_divisor
	mov	edi,divisor
	mov	ecx,4
	call	fit_value
	jnc	divide_by_dword
	xor	eax,eax
	mov	ebx,[edx]
      find_divisor_length:
	mov	al,[edx+4+ebx-1]
	test	al,al
	jnz	divisor_length_ok
	dec	ebx
	jmp	find_divisor_length
      divisor_length_ok:
	bsr	ecx,eax
	inc	cl
	mov	[long_divisor_length],ebx
	mov	[high_bits_count],cl
	mov	ebx,[edx+4+ebx-5]
	shrd	ebx,eax,cl
	inc	ebx
	mov	[divisor],ebx
	mov	eax,[source_term]
	mov	[long_divisor_term],eax
	mov	eax,[destination_term]
	mov	[long_dividend_term],eax
	mov	edi,[free_temporary_terms]
	add	[free_temporary_terms],4*sizeof.ExpressionTerm
	mov	[division_temporary_terms],edi
	xor	eax,eax
	mov	[edi+ExpressionTerm.attributes],eax
    compare_dividend_with_divisor:
	mov	esi,edx
	mov	edi,[long_dividend_term]
	call	get_term_value
	xor	eax,eax
	mov	ebx,[edx]
      find_dividend_length:
	mov	al,[edx+4+ebx-1]
	test	al,al
	jnz	dividend_length_ok
	dec	ebx
	jmp	find_dividend_length
      dividend_length_ok:
	cmp	ebx,[long_divisor_length]
	ja	long_division
	jb	dividend_to_remainder
	bsr	ecx,eax
	inc	cl
	cmp	cl,[high_bits_count]
	ja	long_division
	jb	dividend_to_remainder
      compare_with_divisor_bytes:
	mov	al,[edx+4+ebx-1]
	cmp	al,[esi+4+ebx-1]
	ja	subtract_divisor
	jb	dividend_to_remainder
	dec	ebx
	jnz	compare_with_divisor_bytes
	mov	esi,[long_divisor_term]
	mov	edi,[long_dividend_term]
	mov	eax,EXPR_NUMBER
	mov	[esi+ExpressionTerm.attributes],eax
	mov	[esi+ExpressionTerm.value],zero_value
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],singular_value
	jmp	accomodate_quotient
    dividend_to_remainder:
	mov	esi,[division_temporary_terms]
	mov	[free_temporary_terms],esi
	mov	eax,[esi+ExpressionTerm.attributes]
	mov	edx,[esi+ExpressionTerm.value]
	test	eax,eax
	jnz	quotient_ready
	mov	eax,EXPR_NUMBER
	mov	edx,zero_value
      quotient_ready:
	mov	esi,[long_divisor_term]
	mov	edi,[long_dividend_term]
	xchg	eax,[edi+ExpressionTerm.attributes]
	xchg	edx,[edi+ExpressionTerm.value]
	mov	[esi+ExpressionTerm.attributes],eax
	mov	[esi+ExpressionTerm.value],edx
	jmp	test_remainder
    subtract_divisor:
	mov	esi,[long_divisor_term]
	mov	edi,[long_dividend_term]
	call	subtract_term_values
	mov	eax,EXPR_NUMBER
	mov	edx,singular_value
	xchg	eax,[edi+ExpressionTerm.attributes]
	xchg	edx,[edi+ExpressionTerm.value]
	mov	[esi+ExpressionTerm.attributes],eax
	mov	[esi+ExpressionTerm.value],edx
    accomodate_quotient:
	mov	esi,[division_temporary_terms]
	mov	[free_temporary_terms],esi
	cmp	[esi+ExpressionTerm.attributes],0
	je	quotient_complete
	call	add_term_values
      quotient_complete:
	mov	esi,[long_divisor_term]
      test_remainder:
	cmp	dword [esi],0
	retn
    long_division:
	mov	edi,[division_temporary_terms]
	add	edi,sizeof.ExpressionTerm
	mov	esi,[long_dividend_term]
	mov	eax,[esi+ExpressionTerm.attributes]
	mov	edx,[esi+ExpressionTerm.value]
	mov	[edi+ExpressionTerm.attributes],eax
	mov	[edi+ExpressionTerm.value],edx
	mov	ecx,8
	cmp	[divisor],0
	je	shift_dividend_right
	add	cl,32
      shift_dividend_right:
	xor	edx,edx
	mov	eax,[long_divisor_length]
	shld	edx,eax,3
	shl	eax,3
	sub	cl,[high_bits_count]
	sub	eax,ecx
	sbb	edx,0
	call	shift_term_value_right
	cmp	[divisor],0
	je	quotient_approximation_ready
	mov	esi,edi
	mov	[destination_term],esi
	add	edi,sizeof.ExpressionTerm
	mov	[source_term],edi
	call	divide_by_dword
      quotient_approximation_ready:
	mov	esi,[division_temporary_terms]
	cmp	[esi+ExpressionTerm.attributes],0
	jne	accumulate_quotient
	mov	eax,[edi+ExpressionTerm.attributes]
	mov	edx,[edi+ExpressionTerm.value]
	mov	[esi+ExpressionTerm.attributes],eax
	mov	[esi+ExpressionTerm.value],edx
	jmp	calculate_remainder
      accumulate_quotient:
	xchg	esi,edi
	call	add_term_values
	mov	edi,esi
      calculate_remainder:
	mov	esi,[long_divisor_term]
	call	multiply_term_values
	mov	esi,edi
	mov	edi,[long_dividend_term]
	call	subtract_term_values
	mov	edi,[long_divisor_term]
	call	get_term_value
	jmp	compare_dividend_with_divisor
    negative_divisor:
	call	negate_term_value
	xchg	edi,esi
	call	divide_term_values
	pushf
	call	negate_term_value
	popf
	retn
    negative_dividend:
	call	negate_term_value
	xchg	edi,esi
	call	get_numeric_term_value
	mov	eax,[edx]
	test	byte [edx+4+eax-1],80h
	jnz	negative_dividend_and_divisor
	xchg	edi,esi
	call	divide_term_values
	call	negate_term_value
	jmp	negative_remainder
    negative_dividend_and_divisor:
	call	negate_term_value
	xchg	edi,esi
	call	divide_term_values
    negative_remainder:
	xchg	edi,esi
	call	negate_term_value
	xchg	edi,esi
	retn
    divide_by_dword:
	cmp	[divisor],0
	je	division_by_zero
	mov	ebx,esi
	mov	edi,esi
	call	get_term_value
	mov	esi,edx
	mov	ecx,[esi]
	add	ecx,4+9
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointers_ready_for_division_by_dword
	mov	edi,ebx
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointers_ready_for_division_by_dword:
	mov	eax,[esi]
	add	esi,4
	add	edi,4
	mov	ecx,eax
	and	ecx,not 11b
	xor	edx,edx
	and	eax,11b
	jz	first_dword_for_division_ready
	cmp	eax,2
	jb	single_byte_in_first_dword
	je	two_bytes_in_first_dword
	mov	dl,[esi+ecx+2]
	shl	edx,16
      two_bytes_in_first_dword:
	mov	dh,[esi+ecx+1]
      single_byte_in_first_dword:
	mov	dl,[esi+ecx]
      first_dword_for_division_ready:
	mov	eax,edx
	xor	edx,edx
	div	[divisor]
	mov	[edi+ecx],eax
      divide_dwords:
	sub	ecx,4
	jc	dwords_divided
	mov	eax,[esi+ecx]
	div	[divisor]
	mov	[edi+ecx],eax
	jmp	divide_dwords
      dwords_divided:
	mov	ecx,[esi-4]
	and	ecx,not 11b
	add	ecx,4
	call	optimize_positive_value
	lea	ebx,[edi-4]
	mov	[ebx],ecx
	lea	edi,[edi+ecx+4]
	mov	ecx,5
	mov	[edi],edx
	mov	[edi+4],ch
	call	optimize_positive_value
	lea	eax,[edi-4]
	mov	[eax],ecx
	sub	ebx,[value_workspace.memory_start]
	mov	edi,[destination_term]
	mov	[edi+ExpressionTerm.value],ebx
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	sub	eax,[value_workspace.memory_start]
	mov	esi,[source_term]
	mov	[esi+ExpressionTerm.value],eax
	mov	[esi+ExpressionTerm.attributes],EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	add	eax,4
	add	eax,ecx
	mov	[value_position],eax
	test	ecx,ecx
	retn
    optimize_positive_value:
	mov	al,[edi+ecx-1]
	test	al,al
	js	zero_extension_needed
	jnz	positive_value_optimized
	dec	ecx
	jnz	optimize_positive_value
      positive_value_optimized:
	retn
      zero_extension_needed:
	inc	ecx
	retn
    division_by_zero:
	mov	edx,_indeterminate_result
	call	register_error
	xor	eax,eax
	jmp	zero_product

invert_term_value_bits:
; in:
;  edi - ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,[esi]
	add	ecx,4
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointers_ready_for_bit_inversion
	mov	edi,[destination_term]
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointers_ready_for_bit_inversion:
	mov	edx,[esi]
	xor	ecx,ecx
	test	edx,edx
	jz	invert_zero
	and	edx,not 11b
	add	esi,4
	add	edi,4
      invert_bits_in_dwords:
	cmp	ecx,edx
	je	bits_in_dwords_inverted
	mov	eax,[esi+ecx]
	not	eax
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	invert_bits_in_dwords
      bits_in_dwords_inverted:
	mov	edx,[esi-4]
      invert_bits_in_bytes:
	cmp	ecx,edx
	je	bits_in_bytes_inverted
	mov	al,[esi+ecx]
	not	al
	mov	[edi+ecx],al
	inc	ecx
	jmp	invert_bits_in_bytes
      bits_in_bytes_inverted:
	sub	edi,4
	mov	[edi],ecx
	jmp	value_finished
      invert_zero:
	inc	ecx
	mov	[edi],ecx
	neg	cl
	mov	[edi+4],cl
	jmp	value_finished

bitwise_add_term_values:
; in:
;  esi - source ExpressionTerm
;  edi - destination ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	call	set_up_bitwise_operation
	jc	zero_product
      bitwise_add_dwords:
	cmp	ecx,edx
	je	dwords_added_bitwise
	mov	eax,[esi+ecx]
	xor	eax,[ebx+ecx]
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	bitwise_add_dwords
      dwords_added_bitwise:
	mov	edx,[ebx-4]
      bitwise_add_bytes:
	cmp	ecx,edx
	je	bytes_added_bitwise
	mov	al,[esi+ecx]
	xor	al,[ebx+ecx]
	mov	[edi+ecx],al
	inc	ecx
	jmp	bitwise_add_bytes
      bytes_added_bitwise:
	movsx	ebx,byte [ebx+ecx-1]
	sar	ebx,8
	mov	edx,[esi-4]
	sub	edx,ecx
	and	edx,not 11b
	add	edx,ecx
      bitwise_add_supernumerary_dwords:
	cmp	ecx,edx
	je	supernumerary_dwords_added_bitwise
	mov	eax,[esi+ecx]
	xor	eax,ebx
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	bitwise_add_supernumerary_dwords
      supernumerary_dwords_added_bitwise:
	mov	edx,[esi-4]
      bitwise_add_supernumerary_bytes:
	cmp	ecx,edx
	je	supernumerary_bytes_added_bitwise
	mov	al,[esi+ecx]
	xor	al,bl
	mov	[edi+ecx],al
	inc	ecx
	jmp	bitwise_add_supernumerary_bytes
      supernumerary_bytes_added_bitwise:
	dec	ecx
	mov	esi,[source_term]
	jmp	value_calculated
    set_up_bitwise_operation:
	mov	[source_term],esi
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	ebx,[edx]
	mov	edi,esi
	call	get_numeric_term_value
	mov	ecx,[edx]
	mov	eax,ecx
	or	eax,ebx
	jz	bitwise_zero_with_zero
	cmp	ecx,ebx
	jae	bitwise_result_length_estimated
	mov	ecx,ebx
      bitwise_result_length_estimated:
	add	ecx,4
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	edx,value_workspace
	call	reserve_workspace
	mov	edi,[destination_term]
	call	get_term_value
	mov	ebx,edx
	mov	edi,esi
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	edx,[ebx]
	cmp	edx,[esi]
	jb	shorter_value_selected
	mov	edx,[esi]
	xchg	ebx,esi
      shorter_value_selected:
	and	edx,not 11b
	add	ebx,4
	add	esi,4
	add	edi,4
	xor	al,al
	xor	ecx,ecx
	retn
      bitwise_zero_with_zero:
	stc
	retn

bitwise_multiply_term_values:
; in:
;  esi - source ExpressionTerm
;  edi - destination ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	call	set_up_bitwise_operation
	jc	zero_product
      bitwise_multiply_dwords:
	cmp	ecx,edx
	je	dwords_multiplied_bitwise
	mov	eax,[esi+ecx]
	and	eax,[ebx+ecx]
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	bitwise_multiply_dwords
      dwords_multiplied_bitwise:
	mov	edx,[ebx-4]
      bitwise_multiply_bytes:
	cmp	ecx,edx
	je	bytes_multiplied_bitwise
	mov	al,[esi+ecx]
	and	al,[ebx+ecx]
	mov	[edi+ecx],al
	inc	ecx
	jmp	bitwise_multiply_bytes
      bytes_multiplied_bitwise:
	movsx	ebx,byte [ebx+ecx-1]
	sar	ebx,8
	mov	edx,[esi-4]
	sub	edx,ecx
	and	edx,not 11b
	add	edx,ecx
      bitwise_multiply_supernumerary_dwords:
	cmp	ecx,edx
	je	supernumerary_dwords_multiplied_bitwise
	mov	eax,[esi+ecx]
	and	eax,ebx
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	bitwise_multiply_supernumerary_dwords
      supernumerary_dwords_multiplied_bitwise:
	mov	edx,[esi-4]
      bitwise_multiply_supernumerary_bytes:
	cmp	ecx,edx
	je	supernumerary_bytes_multiplied_bitwise
	mov	al,[esi+ecx]
	and	al,bl
	mov	[edi+ecx],al
	inc	ecx
	jmp	bitwise_multiply_supernumerary_bytes
      supernumerary_bytes_multiplied_bitwise:
	dec	ecx
	mov	esi,[source_term]
	jmp	value_calculated

bitwise_inclusive_or_of_term_values:
; in:
;  esi - source ExpressionTerm
;  edi - destination ExpressionTerm
; out:
;  zf set if result is zero
; preserves: esi, edi
	call	set_up_bitwise_operation
	jc	zero_product
      inclusive_or_of_dwords:
	cmp	ecx,edx
	je	performed_inclusive_or_of_dwords
	mov	eax,[esi+ecx]
	or	eax,[ebx+ecx]
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	inclusive_or_of_dwords
      performed_inclusive_or_of_dwords:
	mov	edx,[ebx-4]
      inclusive_or_of_bytes:
	cmp	ecx,edx
	je	performed_inclusive_or_of_bytes
	mov	al,[esi+ecx]
	or	al,[ebx+ecx]
	mov	[edi+ecx],al
	inc	ecx
	jmp	inclusive_or_of_bytes
      performed_inclusive_or_of_bytes:
	movsx	ebx,byte [ebx+ecx-1]
	sar	ebx,8
	mov	edx,[esi-4]
	sub	edx,ecx
	and	edx,not 11b
	add	edx,ecx
      inclusive_or_of_supernumerary_dwords:
	cmp	ecx,edx
	je	performed_inclusive_or_of_supernumerary_dwords
	mov	eax,[esi+ecx]
	or	eax,ebx
	mov	[edi+ecx],eax
	add	ecx,4
	jmp	inclusive_or_of_supernumerary_dwords
      performed_inclusive_or_of_supernumerary_dwords:
	mov	edx,[esi-4]
      inclusive_or_of_supernumerary_bytes:
	cmp	ecx,edx
	je	performed_inclusive_or_of_supernumerary_bytes
	mov	al,[esi+ecx]
	or	al,bl
	mov	[edi+ecx],al
	inc	ecx
	jmp	inclusive_or_of_supernumerary_bytes
      performed_inclusive_or_of_supernumerary_bytes:
	dec	ecx
	mov	esi,[source_term]
	jmp	value_calculated

shift_term_value_left:
; in:
;  edi - ExpressionTerm
;  dl:eax = number of bits
; out:
;  zf set if result is zero
; preserves: esi, edi
	mov	ebx,eax
	and	al,8-1
	mov	[bit_shift],al
	shrd	ebx,edx,3
	shr	dl,3
	mov	[shift_overflow],dl
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	eax,[edx]
      check_for_shifted_zero:
	test	eax,eax
	jz	shift_left_ok
	cmp	byte [edx+4+eax-1],0
	jne	reserve_memory_for_shift_left
	dec	eax
	jmp	check_for_shifted_zero
      reserve_memory_for_shift_left:
	cmp	[shift_overflow],0
	jne	out_of_memory
	push	esi
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,[esi]
	add	ecx,ebx
	jc	out_of_memory
	add	ecx,5
	jc	out_of_memory
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointers_ready_for_shift_left
	mov	edi,[destination_term]
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointers_ready_for_shift_left:
	add	edi,4
	mov	ecx,ebx
	push	ecx
	shr	ecx,2
	xor	eax,eax
	rep	stosd
	mov	ecx,ebx
	and	ecx,11b
	xor	al,al
	rep	stosb
	mov	cl,[bit_shift]
	mov	edx,[esi]
	add	esi,4
	xor	ebx,ebx
      shift_bytes_left:
	cmp	ebx,edx
	je	bytes_shifted_left
	mov	ah,[esi+ebx]
	shl	eax,cl
	mov	[edi+ebx],ah
	mov	al,[esi+ebx]
	inc	ebx
	jmp	shift_bytes_left
      bytes_shifted_left:
	movsx	eax,byte [esi+ebx-1]
	shl	eax,cl
	mov	[edi+ebx],ah
	pop	ecx
	sub	edi,ecx
	add	ecx,ebx
	pop	esi
	jmp	value_calculated
      shift_left_ok:
	retn

shift_term_value_right:
; in:
;  edi - ExpressionTerm
;  dl:eax = number of bits
; out:
;  zf set if result is zero
; preserves: esi, edi
	push	esi
	mov	ebx,eax
	and	al,8-1
	mov	[bit_shift],al
	shrd	ebx,edx,3
	shr	dl,3
	jz	byte_shift_ok
	or	ebx,-1
      byte_shift_ok:
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,[esi]
	sub	ecx,ebx
	jnc	size_of_shift_result_estimated
	xor	ecx,ecx
      size_of_shift_result_estimated:
	add	ecx,5
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointers_ready_for_shift_right
	mov	edi,[destination_term]
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointers_ready_for_shift_right:
	add	edi,4
	mov	cl,[bit_shift]
	mov	edx,[esi]
	sub	edx,ebx
	jbe	value_dissipated
	lea	esi,[esi+4+ebx]
	xor	ebx,ebx
      shift_bytes_right:
	inc	ebx
	cmp	ebx,edx
	je	bytes_shifted_right
	mov	ax,[esi+ebx-1]
	shr	ax,cl
	mov	[edi+ebx-1],al
	jmp	shift_bytes_right
      bytes_shifted_right:
	dec	ebx
	movsx	eax,byte [esi+ebx]
	shr	eax,cl
	mov	[edi+ebx],al
	mov	ecx,ebx
	pop	esi
	jmp	value_calculated
      value_dissipated:
	mov	edx,[esi]
	movsx	eax,byte [esi+4+edx-1]
	mov	[edi],ah
	xor	ecx,ecx
	pop	esi
	jmp	value_calculated

find_first_set_bit_in_term_value:
; in:
;  esi - ExpressionTerm
; out:
;  zf set when value is zero
;  when zf = 0:
;   dl:eax = index of first set bit
; preserves: esi, edi
	xchg	edi,esi
	call	get_numeric_term_value
	xchg	edi,esi
	mov	ebx,edx
	mov	edx,[ebx]
	and	edx,not 11b
	xor	ecx,ecx
	add	ebx,4
      find_first_set_bit_in_dwords:
	cmp	ecx,edx
	je	no_set_bit_found_in_dwords
	bsf	eax,[ebx+ecx]
	jnz	first_set_bit_found
	add	ecx,4
	jmp	find_first_set_bit_in_dwords
      no_set_bit_found_in_dwords:
	mov	edx,[ebx-4]
      find_first_set_bit_in_bytes:
	cmp	ecx,edx
	je	no_set_bit_found
	movzx	eax,byte [ebx+ecx]
	bsf	eax,eax
	jnz	first_set_bit_found
	inc	ecx
	jmp	find_first_set_bit_in_bytes
      no_set_bit_found:
	retn
      first_set_bit_found:
	xor	dl,dl
	shld	edx,ecx,3
	shl	ecx,3
	add	eax,ecx
	adc	dl,0
	inc	dh
	retn

find_last_set_bit_in_term_value:
; in:
;  esi - ExpressionTerm
; out:
;  cf set when value is zero or negative
;  when cf = 0:
;   dl:eax = index of last set bit
; preserves: esi, edi
	xchg	edi,esi
	call	get_numeric_term_value
	xchg	edi,esi
	mov	ebx,edx
	mov	edx,[ebx]
	add	ebx,4
	test	byte [ebx+edx-1],80h
	jnz	last_set_bit_unreachable
	mov	ecx,edx
	and	edx,11b
      find_last_set_bit_in_dwords:
	cmp	ecx,edx
	je	find_last_set_bit_in_bytes
	sub	ecx,4
	bsr	eax,[ebx+ecx]
	jnz	last_set_bit_found
	jmp	find_last_set_bit_in_dwords
      find_last_set_bit_in_bytes:
	jecxz	last_set_bit_unreachable
	dec	ecx
	movzx	eax,byte [ebx+ecx]
	bsr	eax,eax
	jnz	last_set_bit_found
	jmp	find_last_set_bit_in_bytes
      last_set_bit_unreachable:
	stc
	retn
      last_set_bit_found:
	xor	dl,dl
	shld	edx,ecx,3
	shl	ecx,3
	add	eax,ecx
	adc	dl,0
       ; clc
	retn

truncate_term_value:
; in:
;  edi - ExpressionTerm
; preserves: edi
	cmp	byte [edi+ExpressionTerm.attributes],EXPR_FLOAT
	jne	get_numeric_term_value
	mov	[destination_term],edi
	call	get_term_value
	mov	ecx,[edx+FloatData.exponent]
	cmp	ecx,0
	jl	no_integer_part
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	shr	ecx,5
	lea	ecx,[4+(ecx+1)*4]
	mov	edx,value_workspace
	call	reserve_workspace
	jnc	pointers_ready_for_truncation
	mov	edi,[destination_term]
	call	get_term_value
	mov	esi,edx
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
      pointers_ready_for_truncation:
	add	edi,4
	mov	ecx,32*MANTISSA_SEGMENTS
	sub	ecx,[esi+FloatData.exponent]
	jbe	integer_precision_loss
	dec	ecx
	mov	edx,ecx
	and	ecx,11111b
	shr	edx,5
	neg	edx
	add	edx,MANTISSA_SEGMENTS
	mov	ebx,[esi+FloatData.mantissa+(edx-1)*4]
      extract_integer_bits:
	mov	eax,ebx
	dec	edx
	jz	extract_highest_integer_bits
	mov	ebx,[esi+FloatData.mantissa+(edx-1)*4]
	shrd	eax,ebx,cl
	stosd
	jmp	extract_integer_bits
      extract_highest_integer_bits:
	shr	eax,cl
	stosd
      integer_part_ready:
	and	byte [edi],0
	dec	ecx
	sar	ecx,3
	neg	ecx
	add	ecx,edi
	mov	edi,[destination_term]
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER + EXPRF_VALUE_IN_WORKSPACE
	mov	edx,[value_position]
	mov	[edi+ExpressionTerm.value],edx
	sub	ecx,[value_workspace.memory_start]
	mov	[value_position],ecx
	sub	ecx,edx
	sub	ecx,4
	add	edx,[value_workspace.memory_start]
	mov	[edx],ecx
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	jnz	negate_term_value
	retn
      integer_precision_loss:
	neg	ecx
	inc	ecx
	mov	edx,ecx
	shr	ecx,5
	xor	eax,eax
	rep	stosd
	mov	ecx,edx
	and	ecx,11111b
	mov	edx,MANTISSA_SEGMENTS-1
	mov	eax,[esi+FloatData.mantissa+edx*4]
	shl	eax,cl
	stosd
      extract_significant_bits:
	mov	eax,[esi+FloatData.mantissa+(edx-1)*4]
	mov	ebx,[esi+FloatData.mantissa+edx*4]
	shld	eax,ebx,cl
	stosd
	dec	edx
	jnz	extract_significant_bits
	mov	eax,[esi+FloatData.mantissa+edx*4]
	neg	ecx
	and	ecx,11111b
	jnz	extract_highest_integer_bits
	xor	eax,eax
	jmp	integer_part_ready
      no_integer_part:
	mov	[edi+ExpressionTerm.value],zero_value
	mov	[edi+ExpressionTerm.attributes],EXPR_NUMBER
	retn

reverse_term_value_bytes:
; in:
;  edi - ExpressionTerm
;  ebx = number of bytes
; preserves: esi, edi
	push	esi
	mov	[destination_term],edi
	call	get_numeric_term_value
	mov	edi,[value_position]
	add	edi,[value_workspace.memory_start]
	mov	ecx,ebx
	add	ecx,4
	mov	edx,value_workspace
	call	reserve_workspace
	mov	edi,[destination_term]
	call	get_term_value
	mov	[edi+ExpressionTerm.attributes],EXPR_STRING + EXPRF_VALUE_IN_WORKSPACE
	mov	esi,[value_position]
	mov	[edi+ExpressionTerm.value],esi
	lea	eax,[esi+4+ebx]
	mov	[value_position],eax
	add	esi,[value_workspace.memory_start]
	mov	[esi],ebx
	xor	ecx,ecx
	test	ebx,ebx
	jz	reversed_bytes_ready
     store_reversed_bytes:
	cmp	ecx,[edx]
	jae	extend_reversed_bytes
	mov	al,[edx+4+ecx]
	mov	[esi+4+ebx-1],al
	inc	ecx
	dec	ebx
	jnz	store_reversed_bytes
     reversed_bytes_ready:
	cmp	ecx,[edx]
	jae	reversed_bytes_ok
	mov	al,[edx+4+ecx]
	cbw
	mov	al,ah
	lea	edi,[edx+4+ecx]
	neg	ecx
	add	ecx,[edx]
	repe	scasb
	je	reversed_bytes_ok
	mov	edx,_value_out_of_range
	call	register_error
	jmp	reversed_bytes_ok
     extend_reversed_bytes:
	mov	al,[edx+4+ecx-1]
	cbw
	mov	al,ah
	lea	edi,[esi+4]
	mov	ecx,ebx
	rep	stosb
     reversed_bytes_ok:
	mov	edi,[destination_term]
	pop	esi
	retn
