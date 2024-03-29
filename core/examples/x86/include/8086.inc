
define x86 x86

element x86.reg
element x86.r8	: x86.reg + 1
element x86.r16 : x86.reg + 2

element al? : x86.r8 + 0
element cl? : x86.r8 + 1
element dl? : x86.r8 + 2
element bl? : x86.r8 + 3
element ah? : x86.r8 + 4
element ch? : x86.r8 + 5
element dh? : x86.r8 + 6
element bh? : x86.r8 + 7

element ax? : x86.r16 + 0
element cx? : x86.r16 + 1
element dx? : x86.r16 + 2
element bx? : x86.r16 + 3
element sp? : x86.r16 + 4
element bp? : x86.r16 + 5
element si? : x86.r16 + 6
element di? : x86.r16 + 7

element x86.sreg

element es? : x86.sreg + 0
element cs? : x86.sreg + 1
element ss? : x86.sreg + 2
element ds? : x86.sreg + 3

define x86.byte? :1
define x86.word? :2
define x86.dword? :4


calminstruction calminstruction?.initsym? var*, val&
	publish var, val
end calminstruction

calminstruction calminstruction?.asm? line&
	local	name, i
	initsym name, name.0
	match	name.i, name
	compute i, i+1
	arrange name, name.i
	publish name, line
	arrange line, =assemble name
	assemble line
end calminstruction

calminstruction calminstruction?.xcall? instruction*, arguments&
	arrange instruction, =call instruction
    convert:
	match	, arguments
	jyes	ready
	local	v
	match	v=,arguments, arguments, <>
	jyes	recognize
	arrange v, arguments
	arrange arguments,
    recognize:
	match	(v), v
	jyes	numeric
	match	<v>, v
	jyes	symbolic
    append:
	arrange instruction, instruction=,v
	jump	convert
    numeric:
	compute v, v
    symbolic:
	local proxy, base, i
	initsym base, proxy
	initsym i, 0
	compute i, i+1
	arrange proxy, base.i
	publish proxy, v
	arrange instruction, instruction=,proxy
	jump	convert
    ready:
	assemble instruction
end calminstruction


define @dest @dest
define @src @src
define @aux @aux

iterate context, @dest,@src,@src2,@aux

	namespace context

		iterate name,	size, type, segment_prefix, \
				imm, unresolved, displacement, displacement_size, \
				address, address_registers, segment, offset, jump_type, \
				mod, rm,
			define name
		end iterate

		calminstruction x86.parse_operand#context operand

			local	i, pre, suf, sym

			compute segment_prefix, 0

			compute size, 0
			compute displacement_size, 0

			transform operand

			match	pre suf, operand
			jno	no_size_prefix
			transform pre, x86
			jno	no_size_prefix
			match	:size, pre
			jno	no_size_prefix
			arrange operand, suf
		      no_size_prefix:

			match	[address], operand
			jyes	memory_operand
			match	=ptr? address, operand
			jyes	memory_operand
			match	segment:offset, operand
			jyes	far_operand

		  immediate_operand:
			compute type, 'imm'
			compute imm, +operand

			compute unresolved, 0
			check	defined operand
			jyes	operand_resolved
			compute unresolved, 1
		      operand_resolved:

			check	imm eq 1 elementof imm
			jno	operand_ready
			check	1 metadataof (1 metadataof imm) relativeto x86.reg
			jyes	register_operand
			check	1 metadataof imm relativeto x86.sreg
			jyes	segment_register_operand

		      operand_ready:
			exit

		  register_operand:

			compute type, 'reg'
			compute mod, 11b
			compute rm, 1 metadataof imm - 1 elementof (1 metadataof imm)
			check	size & size <> 1 metadataof (1 metadataof imm) - x86.reg
			jyes	operand_sizes_do_not_match
			compute size, 1 metadataof (1 metadataof imm) - x86.reg

			exit

		  segment_register_operand:

			compute type, 'sreg'
			compute mod, 11b
			compute rm, 1 metadataof imm - x86.sreg
			check	size and not 2
			jyes	operand_sizes_do_not_match
			compute size, 2

			exit

		  memory_operand:
			compute type, 'mem'

			match	segment:address, address
			jno	segment_prefix_ok
			check	segment eq 1 elementof segment & 1 metadataof segment relativeto x86.sreg
			jno	invalid_operand
			compute segment, 1 metadataof segment - x86.sreg
			compute segment_prefix, 26h + segment shl 3
		      segment_prefix_ok:

			check	size
			jyes	size_override
			compute size, sizeof address
		      size_override:

			compute address, address
			compute address_registers, 0
			compute i, 1
		      extract_registers:
			check	i > elementsof address
			jyes	registers_extracted
			check	i metadataof address relativeto x86.r16 | i metadataof address relativeto x86.r32
			jno	next_term
			compute address_registers, address_registers + i elementof address * i scaleof address
		      next_term:
			compute i, i+1
			jump	extract_registers
		      registers_extracted:
			compute displacement, address - address_registers

			check	address_registers eq 0
			jyes	direct_address
			check	address_registers relativeto bx+si
			jyes	rm_0
			check	address_registers relativeto bx+di
			jyes	rm_1
			check	address_registers relativeto bp+si
			jyes	rm_2
			check	address_registers relativeto bp+di
			jyes	rm_3
			check	address_registers relativeto si
			jyes	rm_4
			check	address_registers relativeto di
			jyes	rm_5
			check	address_registers relativeto bp
			jyes	rm_6
			check	address_registers relativeto bx
			jyes	rm_7
			jump	invalid_address

		      direct_address:
			compute mod, 0
			compute rm, 6
			compute displacement_size, 2
			exit

		      rm_0:
			compute rm, 0
			jump	rm_ok
		      rm_1:
			compute rm, 1
			jump	rm_ok
		      rm_2:
			compute rm, 2
			jump	rm_ok
		      rm_3:
			compute rm, 3
			jump	rm_ok
		      rm_4:
			compute rm, 4
			jump	rm_ok
		      rm_5:
			compute rm, 5
			jump	rm_ok
		      rm_6:
			compute rm, 6
			jump	rm_ok
		      rm_7:
			compute rm, 7
		      rm_ok:

			check	displacement relativeto 0
			jno	displacement_16bit
			check	displacement = 0 & rm <> 6
			jyes	displacement_empty
			check	displacement<80h & displacement>=-80h
			jyes	displacement_8bit
			check	displacement-10000h>=-80h & displacement<10000h
			jyes	displacement_8bit_wrap_16bit
		      displacement_16bit:
			compute displacement_size, 2
			compute mod, 2
			exit
		      displacement_empty:
			compute displacement_size, 0
			compute mod, 0
			exit
		      displacement_8bit_wrap_16bit:
			compute displacement, displacement-10000h
		      displacement_8bit:
			compute displacement_size, 1
			compute mod, 1
			exit

		  far_operand:
			compute type, 'far'

			check	size and not 4
			jyes	operand_sizes_do_not_match

			exit

		  invalid_operand:
			err	'invalid operand'
			exit
		  invalid_operand_size:
			err	'invalid operand size'
			exit
		  operand_sizes_do_not_match:
			err	'operand sizes do not match'
			exit
		  invalid_address:
			err	'invalid address'
			exit
		  invalid_address_size:
			err	'invalid address size'
			exit

		end calminstruction

		calminstruction x86.parse_jump_operand#context operand

			match	=far? operand, operand
			jyes	far_jump
			match	=near? operand, operand
			jyes	near_jump
			match	=short? operand, operand
			jyes	short_jump
			compute jump_type, ''
			jump	parse_operand
		    far_jump:
			compute jump_type, 'far'
			jump	parse_operand
		    near_jump:
			compute jump_type, 'near'
			jump	parse_operand
		    short_jump:
			compute jump_type, 'short'

		    parse_operand:

			call	x86.parse_operand#context, operand

		end calminstruction

		calminstruction x86.store_instruction#context opcode*,reg*,imm_size:0,immediate

			check	segment_prefix
			jno	segment_prefix_ok
			check	rm = 2 | rm = 3 | ( mod > 0 & rm = 6 )
			jyes	ss_segment_default
		    ds_segment_default:
			check	segment_prefix = 3Eh
			jyes	segment_prefix_ok
			jump	store_segment_prefix
		    ss_segment_default:
			check	segment_prefix = 36h
			jyes	segment_prefix_ok
		    store_segment_prefix:
			emit	1, segment_prefix
		    segment_prefix_ok:

			emit	1, opcode
			emit	1, mod shl 6 + reg shl 3 + rm

			check	displacement_size = 1
			jyes	displacement_8bit
			check	displacement_size = 2
			jno	displacement_ok
		    displacement_16bit:
			asm	dw displacement
			jump	displacement_ok
		    displacement_8bit:
			emit	1, displacement
		    displacement_ok:

			check	imm_size = 1
			jyes	immediate_8bit
			check	imm_size = 2
			jno	immediate_ok
		    immediate_16bit:
			compute imm, +immediate
			asm	dw imm
			jump	immediate_ok
		    immediate_8bit:
			compute imm, +immediate
			emit	1, imm
		    immediate_ok:

		end calminstruction

	end namespace

end iterate


iterate <instr,basecode>, add,0, or,8, adc,10h, sbb,18h, and,20h, sub,28h, xor,30h, cmp,38h

	calminstruction instr? dest*,src*

		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src

		local	opcode, rm, size

		compute opcode, basecode

		check	@dest.size = 0 & @src.size = 0
		jyes	operand_size_not_specified
		check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		jyes	operand_sizes_do_not_match

		compute size, @dest.size or @src.size

		check	size > 2
		jyes	invalid_operand_size

	    main:

		check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	reg_rm
		check	@src.type = 'mem' & @dest.type = 'reg'
		jyes	rm_reg
		check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	rm_imm

		err	'invalid combination of operands'
		exit

	    operand_size_not_specified:
		err	'operand size not specified'
		compute size, 0
		jump	main

	    operand_sizes_do_not_match:
		err	'operand sizes do not match'
		compute size, 0
		jump	main

	    invalid_operand_size:
		err	'invalid operand size'
		compute size, 0
		jump	main

	    reg_rm:
		check	size = 2
		jno	reg_rm_store
		compute opcode, opcode + 1
	    reg_rm_store:
		call	x86.store_instruction@dest, opcode,@src.rm
		exit

	    rm_reg:
		compute opcode, opcode + 2
		check	size = 2
		jno	rm_reg_store
		compute opcode, opcode + 1
	    rm_reg_store:
		call	x86.store_instruction@src, opcode,@dest.rm
		exit

	    rm_imm:
		check	size = 2
		jyes	rm_imm_word
		check	@dest.type = 'reg' & @dest.rm = 0
		jyes	al_imm

		compute opcode, opcode shr 3
		xcall	x86.store_instruction@dest, (80h),opcode,byte,@src.imm
		exit

	    al_imm:
		emit	1, opcode+4
		emit	1, @src.imm
		exit

	    rm_imm_word:

		check	@src.imm relativeto 0 & @src.imm<80h & @src.imm>=-80h
		jyes	rm_simm
		check	@src.imm relativeto 0 & @src.imm-10000h>=-80h & @src.imm<10000h
		jyes	rm_simm_wrap
		check	@dest.type = 'reg' & @dest.rm = 0
		jyes	ax_imm

		compute rm, opcode shr 3
		xcall	x86.store_instruction@dest, (81h),rm,size,@src.imm
		exit

	    ax_imm:
		emit	1, opcode+5
		asm	dw @src.imm
		exit

	    rm_simm_wrap:
		compute @src.imm, @src.imm - 10000h

	    rm_simm:
		compute rm, opcode shr 3
		xcall	x86.store_instruction@dest, (83h),rm,byte,@src.imm

	end calminstruction

end iterate

iterate <instr,postbyte>, not,2, neg,3, mul,4, imul,5, div,6, idiv,7

	calminstruction instr? src*

		call	x86.parse_operand@src, src

		check	@src.size = 0
		jyes	operand_size_not_specified

		check	@src.size > 2
		jyes	invalid_operand_size

	    main:
		check	@src.type = 'mem' | @src.type = 'reg'
		jno	invalid_operand
		check	@src.size = 2
		jyes	rm_word

		xcall	x86.store_instruction@src, (0F6h),(postbyte)
		exit

	    rm_word:
		xcall	x86.store_instruction@src, (0F7h),(postbyte)
		exit

	    operand_size_not_specified:
		err	'operand size not specified'
		jump	main

	    invalid_operand_size:
		err	'invalid operand size'
		jump	main

	    invalid_operand:
		err	'invalid operand'
		exit

	end calminstruction

end iterate

calminstruction mov? dest*,src*

	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

	check	size > 2
	jyes	invalid_operand_size

    main:

	check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_reg_rm
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	mov_mem_reg
	check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_imm
	check	@src.type = 'sreg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_sreg
	check	@dest.type = 'sreg' & @dest.rm <> 1 & ( @src.type = 'reg' | @src.type = 'mem' )
	jyes	mov_sreg_rm

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    invalid_operand_size:
	err	'invalid operand size'
	compute size, 0
	jump	main

    mov_reg_rm:
	check	@src.type = 'reg' & @dest.type = 'mem' & @src.rm = 0 & @dest.address_registers eq 0
	jyes	mov_ax_dirmem
	check	size < 2
	jyes	mov_reg_rm_8bit
	xcall	x86.store_instruction@dest, (89h),@src.rm
	exit
      mov_reg_rm_8bit:
	xcall	x86.store_instruction@dest, (88h),@src.rm
	exit

    mov_mem_reg:
	check	@src.type = 'mem' & @dest.type = 'reg' & @dest.rm = 0 & @src.address_registers eq 0
	jyes	mov_dirmem_ax
	check	size < 2
	jyes	mov_mem_reg_8bit
	xcall	x86.store_instruction@src, (8Bh),@dest.rm
	exit
      mov_mem_reg_8bit:
	xcall	x86.store_instruction@src, (8Ah),@dest.rm
	exit

    mov_ax_dirmem:
	check	@dest.segment_prefix = 0 | @dest.segment_prefix = 3Eh
	jyes	dest_seg_ok
	emit	1, @dest.segment_prefix
      dest_seg_ok:
	check	size < 2
	jyes	mov_al_dirmem
	emit	1, 0A3h
	asm	dw @dest.address
	exit
      mov_al_dirmem:
	emit	1, 0A2h
	asm	dw @dest.address
	exit

    mov_dirmem_ax:
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	src_seg_ok
	emit	1, @src.segment_prefix
      src_seg_ok:
	check	size < 2
	jyes	mov_dirmem_al
	emit	1, 0A1h
	asm	dw @src.address
	exit
      mov_dirmem_al:
	emit	1, 0A0h
	asm	dw @src.address
	exit

    mov_rm_imm:
	check	@dest.type = 'mem'
	jyes	mov_mem_imm

    mov_reg_imm:
	check	size < 2
	jyes	mov_reg_imm_8bit
	emit	1, 0B8h + @dest.rm
	asm	dw @src.imm
	exit
      mov_reg_imm_8bit:
	emit	1, 0B0h + @dest.rm
	emit	1, @src.imm
	exit

    mov_mem_imm:
	check	size < 2
	jyes	mov_mem_imm_8bit
	xcall	x86.store_instruction@dest, (0C7h),(0),size,@src.imm
	exit
      mov_mem_imm_8bit:
	xcall	x86.store_instruction@dest, (0C6h),(0),byte,@src.imm
	exit

    mov_rm_sreg:
	check	size < 2
	jyes	invalid_operand_size
	xcall	x86.store_instruction@dest, (8Ch),@src.rm
	exit

    mov_sreg_rm:
	check	size < 2
	jyes	invalid_operand_size
	xcall	x86.store_instruction@src, (8Eh),@dest.rm
	exit

end calminstruction

calminstruction test? dest*,src*

	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

	check	size > 2
	jyes	invalid_operand_size

    main:

	check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	test_reg_rm
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	test_mem_reg
	check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	test_rm_imm

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    invalid_operand_size:
	err	'invalid operand size'
	compute size, 0
	jump	main

    test_reg_rm:
	check	size < 2
	jyes	test_reg_rm_8bit
	xcall	x86.store_instruction@dest, (85h),@src.rm
	exit
      test_reg_rm_8bit:
	xcall	x86.store_instruction@dest, (84h),@src.rm
	exit

    test_mem_reg:
	check	size < 2
	jyes	test_mem_reg_8bit
	xcall	x86.store_instruction@src, (85h),@dest.rm
	exit
      test_mem_reg_8bit:
	xcall	x86.store_instruction@src, (84h),@dest.rm
	exit

    test_rm_imm:
	check	size < 2
	jyes	test_rm_imm_8bit
	check	@dest.type = 'reg' & @dest.rm = 0
	jyes	test_ax_imm
	xcall	x86.store_instruction@dest, (0F7h),(0),size,@src.imm
	exit

      test_ax_imm:
	emit	1, 0A9h
	asm	dw @src.imm
	exit

      test_rm_imm_8bit:
	check	@dest.type = 'reg' & @dest.rm = 0
	jyes	test_al_imm
	xcall	x86.store_instruction@dest, (0F6h),(0),byte,@src.imm
	exit
      test_al_imm:
	emit	1, 0A8h
	emit	1, @src.imm
	exit

end calminstruction

calminstruction xchg? dest*,src*

	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

	check	size > 2
	jyes	invalid_operand_size

    main:

	check	@src.type = 'reg' & @dest.type = 'reg'
	jyes	xchg_reg_reg
	check	@src.type = 'reg' & @dest.type = 'mem'
	jyes	xchg_reg_rm
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	xchg_rm_reg

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    invalid_operand_size:
	err	'invalid operand size'
	compute size, 0
	jump	main

    xchg_reg_reg:
	check	@src.rm = 0 | @dest.rm = 0
	jno	xchg_rm_reg
	check	size < 2
	jyes	xchg_rm_reg_8bit
	emit	1, 90h + @src.rm + @dest.rm
	exit

    xchg_reg_rm:
	check	size < 2
	jyes	xchg_reg_rm_8bit
	xcall	x86.store_instruction@dest, (87h),@src.rm
	exit
      xchg_reg_rm_8bit:
	xcall	x86.store_instruction@dest, (86h),@src.rm
	exit

    xchg_rm_reg:
	check	size < 2
	jyes	xchg_rm_reg_8bit
	xcall	x86.store_instruction@src, (87h),@dest.rm
	exit
      xchg_rm_reg_8bit:
	xcall	x86.store_instruction@src, (86h),@dest.rm
	exit

end calminstruction

iterate <instr,postbyte>, inc,0 ,dec,1

	calminstruction instr? dest*

		call	x86.parse_operand@dest, dest

		check	@dest.size > 2
		jyes	invalid_operand_size
		check	@dest.size
		jyes	main

		err	'operand size not specified'
		jump	main

	    invalid_operand_size:
		err	'invalid operand size'
		jump	main

	    main:
		check	@dest.type = 'reg'
		jyes	inc_reg
		check	@dest.type = 'mem'
		jyes	inc_rm

		err	'invalid operand'
		exit

	    inc_reg:
		check	@dest.size < 2
		jyes	inc_rm_8bit
		emit	1, 40h + @dest.rm + postbyte shl 3
		exit

	    inc_rm:
		check	@dest.size < 2
		jyes	inc_rm_8bit
		xcall	x86.store_instruction@dest, (0FFh),(postbyte)
		exit
	      inc_rm_8bit:
		xcall	x86.store_instruction@dest, (0FEh),(postbyte)

	end calminstruction

end iterate

calminstruction push? src*

	call	x86.parse_operand@src, src

	check	@src.size and not 2
	jno	main

	err	'invalid operand size'

    main:
	check	@src.type = 'mem'
	jyes	push_mem
	check	@src.type = 'reg'
	jyes	push_reg
	check	@src.type = 'sreg'
	jyes	push_sreg

	err	'invalid operand'
	exit

    push_mem:
	xcall	x86.store_instruction@src, (0FFh),(110b)
	exit

    push_reg:
	emit	1, 50h + @src.rm
	exit

    push_sreg:
	emit	1, 6 + @src.rm shl 3
	exit

end calminstruction

calminstruction pop? dest*

	call	x86.parse_operand@dest, dest

	check	@dest.size and not 2
	jno	main

	err	'invalid operand size'

    main:
	check	@dest.type = 'mem'
	jyes	pop_mem
	check	@dest.type = 'reg'
	jyes	pop_reg
	check	@dest.type = 'sreg'
	jyes	pop_sreg

    invalid_operand:
	err	'invalid operand'
	exit

    pop_mem:
	xcall	x86.store_instruction@dest, (8Fh),(0)
	exit

    pop_reg:
	emit	1, 58h + @dest.rm
	exit

    pop_sreg:
	emit	1, 7 + @dest.rm shl 3
	exit

end calminstruction

iterate reg, ax,cx,dx,bx,sp,bp,si,di, es,cs,ss,ds
	define x86.compact.reg? {reg}
end iterate

iterate <instr>, push,pop

	calminstruction instr? operand

		local	head, tail

		match	head tail, operand
		jno	plain
		transform head, x86.compact
		jno	plain
		match	{head}, head
		jno	plain
	    loop:
		arrange operand, =instr head
		assemble operand
		match	head tail, tail
		jno	final
		transform head, x86.compact
		jno	error
		match	{head}, head
		jyes	loop
	    error:
		err	'only register operands allowed in compact syntax'
		exit
	    final:
		transform tail, x86.compact
		jno	error
		match	{operand}, tail
		jno	error
	    plain:
		arrange operand, =instr operand
		assemble operand

	end calminstruction

end iterate

iterate <instr,opcode>, ret,0C2h, retn,0C2h, retf,0CAh

	calminstruction instr? operand
		match	, operand
		jyes	ret_short
		check	operand
		jno	ret_short
		emit	1, opcode
		asm	dw operand
		exit
	    ret_short:
		emit	1, opcode + 1
	end calminstruction

end iterate

calminstruction lea? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.type = 'mem' & @dest.type = 'reg'
	jno	invalid_combination_of_operands
	check	@dest.size = 2
	jyes	ok
	err	'invalid operand size'
    ok:
	xcall	x86.store_instruction@src, (8Dh),@dest.rm
	exit
    invalid_combination_of_operands:
	err	'invalid combination of operands'
end calminstruction

iterate <instr,opcode>, les,0C4h, lds,0C5h

	calminstruction instr? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@dest.size and not 2 | @src.size and not 4
		jyes	invalid_operand_size
		check	@src.type = 'mem' & @dest.type = 'reg'
		jno	invalid_combination_of_operands
		xcall	x86.store_instruction@src, (opcode),@dest.rm
		exit
	    invalid_operand_size:
		err	'invalid operand size'
		exit
	    invalid_combination_of_operands:
		err	'invalid combination of operands'
	end calminstruction

end iterate

iterate <instr,postbyte>, rol,0, ror,1, rcl,2, rcr,3, shl,4, sal, 4, shr,5, sar,7

	calminstruction instr? dest*,cnt*

		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, cnt

		check	@dest.size = 0
		jyes	operand_size_not_specified
		check	@dest.size > 2
		jyes	invalid_operand_size
		check	@src.size and not 1
		jyes	invalid_operand_size

	    main:
		check	@src.type = 'reg' & @src.size = 1 & @src.rm = 1 & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shift_rm_cl
		check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shift_rm_imm

		err	'invalid combination of operands'
		exit

	    shift_rm_cl:
		check	@dest.size < 2
		jyes	shift_r8_cl
		xcall	x86.store_instruction@dest, (0D3h),(postbyte)
		exit
	      shift_r8_cl:
		xcall	x86.store_instruction@dest, (0D2h),(postbyte)
		exit

	    shift_rm_imm:
		compute @src.imm, @src.imm and (1 shl (@dest.size shl 3) - 1)
		check	@dest.size < 2
		jyes	shift_rm8_imm
	      shift_rm16_imm:
		check	@src.imm
		jno	done
		xcall	x86.store_instruction@dest, (0D1h),(postbyte)
		compute @src.imm, @src.imm - 1
		jump	shift_rm16_imm
	      shift_rm8_imm:
		check	@src.imm
		jno	done
		xcall	x86.store_instruction@dest, (0D0h),(postbyte)
		compute @src.imm, @src.imm - 1
		jump	shift_rm8_imm
	      done:
		exit

	    operand_size_not_specified:
		err	'operand size not specified'
		jump	main
	    invalid_operand_size:
		err	'invalid operand size'
		jump	main

	end calminstruction

end iterate

calminstruction call? dest*

	call	x86.parse_jump_operand@dest, dest

	check	@dest.type = 'imm'
	jyes	call_imm
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jyes	call_rm
	check	@dest.type = 'far'
	jyes	call_direct_far

    invalid_operand:
	err	'invalid operand'
	exit

    call_direct_far:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	check	@dest.size and not 4
	jyes	invalid_operand
	emit	1, 9Ah
	asm	dw @dest.offset,@dest.segment
	exit

    call_rm:
	check	@dest.size = 4
	jyes	call_rm_dword
	check	@dest.size = 2
	jyes	call_rm_word
	check	@dest.size
	jyes	invalid_operand
	check	@dest.jump_type = 'far'
	jyes	call_rm_far
	check	@dest.jump_type = 'near'
	jyes	call_rm_near
	err	'operand size not specified'
	exit

      call_rm_dword:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
      call_rm_far:
	xcall	x86.store_instruction@dest, (0FFh),(11b)
	exit

      call_rm_word:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
      call_rm_near:
	xcall	x86.store_instruction@dest, (0FFh),(10b)
	exit

    call_imm:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	check	@dest.imm relativeto 0 & (@dest.imm < 0 | @dest.imm >= 10000h)
	jyes	value_out_of_range
	emit	1, 0E8h
	compute @dest.imm, @dest.imm-($+2)
	asm	dw @dest.imm
	exit

    value_out_of_range:
	err	'value out of range'

end calminstruction

calminstruction jmp? dest*

	call	x86.parse_jump_operand@dest, dest

	check	@dest.type = 'imm'
	jyes	jmp_imm
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jyes	jmp_rm
	check	@dest.type = 'far'
	jyes	jmp_direct_far

    invalid_operand:
	err	'invalid operand'
	exit

    jmp_direct_far:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	check	@dest.size and not 4
	jyes	invalid_operand
	emit	1, 0EAh
	asm	dw @dest.offset,@dest.segment
	exit

    jmp_rm:
	check	@dest.size = 4
	jyes	jmp_rm_dword
	check	@dest.size = 2
	jyes	jmp_rm_word
	check	@dest.size
	jyes	invalid_operand
	check	@dest.jump_type = 'far'
	jyes	jmp_rm_far
	check	@dest.jump_type = 'near'
	jyes	jmp_rm_near
	err	'operand size not specified'
	exit

      jmp_rm_dword:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
      jmp_rm_far:
	xcall	x86.store_instruction@dest, (0FFh),(101b)
	exit

      jmp_rm_word:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
      jmp_rm_near:
	xcall	x86.store_instruction@dest, (0FFh),(100b)
	exit

    jmp_imm:
	check	@dest.imm relativeto 0 & (@dest.imm < 0 | @dest.imm >= 10000h)
	jyes	value_out_of_range
	check	@dest.jump_type = 'near'
	jyes	jmp_imm_near
	check	@dest.jump_type = 'short'
	jyes	jmp_imm_short_verify
	check	@dest.jump_type
	jyes	invalid_operand
	check	~ $ relativeto 0 & @dest.imm relativeto 0
	jno	jmp_optimize
	compute @dest.imm, @dest.imm + $ - 0 scaleof $
	err	'invalid address'
      jmp_optimize:
	check	@dest.unresolved
	jyes	jmp_imm_short
	check	@dest.imm relativeto $
	jno	jmp_imm_near
	check	(@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h
	jyes	jmp_imm_short
      jmp_imm_near:
	emit	1, 0E9h
	compute @dest.imm, @dest.imm-($+2)
	asm	dw @dest.imm
	exit
      jmp_imm_short_verify:
	check	(@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h
	jno	relative_jump_out_of_range
      jmp_imm_short:
	emit	1, 0EBh
	compute @dest.imm, (@dest.imm-($+1)) and 0FFh
	emit	1, @dest.imm
	exit

    relative_jump_out_of_range:
	emit	2
	err	'relative jump out of range'
	exit

    value_out_of_range:
	err	'value out of range'

end calminstruction

x86.jumps = 0

calminstruction jumps?
	compute x86.jumps, 1
end calminstruction

calminstruction nojumps?
	compute x86.jumps, 0
end calminstruction

iterate <instr,opcode>, jo,70h, jno,71h, jc,72h, jb,72h, jnae,72h, jnc,73h, jnb,73h, jae,73h, jz,74h, je,74h, jnz,75h, jne,75h, jna,76h, jbe,76h, ja,77h, jnbe,77h, \
			js,78h, jns,79h, jp,7Ah, jpe,7Ah, jnp,7Bh, jpo,7Bh, jl,7Ch, jnge,7Ch, jnl,7Dh, jge,7Dh, jng,7Eh, jle,7Eh, jg,7Fh, jnle,7Fh

	calminstruction instr? dest*

		call	x86.parse_jump_operand@dest, dest

		check	@dest.type <> 'imm' & ( @dest.jump_type & @dest.jump_type <> 'short' )
		jyes	invalid_operand

		check	@dest.imm relativeto 0 & (@dest.imm < 0 | @dest.imm >= 10000h)
		jyes	value_out_of_range

		check	@dest.unresolved | ( @dest.imm-($+2) < 80h & @dest.imm-($+2) >= -80h )
		jyes	in_range
		check	(@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h
		jyes	in_range

		check	x86.jumps & ~ @dest.jump_type
		jyes	trampoline

		emit	1
		emit	1
		err	'relative jump out of range'
		exit

	    in_range:
		compute @dest.imm, (@dest.imm-($+2)) and 0FFh
		emit	1, opcode
		emit	1, @dest.imm
		exit

	    trampoline:
		emit	1, opcode xor 1
		emit	1, 3
		emit	1, 0E9h
		compute @dest.imm, @dest.imm-($+2)
		asm	dw @dest.imm
		exit

	    invalid_operand:
		err	'invalid operand'
		exit

	    value_out_of_range:
		err	'value out of range'

	end calminstruction
end iterate

iterate <instr,opcode>, loopnz,0E0h, loopne,0E0h, loopz,0E1h, loope,0E1h, loop,0E2h, jcxz,0E3h

	calminstruction instr? dest*

		call	x86.parse_jump_operand@dest, dest

		check	@dest.type = 'imm' & ( @dest.jump_type = 'short' | ~ @dest.jump_type )
		jno	invalid_operand

		emit	1, opcode

		check	@dest.imm-($+1) < 80h & @dest.imm-($+1) >= -80h
		jyes	relative_offset_ok
		check	(@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h
		jyes	relative_offset_ok
		emit	1
		err	'relative jump out of range'
		exit
	    relative_offset_ok:
		compute @dest.imm, (@dest.imm-($+1)) and 0FFh
		emit	1, @dest.imm
		exit

	    invalid_operand:
		err	'invalid operand'
		exit

	end calminstruction
end iterate

iterate <instr,opcode>, daa,27h, das,2Fh, aaa,37h, aas,3Fh, nop,90h, cbw,98h, cwd,99h, \
			int3,0CCh, into,0CEh, iret,0CFh, salc,0D6h, \
			hlt,0F4h, cmc,0F5h, clc,0F8h, stc,0F9h, cli,0FAh, sti,0FBh, cld,0FCh, std,0FDh, \
			pushf,9Ch, popf,9Dh, sahf,9Eh, lahf,9Fh, \
			movsb,0A4h, cmpsb,0A6h, stosb,0AAh, lodsb,0ACh, scasb,0AEh, xlatb,0D7h, \
			movsw,0A5h, cmpsw,0A7h, stosw,0ABh, lodsw,0ADh, scasw,0AFh

	calminstruction instr?
		emit	1, opcode
	end calminstruction

end iterate

iterate <prefix,opcode>, lock,0F0h, repnz,0F2h, repne,0F2h, rep,0F3h, repz,0F3h, repe,0F3h

	calminstruction prefix? instr&
		emit	1, opcode
		assemble instr
	end calminstruction

end iterate

calminstruction int? number*
	emit	1, 0CDh
	emit	1, number
end calminstruction

calminstruction aam? number:10
	emit	1, 0D4h
	emit	1, number
end calminstruction

calminstruction aad? number:10
	emit	1, 0D5h
	emit	1, number
end calminstruction

calminstruction movs? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	local	size
	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match
	compute size, @dest.size or @src.size
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @src.rm = 4 & @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	size = 2
	jyes	movs_word
	emit	1, 0A4h
	exit
    movs_word:
	emit	1, 0A5h
	exit
    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction cmps? src*,dest*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	local	size
	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match
	compute size, @dest.size or @src.size
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @src.rm = 4 & @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	size = 2
	jyes	cmps_word
	emit	1, 0A6h
	exit
    cmps_word:
	emit	1, 0A7h
	exit
    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction stos? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_operand
	check	@dest.size = 2
	jyes	stos_word
	emit	1, 0AAh
	exit
    stos_word:
	emit	1, 0ABh
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction lods? src*
	call	x86.parse_operand@src, src
	check	@src.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @src.rm = 4
	jno	invalid_operand
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	@src.size = 2
	jyes	lods_word
	emit	1, 0ACh
	exit
    lods_word:
	emit	1, 0ADh
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction scas? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_operand
	check	@dest.size = 2
	jyes	scas_word
	emit	1, 0AEh
	exit
    scas_word:
	emit	1, 0AFh
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction xlat? src*
	call	x86.parse_operand@src, src
	check	@src.size > 1
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @src.rm = 7
	jno	invalid_operand
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	emit	1, 0D7h
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction in? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'reg' & @dest.rm = 0
	jyes	in_ax_dx
	check	@src.type = 'imm' & @dest.type = 'reg' & @dest.rm = 0
	jyes	in_ax_imm
	err	'invalid combination of operands'
	exit
    in_ax_dx:
	check	@dest.size = 2
	jno	in_al_dx
	emit	1, 0EDh
	exit
    in_al_dx:
	emit	1, 0ECh
	exit
    in_ax_imm:
	check	@dest.size = 2
	jno	in_al_imm
	emit	1, 0E5h
	emit	1, @src.imm
	exit
    in_al_imm:
	emit	1, 0E4h
	emit	1, @src.imm
	exit
end calminstruction

calminstruction out? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'reg' & @src.rm = 0
	jyes	out_dx_ax
	check	@dest.type = 'imm' & @src.type = 'reg' & @src.rm = 0
	jyes	out_imm_ax
	err	'invalid combination of operands'
	exit
    out_dx_ax:
	check	@src.size = 2
	jno	out_dx_al
	emit	1, 0EFh
	exit
    out_dx_al:
	emit	1, 0EEh
	exit
    out_imm_ax:
	check	@src.size = 2
	jno	out_imm_al
	emit	1, 0E7h
	emit	1, @dest.imm
	exit
    out_imm_al:
	emit	1, 0E6h
	emit	1, @dest.imm
	exit
end calminstruction
