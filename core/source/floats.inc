
struct FloatData
	attributes dd ? 		 ; FLOAT_#
	exponent dd ?
	mantissa rd MANTISSA_SEGMENTS
ends

MANTISSA_SEGMENTS = 4

FLOAT_NEGATIVE = 1
FLOAT_INFINITE = 2
FLOAT_INDETERMINATE = 4
FLOAT_UNDERFLOW = 8

number_to_float:
; in:
;  esi - 32-bit length followed by binary data of that length
;  edi - FloatData
; preserves: edi
; note: the number is treated as unsigned
	xor	eax,eax
	mov	[edi+FloatData.attributes],eax
	lodsd
	mov	ecx,eax
	cmp	eax,10000000h
	ja	unsigned_too_large
	shl	eax,3
	dec	eax
	mov	[edi+FloatData.exponent],eax
	mov	edx,MANTISSA_SEGMENTS*4
	cmp	ecx,edx
	ja	unsigned_precision_loss
	xor	eax,eax
	mov	[mantissa_tail],eax
    zero_appended_segments:
	sub	edx,4
	jc	converted_mantissa_ready
	cmp	ecx,edx
	ja	appended_segments_zeroed
	mov	[edi+FloatData.mantissa+edx],eax
	jmp	zero_appended_segments
    appended_segments_zeroed:
	sub	ecx,edx
	jz	copy_significant_dwords
    read_lowest_bytes:
	lodsb
	ror	eax,8
	loop	read_lowest_bytes
	mov	[edi+FloatData.mantissa+edx],eax
	sub	edx,4
	jc	converted_mantissa_ready
    copy_significant_dwords:
	lodsd
	mov	[edi+FloatData.mantissa+edx],eax
	sub	edx,4
	jnc	copy_significant_dwords
    converted_mantissa_ready:
	mov	esi,edi
	call	normalize_float
	call	round_float
	retn
    unsigned_precision_loss:
	sub	ecx,edx
	add	esi,ecx
	mov	eax,[esi-4]
	sub	ecx,4
	jz	unsigned_tail_ready
	jc	unsigned_short_tail
	xor	dl,dl
	lea	ebx,[esi-4]
    unsigned_tail_contraction:
	dec	ebx
	or	dl,[ebx]
	loop	unsigned_tail_contraction
	setnz	dl
	and	dl,1
	or	al,dl
	jmp	unsigned_tail_ready
    unsigned_short_tail:
	neg	cl
	shl	cl,3
	shr	eax,cl
	shl	eax,cl
    unsigned_tail_ready:
	mov	[mantissa_tail],eax
	mov	edx,(MANTISSA_SEGMENTS-1)*4
	jmp	copy_significant_dwords
    unsigned_too_large:
	or	[edi+FloatData.attributes],FLOAT_INFINITE
	retn

get_float_exponent:
; in:
;  esi - FloatData
; out:
;  zf set when value is zero
;  eax = exponent of normalized value
; preserves: ebx, ecx, esi, edi
; note:
;  when value cannot be normalized, smallest possible exponent is returned
;  float attributes are ignored
	xor	eax,eax
    find_exponent:
	inc	eax
	bsr	edx,[esi+FloatData.mantissa+(eax-1)*4]
	jnz	exponent_found
	cmp	eax,MANTISSA_SEGMENTS
	jne	find_exponent
	xor	edx,edx
    minimal_exponent:
	mov	eax,-80000000h
	test	edx,edx
	retn
    exponent_found:
	assert	MANTISSA_SEGMENTS shl 5 <= 1 shl 32
	inc	edx
	shl	eax,5
	sub	edx,eax
	mov	eax,[esi+FloatData.exponent]
	add	eax,edx
	jo	minimal_exponent
	test	esi,esi
	retn

get_float_unnormalization:
; in:
;  esi - FloatData
; out:
;  cf set when value is zero
;  when cf = 0:
;   ecx = number of leading zeroes in mantissa
; preserves: ebx, edx, esi, edi
	xor	ecx,ecx
    find_highest_set_bit:
	inc	ecx
	bsr	eax,[esi+FloatData.mantissa+(ecx-1)*4]
	jnz	highest_set_bit_found
	cmp	ecx,MANTISSA_SEGMENTS
	jne	find_highest_set_bit
	stc
	retn
    highest_set_bit_found:
	assert	MANTISSA_SEGMENTS shl 5 <= 1 shl 32
	inc	eax
	shl	ecx,5
	sub	ecx,eax
       ; clc
	retn

normalize_float:
; in:
;  esi - source FloatData
;  edi - destination FloatData
;  [mantissa_tail] = additional bits of source mantissa, lowest bit indicating that there are set bits far down
; out:
;  [mantissa_tail] = additional bits of destination mantissa, lowest bit indicating that there are set bits far down
; preserves: esi, edi
; note: float attributes are ignored
	xor	edx,edx
    find_highest_bit:
	bsr	ecx,[esi+FloatData.mantissa+edx*4]
	jnz	highest_bit_found
	inc	edx
	cmp	edx,MANTISSA_SEGMENTS
	jne	find_highest_bit
	bsr	ecx,[mantissa_tail]
	jnz	highest_bit_found
	call	copy_float_data
	mov	[edi+FloatData.exponent],-80000000h
	retn
    copy_float_data:
	cmp	esi,edi
	je	float_data_ready
	assert	sizeof.FloatData and 11b = 0
	mov	ecx,sizeof.FloatData shr 2
	rep	movsd
	sub	esi,sizeof.FloatData
	sub	edi,sizeof.FloatData
    float_data_ready:
	retn
    highest_bit_found:
	not	ecx
	add	ecx,32
	mov	eax,edx
	shl	eax,5
	add	eax,ecx
	neg	eax
	jz	copy_float_data
	add	eax,[esi+FloatData.exponent]
	jo	copy_float_data
	mov	[edi+FloatData.exponent],eax
	add	edi,FloatData.mantissa
    shift_mantissa_up:
	push	edx
	xor	eax,eax
	cmp	edx,MANTISSA_SEGMENTS-1
	ja	shift_mantissa_tail_up
	mov	eax,[esi+FloatData.mantissa+edx*4]
	je	shift_mantissa_tail_up
    shift_mantissa_segment_up:
	inc	edx
	mov	ebx,[esi+FloatData.mantissa+edx*4]
	shld	eax,ebx,cl
	stosd
	mov	eax,ebx
	cmp	edx,MANTISSA_SEGMENTS-1
	jne	shift_mantissa_segment_up
    shift_mantissa_tail_up:
	mov	ebx,[mantissa_tail]
	and	ebx,not 1
	shld	eax,ebx,cl
	stosd
	mov	eax,ebx
	shl	eax,cl
	pop	ecx
	jecxz	mantissa_normalized
	stosd
	dec	ecx
	xor	eax,eax
	rep	stosd
    mantissa_normalized:
	and	[mantissa_tail],1
	or	[mantissa_tail],eax
	sub	edi,FloatData.mantissa + MANTISSA_SEGMENTS*4
	mov	eax,[esi+FloatData.attributes]
	mov	[edi+FloatData.attributes],eax
	retn

unnormalize_float:
; in:
;  esi - source FloatData
;  edi - destination FloatData
;  ecx = target exponent
;  [mantissa_tail] = additional bits of source mantissa, lowest bit indicating that there are set bits far down
; out:
;  [mantissa_tail] = additional bits of destination mantissa, lowest bit indicating that there are set bits far down
; preserves: esi, edi
; note:
;  when target exponent is smaller than the actual one, the overflowing bits are ignored
;  float attributes are ignored
	mov	eax,[esi+FloatData.exponent]
	mov	[edi+FloatData.exponent],ecx
	sub	ecx,eax
	jz	copy_float_data
	js	contrary_unnormalization
	mov	ebx,ecx
	shr	ebx,5
	and	cl,11111b
	add	edi,FloatData.mantissa + (MANTISSA_SEGMENTS-1)*4
	std
	mov	edx,MANTISSA_SEGMENTS
	sub	edx,ebx
	push	ebx
	mov	ebx,edx
	mov	edx,MANTISSA_SEGMENTS
	mov	eax,[mantissa_tail]
    collect_far_mantissa:
	cmp	edx,ebx
	je	far_mantissa_collected
	or	[mantissa_tail],eax
	mov	eax,[esi+FloatData.mantissa+(edx-1)*4]
	dec	edx
	jz	unnormalization_underflow
	jmp	collect_far_mantissa
    far_mantissa_collected:
	mov	ebx,[esi+FloatData.mantissa+(edx-1)*4]
	shrd	eax,ebx,cl
	xchg	eax,[mantissa_tail]
	test	eax,eax
	setnz	al
	and	eax,1
	or	[mantissa_tail],eax
	dec	edx
	jz	shift_first_mantissa_segment_down
    shift_mantissa_segment_down:
	mov	eax,ebx
	mov	ebx,[esi+FloatData.mantissa+(edx-1)*4]
	shrd	eax,ebx,cl
	stosd
	dec	edx
	jnz	shift_mantissa_segment_down
    shift_first_mantissa_segment_down:
	mov	eax,[esi+FloatData.mantissa]
	shr	eax,cl
	stosd
	pop	ecx
    zero_higher_mantissa_segments:
	xor	eax,eax
	rep	stosd
	sub	edi,FloatData.mantissa-4
	cld
	mov	eax,[esi+FloatData.attributes]
	mov	[edi+FloatData.attributes],eax
	retn
    unnormalization_underflow:
	cmp	edx,ebx
	je	only_mantissa_tail
	or	[mantissa_tail],eax
	xor	eax,eax
    only_mantissa_tail:
	shr	eax,cl
	xchg	eax,[mantissa_tail]
	test	eax,eax
	setnz	al
	and	eax,1
	or	[mantissa_tail],eax
	pop	ebx
	mov	ecx,MANTISSA_SEGMENTS
	jmp	zero_higher_mantissa_segments
    contrary_unnormalization:
	add	edi,FloatData.mantissa
	neg	ecx
	mov	edx,ecx
	shr	edx,5
	cmp	edx,MANTISSA_SEGMENTS
	ja	unnormalization_overflow
	and	cl,11111b
	jmp	shift_mantissa_up
    unnormalization_overflow:
	mov	ecx,MANTISSA_SEGMENTS
	xor	eax,eax
	rep	stosd
	sub	edi,FloatData.mantissa + MANTISSA_SEGMENTS*4
	retn

round_float:
; in:
;  edi - FloatData
;  [mantissa_tail] = additional bits of mantissa, lowest bit indicating that there are set bits far down
; preserves: ebx, esi, edi
	test	[mantissa_tail],1 shl 31
	jz	rounding_done
	test	[mantissa_tail],not (1 shl 31)
	jnz	round_up
	test	[edi+FloatData.mantissa+(MANTISSA_SEGMENTS-1)*4],1
	jz	rounding_done
    round_up:
	mov	edx,MANTISSA_SEGMENTS
    increment_mantissa:
	inc	[edi+FloatData.mantissa+(edx-1)*4]
	jnz	rounding_done
	dec	edx
	jnz	increment_mantissa
	or	[edi+FloatData.mantissa],80000000h
	add	[edi+FloatData.exponent],1
	jo	float_overflow
    rounding_done:
	retn
    float_overflow:
	or	[edi+FloatData.attributes],FLOAT_INFINITE
	retn

add_floats:
; in:
;  ebx - FloatData with first addend
;  esi - FloatData with second addend
;  edi - FloatData where sum should be stored
; preserves: edi
	test	[ebx+FloatData.attributes],FLOAT_NEGATIVE
	setnz	[first_sign]
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setnz	[second_sign]
    float_summator:
	call	get_float_exponent
	mov	ecx,eax
	xchg	esi,ebx
	call	get_float_exponent
	cmp	eax,ecx
	jge	significant_mantissa_selected
	xchg	esi,ebx
	mov	al,[first_sign]
	xchg	al,[second_sign]
	mov	[first_sign],al
    significant_mantissa_selected:
	push	edi
	mov	eax,[esi+FloatData.mantissa]
	test	eax,eax
	js	align_second_mantissa
	push	ebx
	mov	edi,[temporary_floats]
	and	[mantissa_tail],0
	call	normalize_float
	mov	esi,edi
	pop	ebx
    align_second_mantissa:
	push	esi
	and	[mantissa_tail],0
	mov	ecx,[esi+FloatData.exponent]
	mov	esi,ebx
	mov	edi,[temporary_floats]
	add	edi,sizeof.FloatData
	call	unnormalize_float
	mov	ebx,edi
	pop	esi
	pop	edi
	mov	eax,[esi+FloatData.attributes]
	or	eax,[ebx+FloatData.attributes]
	and	eax,FLOAT_INFINITE or FLOAT_INDETERMINATE
	mov	[edi+FloatData.attributes],eax
	mov	ecx,[esi+FloatData.exponent]
	mov	[edi+FloatData.exponent],ecx
	mov	al,[first_sign]
	xor	al,[second_sign]
	jnz	opposite_mantissa
	mov	ecx,MANTISSA_SEGMENTS
	clc
    add_mantissa_segments:
	mov	eax,[esi+FloatData.mantissa+(ecx-1)*4]
	adc	eax,[ebx+FloatData.mantissa+(ecx-1)*4]
	mov	[edi+FloatData.mantissa+(ecx-1)*4],eax
	loop	add_mantissa_segments
	jnc	sum_value_ready
	mov	ecx,[edi+FloatData.exponent]
	add	ecx,1
	jo	float_overflow
	mov	esi,edi
	call	unnormalize_float
	or	[edi+FloatData.mantissa],80000000h
	jmp	sum_value_ready
    opposite_mantissa:
	xor	ecx,ecx
    compare_mantissa_segments:
	mov	eax,[esi+FloatData.mantissa+ecx*4]
	cmp	eax,[ebx+FloatData.mantissa+ecx*4]
	ja	keep_mantissa_sources
	jb	switch_mantissa_sources
	inc	ecx
	cmp	ecx,MANTISSA_SEGMENTS
	jne	compare_mantissa_segments
    switch_mantissa_sources:
	xchg	esi,ebx
	mov	al,[first_sign]
	xchg	al,[second_sign]
	mov	[first_sign],al
	clc
	jmp	mantissa_subtraction_ready
    keep_mantissa_sources:
	neg	[mantissa_tail]
    mantissa_subtraction_ready:
	mov	ecx,MANTISSA_SEGMENTS
    subtract_mantissa_segments:
	mov	eax,[esi+FloatData.mantissa+(ecx-1)*4]
	sbb	eax,[ebx+FloatData.mantissa+(ecx-1)*4]
	mov	[edi+FloatData.mantissa+(ecx-1)*4],eax
	loop	subtract_mantissa_segments
	mov	esi,edi
	call	normalize_float
    sum_value_ready:
	xor	eax,eax
	cmp	[first_sign],0
	je	round_float
	or	[edi+FloatData.attributes],FLOAT_NEGATIVE
	jmp	round_float

subtract_floats:
; in:
;  ebx - FloatData with minuend
;  esi - FloatData with subtrahend
;  edi - FloatData where difference should be stored
; preserves: edi
	test	[ebx+FloatData.attributes],FLOAT_NEGATIVE
	setnz	[first_sign]
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setz	[second_sign]
	jmp	float_summator

multiply_float_by_unsigned_int:
; in:
;  esi - source FloatData
;  ecx = unsigned multiplier
;  edi - FloatData where multipled value should be stored
; preserves: edi
	mov	[multiplier],ecx
	cmp	esi,edi
	je	multiply_mantissa
	mov	eax,[esi+FloatData.attributes]
	mov	edx,[esi+FloatData.exponent]
	mov	[edi+FloatData.attributes],eax
	mov	[edi+FloatData.exponent],edx
    multiply_mantissa:
	xor	edx,edx
	mov	[mantissa_tail],edx
	mov	ecx,MANTISSA_SEGMENTS
    multiply_mantissa_segments:
	mov	ebx,edx
	mov	eax,[esi+FloatData.mantissa+(ecx-1)*4]
	mul	[multiplier]
	add	eax,ebx
	mov	[edi+FloatData.mantissa+(ecx-1)*4],eax
	loop	multiply_mantissa_segments
	mov	esi,edi
	bsr	ecx,edx
	jz	normalize_float
	inc	ecx
	mov	eax,ecx
	neg	cl
	add	cl,32
	shl	edx,cl
	push	edx
	mov	ecx,eax
	add	ecx,[esi+FloatData.exponent]
	jo	float_overflow
	call	unnormalize_float
	pop	eax
	or	[edi+FloatData.mantissa],eax
	jmp	round_float

multiply_floats:
; in:
;  ebx - FloatData with first factor
;  esi - FloatData with second factor
;  edi - FloatData where product should be stored
; preserves: edi
	test	[esi+FloatData.attributes],FLOAT_INFINITE or FLOAT_INDETERMINATE
	jnz	first_factor_not_zero
	call	get_float_exponent
	jz	multiply_zero_float
    first_factor_not_zero:
	xchg	esi,ebx
	test	[esi+FloatData.attributes],FLOAT_INFINITE or FLOAT_INDETERMINATE
	jnz	second_factor_not_zero
	call	get_float_exponent
	jz	multiply_zero_float
    second_factor_not_zero:
	mov	eax,[ebx+FloatData.attributes]
	mov	edx,[esi+FloatData.attributes]
	xor	eax,edx
	and	edx,FLOAT_INFINITE or FLOAT_INDETERMINATE
	or	eax,edx
	mov	[edi+FloatData.attributes],eax
	mov	eax,[ebx+FloatData.exponent]
	add	eax,[esi+FloatData.exponent]
	jo	exponent_overflow
	inc	eax
	jo	exponent_overflow
	mov	[edi+FloatData.exponent],eax
	push	edi
	mov	edi,accumulator
	xor	eax,eax
	mov	ecx,3
	rep	stosd
	mov	[mantissa_tail],eax
	mov	ecx,MANTISSA_SEGMENTS-1
	mov	edi,ecx
    multiply_low_segments:
	or	[mantissa_tail],eax
	mov	eax,[ebx+FloatData.mantissa+ecx*4]
	mul	[esi+FloatData.mantissa+edi*4]
	add	[accumulator],eax
	adc	[accumulator+4],edx
	adc	[accumulator+8],0
	dec	ecx
	inc	edi
	cmp	edi,MANTISSA_SEGMENTS
	jb	multiply_low_segments
	dec	edi
	xchg	ecx,edi
	xor	eax,eax
	xchg	eax,[accumulator+8]
	xchg	eax,[accumulator+4]
	xchg	eax,[accumulator]
	cmp	edi,0
	jge	multiply_low_segments
	xchg	eax,[mantissa_tail]
	test	eax,eax
	setnz	al
	and	eax,1
	or	[mantissa_tail],eax
	dec	ecx
	inc	edi
    multiply_high_segments:
	mov	eax,[ebx+FloatData.mantissa+ecx*4]
	mul	[esi+FloatData.mantissa+edi*4]
	add	[accumulator],eax
	adc	[accumulator+4],edx
	adc	[accumulator+8],0
	dec	ecx
	inc	edi
	cmp	ecx,0
	jge	multiply_high_segments
	inc	ecx
	xchg	ecx,edi
	xor	eax,eax
	xchg	eax,[accumulator+8]
	xchg	eax,[accumulator+4]
	xchg	eax,[accumulator]
	mov	edx,[esp]
	mov	[edx+FloatData.mantissa+ecx*4],eax
	sub	ecx,2
	ja	multiply_high_segments
	mov	eax,[ebx+FloatData.mantissa]
	mul	[esi+FloatData.mantissa]
	add	eax,[accumulator]
	adc	edx,[accumulator+4]
	pop	edi
	mov	[edi+FloatData.mantissa+4],eax
	mov	[edi+FloatData.mantissa],edx
	mov	esi,edi
	call	normalize_float
	jmp	round_float
    multiply_zero_float:
	mov	eax,[ebx+FloatData.attributes]
	test	eax,FLOAT_INFINITE or FLOAT_INDETERMINATE
	jnz	indeterminate_product
	mov	edx,[esi+FloatData.attributes]
	and	edx,FLOAT_NEGATIVE
	xor	eax,edx
	mov	[edi+FloatData.attributes],eax
	xor	eax,eax
    zero_float_result:
	mov	[edi+FloatData.exponent],eax
	xor	eax,eax
	add	edi,FloatData.mantissa
	mov	ecx,MANTISSA_SEGMENTS
	rep	stosd
	sub	edi,FloatData.mantissa + MANTISSA_SEGMENTS*4
	retn
    indeterminate_product:
	or	[edi+FloatData.attributes],FLOAT_INDETERMINATE
	retn
    exponent_overflow:
	js	float_overflow
	mov	[edi+FloatData.attributes],FLOAT_UNDERFLOW
	jmp	zero_float_result

multiply_float_by_power_of_ten:
; in:
;  esi - FloatData with number to multiply
;  ecx = signed number representing exponent
;  edi - FloatData for the result
; preserves: edi
	call	get_float_exponent
	jz	copy_float_data
	cmp	ecx,0
	je	copy_float_data
	jl	negative_power_of_ten
	mov	[exponent],ecx
	mov	ebx,edi
	mov	edi,[temporary_floats]
	xor	eax,eax
	mov	[edi+FloatData.attributes],eax
	mov	al,3
	mov	[edi+FloatData.exponent],eax
	add	edi,FloatData.mantissa
	mov	eax,0A0000000h
	stosd
	xor	eax,eax
	mov	ecx,MANTISSA_SEGMENTS-1
	rep	stosd
	mov	edi,ebx
    get_power_bit:
	shr	[exponent],1
	jnc	square_multiplier
	mov	ebx,[temporary_floats]
	call	multiply_floats
	test	[edi+FloatData.attributes],FLOAT_UNDERFLOW
	jnz	power_exhausted
    square_multiplier:
	cmp	[exponent],0
	je	power_exhausted
	push	edi
	mov	ebx,[temporary_floats]
	mov	esi,ebx
	mov	edi,ebx
	call	multiply_floats
	test	[edi+FloatData.attributes],FLOAT_UNDERFLOW
	jnz	power_of_ten_underflow
	pop	edi
	mov	esi,edi
	jmp	get_power_bit
    power_exhausted:
	retn
    negative_power_of_ten:
	neg	ecx
	mov	[exponent],ecx
	mov	ebx,edi
	mov	edi,[temporary_floats]
	xor	eax,eax
	mov	[edi+FloatData.attributes],eax
	or	eax,-4
	mov	[edi+FloatData.exponent],eax
	add	edi,FloatData.mantissa
	mov	eax,0CCCCCCCCh
	mov	ecx,MANTISSA_SEGMENTS
	dec	ecx
	rep	stosd
	inc	eax
	stosd
	mov	edi,ebx
	jmp	get_power_bit
    power_of_ten_underflow:
	pop	edi
	or	[edi+FloatData.attributes],FLOAT_UNDERFLOW
	retn

multiplicative_inverse:
; in:
;  esi - source FloatData
;  edi - destination FloatData
; preserves: edi
	and	[mantissa_tail],0
	call	normalize_float
	mov	eax,[edi+FloatData.attributes]
	test	eax,FLOAT_INFINITE
	jnz	reverse_of_infinity
	mov	ecx,[edi+FloatData.exponent]
	cmp	ecx,-80000000h
	je	float_overflow
	and	[edi+FloatData.attributes],0
	or	[edi+FloatData.exponent],-1
	push	eax ecx edi
	mov	[iterations],3 + bsr (MANTISSA_SEGMENTS+1)
	mov	ebx,edi
	mov	edi,[temporary_floats]
	add	edi,2*sizeof.FloatData
	xor	eax,eax
	mov	[edi+FloatData.attributes],eax
	inc	eax
	mov	[edi+FloatData.exponent],eax
	add	edi,FloatData.mantissa
	mov	ecx,MANTISSA_SEGMENTS
	mov	eax,0B4B4B4B4h
	rep	stosd
	assert	FloatData.mantissa + MANTISSA_SEGMENTS*4 = sizeof.FloatData
	xor	eax,eax
	mov	[edi+FloatData.attributes],eax
	mov	[edi+FloatData.exponent],eax
	add	edi,FloatData.mantissa
	mov	ecx,MANTISSA_SEGMENTS
	mov	eax,0F0F0F0F0h
	rep	stosd
	sub	edi,FloatData.mantissa + MANTISSA_SEGMENTS*4
	mov	esi,edi
	call	multiply_floats
	lea	ebx,[edi-sizeof.FloatData]
	mov	esi,edi
	mov	edi,ebx
    newton_raphson_iteration:
	call	subtract_floats
	mov	ebx,edi
	mov	esi,edi
	add	edi,sizeof.FloatData
	call	multiply_floats
	mov	ebx,[esp]
	mov	esi,edi
	call	multiply_floats
	mov	esi,edi
	sub	edi,sizeof.FloatData
	inc	[edi+FloatData.exponent]
	mov	ebx,edi
	dec	[iterations]
	jnz	newton_raphson_iteration
	pop	edi
	call	subtract_floats
	pop	ecx eax
	not	ecx
	add	[edi+FloatData.exponent],ecx
	and	[edi+FloatData.attributes],FLOAT_INFINITE or FLOAT_INDETERMINATE
	or	[edi+FloatData.attributes],eax
	retn
    reverse_of_infinity:
	and	eax,FLOAT_NEGATIVE or FLOAT_INDETERMINATE
	mov	[edi+FloatData.attributes],eax
	xor	eax,eax
	jmp	zero_float_result

divide_floats:
; in:
;  ebx - FloatData with dividend
;  esi - FloatData with divisor
;  edi - FloatData where quotient should be stored
; preserves: edi
	push	ebx
	call	multiplicative_inverse
	pop	ebx
	mov	esi,edi
	call	multiply_floats
	retn

fit_float:
; in:
;  esi - FloatData
;  ecx = number of bytes that the value has to fit into
;  edi - buffer for the specified amount of bytes
; preserves: esi, edi
	cmp	ecx,2
	je	half_precision
	cmp	ecx,4
	je	single_precision
	cmp	ecx,8
	je	double_precision
	cmp	ecx,10
	je	extended_precision
	cmp	ecx,16
	je	quadruple_precision
	mov	edx,_invalid_value
	call	register_error
	retn
    float_out_of_range:
	mov	edx,_value_out_of_range
	call	register_error
	retn
    zero_float:
	dec	ecx
	xor	al,al
	rep	stosb
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setnz	al
	ror	al,1
	stosb
	retn
    half_precision:
	call	get_float_exponent
	jz	zero_float
	mov	ebx,[esi+FloatData.exponent]
	sub	ebx,eax
	add	eax,0Fh
	js	half_precision_denormal
	je	half_precision_denormal
	cmp	eax,1Fh
	jae	float_out_of_range
	mov	ecx,10
	shl	eax,cl
	mov	[edi],ax
	inc	ebx
	call	read_mantissa_bits
	or	[edi],ax
	call	get_mantissa_rounding_increment
	add	[edi],ax
	mov	dx,1Fh shl 10
	mov	ax,dx
	and	ax,[edi]
	cmp	ax,dx
	je	float_out_of_range
      half_precision_ok:
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setnz	al
	shl	ax,15
	or	[edi],ax
	retn
      half_precision_denormal:
	neg	eax
	mov	ecx,10
	sub	ecx,eax
	jc	float_out_of_range
	call	read_mantissa_bits
	mov	[edi],ax
	call	get_mantissa_rounding_increment
	add	[edi],ax
	jz	float_out_of_range
	jmp	half_precision_ok
    single_precision:
	call	get_float_exponent
	jz	zero_float
	mov	ebx,[esi+FloatData.exponent]
	sub	ebx,eax
	add	eax,7Fh
	js	single_precision_denormal
	je	single_precision_denormal
	cmp	eax,0FFh
	jae	float_out_of_range
	mov	ecx,23
	shl	eax,cl
	mov	[edi],eax
	inc	ebx
	call	read_mantissa_bits
	or	[edi],eax
	call	get_mantissa_rounding_increment
	add	[edi],eax
	mov	edx,0FFh shl 23
	mov	eax,edx
	and	eax,[edi]
	cmp	eax,edx
	je	float_out_of_range
      single_precision_ok:
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setnz	al
	shl	eax,31
	or	[edi],eax
	retn
      single_precision_denormal:
	neg	eax
	mov	ecx,23
	sub	ecx,eax
	jc	float_out_of_range
	call	read_mantissa_bits
	mov	[edi],eax
	call	get_mantissa_rounding_increment
	add	[edi],eax
	jz	float_out_of_range
	jmp	single_precision_ok
    double_precision:
	call	get_float_exponent
	jz	zero_float
	mov	ebx,[esi+FloatData.exponent]
	sub	ebx,eax
	add	eax,3FFh
	js	double_precision_denormal
	je	double_precision_denormal
	cmp	eax,7FFh
	jae	float_out_of_range
	mov	ecx,20
	shl	eax,cl
	mov	[edi+4],eax
	inc	ebx
	call	read_mantissa_bits
	or	[edi+4],eax
	mov	ecx,32
	call	read_mantissa_bits
	mov	[edi],eax
	call	get_mantissa_rounding_increment
	xor	edx,edx
	add	[edi],eax
	adc	[edi+4],edx
	mov	edx,7FFh shl 20
	mov	eax,edx
	and	eax,[edi+4]
	cmp	eax,edx
	je	float_out_of_range
      double_precision_ok:
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setnz	al
	shl	eax,31
	or	[edi+4],eax
	retn
      double_precision_denormal:
	neg	eax
	mov	ecx,52
	sub	ecx,eax
	jc	float_out_of_range
	xor	eax,eax
	cmp	ecx,32
	jbe	double_precision_denormal_high_segment_ready
	sub	ecx,32
	call	read_mantissa_bits
	mov	ecx,32
      double_precision_denormal_high_segment_ready:
	mov	[edi+4],eax
	call	read_mantissa_bits
	mov	[edi],eax
	call	get_mantissa_rounding_increment
	xor	edx,edx
	add	[edi],eax
	jc	double_precision_denormal_carry
	jnz	double_precision_ok
	cmp	[edi+4],edx
	je	float_out_of_range
	jmp	double_precision_ok
      double_precision_denormal_carry:
	adc	[edi+4],edx
	jmp	double_precision_ok
    extended_precision:
	call	get_float_exponent
	jz	zero_float
	mov	ebx,[esi+FloatData.exponent]
	sub	ebx,eax
	add	eax,3FFFh
	js	extended_precision_denormal
	jz	extended_precision_denormal
	cmp	eax,7FFFh
	jae	float_out_of_range
	mov	[edi+8],ax
	mov	ecx,32
	call	read_mantissa_bits
	mov	[edi+4],eax
	mov	ecx,32
	call	read_mantissa_bits
	mov	[edi],eax
	call	get_mantissa_rounding_increment
	xor	edx,edx
	add	[edi],eax
	adc	[edi+4],edx
	jnc	extended_precision_ok
	or	byte [edi+7],80h
	inc	word [edi+8]
	cmp	word [edi+8],7FFFh
	je	float_out_of_range
      extended_precision_ok:
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setnz	al
	shl	ax,15
	or	[edi+8],ax
	retn
      extended_precision_denormal:
	and	word [edi+8],0
	neg	eax
	mov	ecx,63
	sub	ecx,eax
	jc	float_out_of_range
	xor	eax,eax
	cmp	ecx,32
	jbe	extended_precision_denormal_high_segment_ready
	sub	ecx,32
	call	read_mantissa_bits
	mov	ecx,32
      extended_precision_denormal_high_segment_ready:
	mov	[edi+4],eax
	call	read_mantissa_bits
	mov	[edi],eax
	call	get_mantissa_rounding_increment
	xor	edx,edx
	add	[edi],eax
	jc	extended_precision_denormal_carry
	jnz	extended_precision_ok
	cmp	[edi+4],edx
	je	float_out_of_range
	jmp	extended_precision_ok
      extended_precision_denormal_carry:
	adc	[edi+4],edx
	jns	extended_precision_ok
	inc	byte [edi+8]
	jmp	extended_precision_ok
    quadruple_precision:
	call	get_float_exponent
	jz	zero_float
	mov	ebx,[esi+FloatData.exponent]
	sub	ebx,eax
	add	eax,3FFFh
	js	quadruple_precision_denormal
	je	quadruple_precision_denormal
	cmp	eax,7FFFh
	jae	float_out_of_range
	mov	ecx,16
	shl	eax,cl
	mov	[edi+12],eax
	inc	ebx
	call	read_mantissa_bits
	or	[edi+12],eax
	mov	ecx,32
	call	read_mantissa_bits
	mov	[edi+8],eax
	mov	ecx,32
	call	read_mantissa_bits
	mov	[edi+4],eax
	mov	ecx,32
	call	read_mantissa_bits
	mov	[edi],eax
	call	get_mantissa_rounding_increment
	xor	edx,edx
	add	[edi],eax
	adc	[edi+4],edx
	adc	[edi+8],edx
	adc	[edi+12],edx
	mov	edx,7FFFh shl 16
	mov	eax,edx
	and	eax,[edi+12]
	cmp	eax,edx
	je	float_out_of_range
      quadruple_precision_ok:
	test	[esi+FloatData.attributes],FLOAT_NEGATIVE
	setnz	al
	shl	eax,31
	or	[edi+12],eax
	retn
      quadruple_precision_denormal:
	neg	eax
	mov	ecx,112
	sub	ecx,eax
	jc	float_out_of_range
	xor	eax,eax
	sub	ecx,96
	jbe	quadruple_precision_denormal_first_segment_ready
	call	read_mantissa_bits
	xor	ecx,ecx
      quadruple_precision_denormal_first_segment_ready:
	mov	[edi+12],eax
	add	ecx,96
	xor	eax,eax
	sub	ecx,64
	jbe	quadruple_precision_denormal_second_segment_ready
	call	read_mantissa_bits
	xor	ecx,ecx
      quadruple_precision_denormal_second_segment_ready:
	mov	[edi+8],eax
	add	ecx,64
	xor	eax,eax
	sub	ecx,32
	jbe	quadruple_precision_denormal_third_segment_ready
	call	read_mantissa_bits
	xor	ecx,ecx
      quadruple_precision_denormal_third_segment_ready:
	mov	[edi+4],eax
	add	ecx,32
	call	read_mantissa_bits
	mov	[edi],eax
	call	get_mantissa_rounding_increment
	xor	edx,edx
	add	[edi],eax
	jc	quadruple_precision_denormal_carry
	jnz	quadruple_precision_ok
	or	edx,[edi+4]
	or	edx,[edi+8]
	or	edx,[edi+12]
	jz	float_out_of_range
	jmp	quadruple_precision_ok
      quadruple_precision_denormal_carry:
	adc	[edi+4],edx
	adc	[edi+8],edx
	adc	[edi+12],edx
	jmp	quadruple_precision_ok
    read_mantissa_bits:
	add	ebx,ecx
	mov	edx,ebx
	dec	edx
	mov	al,dl
	shr	edx,5
	and	al,11111b
	inc	al
	mov	ch,cl
	cmp	edx,MANTISSA_SEGMENTS
	ja	out_of_mantissa_bits
	je	read_from_mantissa_border
	mov	cl,32
	sub	cl,al
	cmp	al,ch
	jae	read_from_single_mantissa_segment
	mov	eax,[esi+FloatData.mantissa+edx*4]
	mov	edx,[esi+FloatData.mantissa+(edx-1)*4]
	shrd	eax,edx,cl
	jmp	cut_out_mantissa_bits
      read_from_single_mantissa_segment:
	mov	eax,[esi+FloatData.mantissa+edx*4]
	shr	eax,cl
      cut_out_mantissa_bits:
	mov	cl,ch
	cmp	cl,32
	je	mantissa_bits_ok
	mov	edx,1
	shl	edx,cl
	dec	edx
	and	eax,edx
      mantissa_bits_ok:
	retn
      read_from_mantissa_border:
	cmp	al,32
	jae	out_of_mantissa_bits
	mov	cl,al
	mov	eax,[esi+FloatData.mantissa+(edx-1)*4]
	shl	eax,cl
	jmp	cut_out_mantissa_bits
      out_of_mantissa_bits:
	xor	eax,eax
	retn
    get_mantissa_rounding_increment:
	mov	al,bl
	shr	ebx,5
	and	al,11111b
	mov	cl,31
	sub	cl,al
	cmp	ebx,MANTISSA_SEGMENTS
	jae	no_rounding_increment
	mov	edx,1
	shl	edx,cl
	mov	eax,[esi+FloatData.mantissa+ebx*4]
	test	eax,edx
	jz	no_rounding_increment
	dec	edx
	and	eax,edx
	jnz	rounding_increment_needed
	mov	edx,ebx
      check_whole_mantissa:
	inc	edx
	cmp	edx,MANTISSA_SEGMENTS
	jae	rounding_tied
	cmp	dword [esi+FloatData.mantissa+edx*4],0
	je	check_whole_mantissa
      rounding_increment_needed:
	mov	eax,1
	retn
      rounding_tied:
	inc	cl
	cmp	cl,32
	jb	test_mantissa_parity
	dec	ebx
      test_mantissa_parity:
	mov	eax,[esi+FloatData.mantissa+ebx*4]
	shr	eax,cl
	test	eax,1
	jnz	rounding_increment_needed
      no_rounding_increment:
	xor	eax,eax
	retn
