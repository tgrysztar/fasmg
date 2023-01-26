
include '8086.inc'

define @aux @aux

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
	check	@src.type = 'imm'
	jyes	push_imm

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

    push_imm:
	check	@src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
	jyes	push_simm
	check	@src.imm relativeto 0 & @src.imm - 10000h >= -80h & @src.imm < 10000h
	jyes	push_simm_wrap
	emit	1, 68h
	asm	dw @src.imm
	exit
      push_simm_wrap:
	compute @src.imm, @src.imm - 10000h
      push_simm:
	emit	1, 6Ah
	emit	1, @src.imm
	exit

end calminstruction

calminstruction x86.pop_instruction size:0,dest*

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
	check	@dest.rm = 1
	jyes	invalid_operand
	emit	1, 7 + @dest.rm shl 3
	exit

end calminstruction

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

iterate <instr,opcode>, pusha,60h, popa,61h, insb,6Ch, outsb,6Eh, insw,6Dh, outsw,6Fh, leave,0C9h

	calminstruction instr?
		emit	1, opcode
	end calminstruction

end iterate

calminstruction imul? dest*,src&

	local	size

	call	x86.parse_operand@dest, dest

	match	, src
	jyes	imul_rm

	local	src1, src2

	match	src1 =, src2, src
	jyes	imul_second_source

	call	x86.parse_operand@src, src

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

	check	@src.type = 'imm' & @dest.type = 'reg'
	jno	invalid_combination_of_operands

	compute @aux.type, @src.type
	compute @aux.imm, @src.imm
	compute @src.type, @dest.type
	compute @src.mod, @dest.mod
	compute @src.rm, @dest.rm

	jump	main

    imul_second_source:
	call	x86.parse_operand@src, src1
	call	x86.parse_operand@aux, src2

	check	@dest.size = 0 & @src.size = 0 & @aux.size = 0
	jyes	operand_size_not_specified

	compute size, @dest.size or @src.size or @aux.size

	check	(@dest.size & @dest.size <> size) | (@src.size & @src.size <> size) | (@aux.size & @aux.size <> size)
	jyes	operand_sizes_do_not_match

	jump	main

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    main:
	check	@aux.type = 'imm' & ( @src.type = 'mem' | @src.type = 'reg' ) & @dest.type = 'reg' & size = 2
	jyes	imul_reg_rm_imm

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    imul_rm:
	check	@dest.size
	jyes	imul_rm_size_ok
	err	'operand size not specified'
      imul_rm_size_ok:
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jno	invalid_combination_of_operands
	check	@dest.size < 2
	jyes	imul_rm_8bit
	xcall	x86.store_instruction@dest, (0F7h),(5)
	exit
      imul_rm_8bit:
	xcall	x86.store_instruction@dest, (0F6h),(5)
	exit

    imul_reg_rm_imm:
	check	@aux.imm relativeto 0 & @aux.imm < 80h & @aux.imm >= -80h
	jyes	imul_reg_rm_simm
	check	@aux.imm relativeto 0 & @aux.imm - 10000h >= -80h & @aux.imm < 10000h
	jyes	imul_reg_rm_simm_wrap
	xcall	x86.store_instruction@src, (69h),@dest.rm,size,@aux.imm
	exit
      imul_reg_rm_simm_wrap:
	compute @aux.imm, @aux.imm - 1 shl (size shl 3)
      imul_reg_rm_simm:
	xcall	x86.store_instruction@src, (6Bh),@dest.rm,byte,@aux.imm
	exit

end calminstruction

iterate <instr,postbyte>, rol,0, ror,1, rcl,2, rcr,3, shl,4, sal, 4, shr,5, sar,7

	calminstruction instr? dest*,cnt*

		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, cnt

		check	@dest.size = 0
		jyes	operand_size_not_specified
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
		check	@dest.size < 2
		jyes	shift_rm8_imm
		check	@src.imm = 1
		jyes	shift_rm_1
		xcall	x86.store_instruction@dest, (0C1h),(postbyte),byte,@src.imm
		exit
	      shift_rm_1:
		xcall	x86.store_instruction@dest, (0D1h),(postbyte)
		exit
	      shift_rm8_imm:
		check	@src.imm = 1
		jyes	shift_rm8_1
		xcall	x86.store_instruction@dest, (0C0h),(postbyte),byte,@src.imm
		exit
	      shift_rm8_1:
		xcall	x86.store_instruction@dest, (0D0h),(postbyte)
		exit

	    operand_size_not_specified:
		err	'operand size not specified'
		jump	main
	    invalid_operand_size:
		err	'invalid operand size'
		jump	main

	end calminstruction

end iterate

calminstruction ins? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
	compute size, 0
    size_ok:
	check	@src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_combination_of_operands
	check	@dest.size = 2
	jyes	ins_word
	assemble x86.insb
	exit
    ins_word:
	assemble x86.insw
	exit
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction outs? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'mem' & @src.mod = 0 & @src.rm = 4
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	@src.size = 2
	jyes	outs_word
	assemble x86.outsb
	exit
    outs_word:
	assemble x86.outsw
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

calminstruction enter? alloc*,nesting*
	call	x86.parse_operand@src, alloc
	call	x86.parse_operand@aux, nesting
	check	(@src.size and not 2) | (@aux.size and not 1)
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	check	@src.type = 'imm' & @aux.type = 'imm'
	jno	operand_ok
	err	'invalid operand'
	exit
    operand_ok:
	emit	1, 0C8h
	asm	dw @src.imm
	emit	1, @aux.imm
end calminstruction

calminstruction bound? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.type = 'mem' & @dest.type = 'reg'
	jno	invalid_combination_of_operands
	check	@dest.size = 2
	jno	invalid_operand_size
	check	@src.size and not @dest.size
	jno	size_ok
	err	'operand sizes do not match'
    size_ok:
	xcall	x86.store_instruction@src, (62h),@dest.rm
	exit
    invalid_operand_size:
	err	'invalid operand size'
    invalid_combination_of_operands:
	err	'invalid combination of operands'
end calminstruction
