
define x86 x86

element x86.reg
element x86.r8	: x86.reg + 1
element x86.r16 : x86.reg + 2
element x86.r32 : x86.reg + 4

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

element eax? : x86.r32 + 0
element ecx? : x86.r32 + 1
element edx? : x86.r32 + 2
element ebx? : x86.r32 + 3
element esp? : x86.r32 + 4
element ebp? : x86.r32 + 5
element esi? : x86.r32 + 6
element edi? : x86.r32 + 7

element x86.sreg

element es? : x86.sreg + 0
element cs? : x86.sreg + 1
element ss? : x86.sreg + 2
element ds? : x86.sreg + 3
element fs? : x86.sreg + 4
element gs? : x86.sreg + 5

element x86.creg

element x86.crx : x86.creg + 0
element x86.drx : x86.creg + 1
element x86.trx : x86.creg + 4

repeat 8, i:0
	element cr#i? : x86.crx + i
	element dr#i? : x86.drx + i
	element tr#i? : x86.trx + i
end repeat

define x86.byte? :1
define x86.word? :2
define x86.dword? :4
define x86.pword? :6
define x86.fword? :6
define x86.qword? :8

x86.mode = 16

macro use16?
	x86.mode = 16
end macro

macro use32?
	x86.mode = 32
end macro

define @dest @dest
define @src @src
define @src2 @src2
define @aux @aux

macro calminstruction?.asmcmd? pattern&
	local	cmd
	arrange cmd, pattern
	assemble cmd
end macro

calminstruction x86.parse_operand namespace, operand

	local	size, type, segment_prefix, prefix, opcode_prefix
	local	imm, unresolved, displacement, displacement_size
	local	address, address_registers, segment, offset
	local	mode, mod, rm
	local	scale, index, base

	local	i, pre, suf, sym

	compute segment_prefix, 0
	compute prefix, 0
	compute opcode_prefix, 0

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
	jno	export_immediate
	check	1 metadataof (1 metadataof imm) relativeto x86.reg
	jyes	register_operand
	check	1 metadataof imm relativeto x86.sreg
	jyes	segment_register_operand

	jump	export_immediate

  register_operand:

	compute type, 'reg'
	compute mode, x86.mode
	compute mod, 11b
	compute rm, 1 metadataof imm - 1 elementof (1 metadataof imm)
	check	size & size <> 1 metadataof (1 metadataof imm) - x86.reg
	jyes	operand_sizes_do_not_match
	compute size, 1 metadataof (1 metadataof imm) - x86.reg

	jump	export_register

  segment_register_operand:

	compute type, 'sreg'
	compute mode, x86.mode
	compute mod, 11b
	compute rm, 1 metadataof imm - x86.sreg
	check	size & size <> 2 & size <> 4
	jyes	invalid_operand_size

	jump	export_register

  memory_operand:
	compute type, 'mem'

	match	segment:address, address
	jno	segment_prefix_ok
	check	segment eq 1 elementof segment & 1 metadataof segment relativeto x86.sreg
	jno	invalid_operand
	compute segment, 1 metadataof segment - x86.sreg
	check	segment >= 4
	jyes	segment_prefix_386
	compute segment_prefix, 26h + segment shl 3
	jump	segment_prefix_ok
      segment_prefix_386:
	compute segment_prefix, 64h + segment-4
      segment_prefix_ok:

	compute mode, 0

	match	pre suf, address
	jno	no_address_size_prefix
	transform pre, x86
	jno	no_address_size_prefix
	match	:pre, pre
	jno	no_address_size_prefix
	arrange address, suf
	check	pre = 2 | pre = 4 | pre = 8
	jno	invalid_address_size
	compute mode, pre shl 3
      no_address_size_prefix:

	compute scale, 0
	compute index, 0
	compute base, 0

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

	check	mode
	jyes	verify_mode
	compute mode, x86.mode
	check	mode = 16 & displacement relativeto 0 & displacement >= 10000h & address_registers eq 0
	jno	mode_ok
	compute mode, 32
	jump	mode_ok
      verify_mode:
	check	(mode = 16 & 1 metadataof address_registers relativeto x86.r32) | (mode = 32 & 1 metadataof address_registers relativeto x86.r16)
	jyes	invalid_address
      mode_ok:

	check	address_registers eq 0
	jyes	direct_address
	check	1 metadataof address_registers relativeto x86.r32
	jyes	address_32bit
	check	1 metadataof address_registers relativeto x86.r16
	jyes	address_16bit
	jump	invalid_address

    direct_address:
	compute mod, 0
	check	mode = 16
	jyes	direct_address_16bit
      direct_address_32bit:
	compute rm, 5
	compute displacement_size, 4
	jump	export_address
      direct_address_16bit:
	compute rm, 6
	compute displacement_size, 2
	jump	export_address

    address_16bit:
	compute mode, 16

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
	jump	export_address
      displacement_empty:
	compute displacement_size, 0
	compute mod, 0
	jump	export_address
      displacement_8bit_wrap_16bit:
	compute displacement, displacement-10000h
      displacement_8bit:
	compute displacement_size, 1
	compute mod, 1
	jump	export_address

    address_32bit:
	compute mode,32
	check	2 scaleof address_registers = 0
	jyes	one_register
	check	3 scaleof address_registers = 0 & 2 metadataof address_registers relativeto x86.r32
	jyes	two_registers
	jump	invalid_address

      one_register:
	compute scale, 1 scaleof address_registers
	compute base, 1 metadataof address_registers - x86.r32
	check	scale = 1
	jyes	one_register_unscaled
	check	base <> 4 & (scale = 4 | scale = 8)
	jyes	one_register_scaled
	check	base <> 4 & (scale = 2 | scale = 3 | scale = 5 | scale = 9)
	jyes	one_register_split
	jump	invalid_address
      one_register_unscaled:
	compute rm, base
	check	rm = 4
	jno	setup_displacement
	compute index, 4
	jump	setup_displacement
      one_register_scaled:
	compute rm, 4
	compute index, base
	compute base, 5
	jump	index_only
      one_register_split:
	compute rm, 4
	compute index, base
	compute scale, scale - 1
	jump	setup_displacement
      two_registers:
	compute rm,4
	check	1 scaleof address_registers = 1
	jyes	base_first
	check	2 scaleof address_registers = 1
	jyes	base_second
	jump	invalid_address
      base_first:
	compute base, 1 metadataof address_registers - x86.r32
	compute index, 2 metadataof address_registers - x86.r32
	compute scale, 2 scaleof address_registers
	jump	process_sib
      base_second:
	compute base, 2 metadataof address_registers - x86.r32
	compute index, 1 metadataof address_registers - x86.r32
	compute scale, 1 scaleof address_registers
      process_sib:
	check	index = 4
	jyes	forbidden_index
	check	segment_prefix = 36h & index = 5 & scale = 1
	jyes	switch_to_index
	check	segment_prefix = 3Eh & base = 5 & scale = 1
	jyes	switch_to_base
	check	scale > 2 & scale <> 4 & scale <> 8
	jyes	invalid_address
	jump	setup_displacement
      forbidden_index:
	check	scale = 1
	jno	invalid_address
	compute index, base
	compute base, 4
	jump	setup_displacement
      switch_to_index:
	compute index, base
	compute base, 5
	jump	setup_displacement
      switch_to_base:
	compute base, index
	compute index, 5
	jump	setup_displacement

      setup_displacement:
	check	displacement relativeto 0
	jno	displacement_32bit
	check	displacement = 0 & rm <> 5 & (rm <> 4 | base <> 5)
	jyes	displacement_empty
	check	displacement < 80h & displacement >= -80h
	jyes	displacement_8bit
	check	displacement-100000000h >= -80h & displacement < 100000000h
	jyes	displacement_8bit_wrap_32bit
	check	segment_prefix = 3Eh & base = 5 & index = 5 & scale = 1
	jno	displacement_32bit
	compute scale, 2
	jump	index_only
      displacement_32bit:
	compute displacement_size, 4
	compute mod, 2
	jump	export_address
      displacement_8bit_wrap_32bit:
	compute displacement, displacement-100000000h
	jump	displacement_8bit
      index_only:
	compute displacement_size, 4
	compute mod, 0
	jump	export_address

  far_operand:
	compute type, 'far'

	check	size & size <> 4 & size <> 6
	jyes	operand_sizes_do_not_match

	arrange sym, namespace.=segment
	publish sym, segment

	arrange sym, namespace.=offset
	publish sym, offset

	jump	export_common

  export_immediate:

	arrange sym, namespace.=imm
	publish sym, imm

	arrange sym, namespace.=unresolved
	publish sym, unresolved

	jump	export_common

  export_address:

	arrange sym, namespace.=address
	publish sym, address

	arrange sym, namespace.=address_registers
	publish sym, address_registers

	arrange sym, namespace.=scale
	publish sym, scale

	arrange sym, namespace.=index
	publish sym, index

	arrange sym, namespace.=base
	publish sym, base

	arrange sym, namespace.=displacement
	publish sym, displacement

  export_register:

	arrange sym, namespace.=mode
	publish sym, mode

	arrange sym, namespace.=mod
	publish sym, mod

	arrange sym, namespace.=rm
	publish sym, rm

  export_common:

	arrange sym, namespace.=type
	publish sym, type

	arrange sym, namespace.=size
	publish sym, size

	arrange sym, namespace.=displacement_size
	publish sym, displacement_size

	arrange sym, namespace.=segment_prefix
	publish sym, segment_prefix

	arrange sym, namespace.=prefix
	publish sym, prefix

	arrange sym, namespace.=opcode_prefix
	publish sym, opcode_prefix

	exit

  invalid_operand:
	asmcmd	=err 'invalid operand'
	exit
  invalid_operand_size:
	asmcmd	=err 'invalid operand size'
	exit
  operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	exit
  invalid_address:
	asmcmd	=err 'invalid address'
	exit
  invalid_address_size:
	asmcmd	=err 'invalid address size'
	exit

end calminstruction

calminstruction x86.parse_jump_operand namespace, operand

	local	jump_type
	local	sym

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

	arrange operand, =x86.=parse_operand namespace, operand
	assemble operand

	arrange sym, namespace.=jump_type
	publish sym, jump_type

	local	type, size

	arrange type, namespace.=type
	check	type = 'imm'
	jno	done

	arrange size, namespace.=size
	check	size = 0
	jno	verify_target_address

	compute size, x86.mode shr 3
	arrange sym, namespace.=size
	publish sym, size

    verify_target_address:

	local	imm

	arrange sym, namespace.=imm
	compute imm, sym

	check	imm relativeto 0
	jno	done
	check	imm < 0
	jyes	negative
	check	imm >= 1 shl (size*8)
	jno	done
    out_of_range:
	asmcmd	=err 'value out of range'
	exit
    negative:
	check	imm < - 1 shl (size*8-1)
	jyes	out_of_range
	compute imm, imm and (1 shl (size*8) - 1)
	publish sym, imm

    done:

end calminstruction

calminstruction x86.select_operand_prefix rm_operand*,size*

	local	sym, prefix

	check	(size = 2 & x86.mode = 32) | (size = 4 & x86.mode = 16)
	jno	no_prefix

	compute prefix, 66h
	arrange sym, rm_operand.=prefix
	publish sym, prefix

    no_prefix:

	check	size <> 0 & size <> 2 & size <> 4
	jno	done

	asmcmd	=err 'invalid operand size'

    done:

end calminstruction

calminstruction x86.store_operand_prefix size*

	check	(size = 2 & x86.mode = 32) | (size = 4 & x86.mode = 16)
	jno	no_prefix

	asmcmd	=db 66h
	exit

    no_prefix:

	check	size <> 0 & size <> 2 & size <> 4
	jno	done

	asmcmd	=err 'invalid operand size'

    done:

end calminstruction

calminstruction x86.store_instruction opcode*,rm_operand*,reg*,imm_size:0,imm

	local	segment_prefix, prefix, opcode_prefix
	local	mode, mod, rm
	local	scale, index, base
	local	displacement, displacement_size

	arrange segment_prefix, rm_operand.=segment_prefix
	arrange prefix, rm_operand.=prefix
	arrange opcode_prefix, rm_operand.=opcode_prefix

	arrange mode, rm_operand.=mode
	arrange mod, rm_operand.=mod
	arrange rm, rm_operand.=rm
	arrange base, rm_operand.=base

	arrange displacement_size, rm_operand.=displacement_size
	arrange displacement, rm_operand.=displacement

	check	segment_prefix
	jno	segment_prefix_ok

	check	mode = 16 & ( rm = 2 | rm = 3 | ( mod > 0 & rm = 6 ) )
	jyes	ss_segment_default
	check	mode = 32 & ( ( mod > 0 & rm = 5 ) | ( rm = 4 & base = 4 ) | ( mod > 0 & rm = 4 & base = 5 ) )
	jyes	ss_segment_default

    ds_segment_default:
	check	segment_prefix = 3Eh
	jyes	segment_prefix_ok
	jump	store_segment_prefix
    ss_segment_default:
	check	segment_prefix = 36h
	jyes	segment_prefix_ok
    store_segment_prefix:
	asmcmd	=db segment_prefix
    segment_prefix_ok:

	check	mod <> 11b & mode <> x86.mode
	jno	addressing_prefix_ok
	asmcmd	=db 67h
    addressing_prefix_ok:

	check	prefix
	jno	operand_prefix_ok
	asmcmd	=db prefix
    operand_prefix_ok:

	check	opcode_prefix
	jno	opcode_prefix_ok
	asmcmd	=db opcode_prefix
    opcode_prefix_ok:

	local	modrm, sib

	compute modrm, mod shl 6 + (reg and 111b) shl 3 + rm and 111b
	asmcmd	=db opcode, modrm

	check	mod <> 11b & rm = 4 & mode = 32
	jno	sib_ok
	arrange scale, rm_operand.=scale
	arrange index, rm_operand.=index
	compute sib, (bsf scale) shl 6 + (index and 111b) shl 3 + base and 111b
	asmcmd	=db sib
    sib_ok:

	check	displacement_size = 1
	jyes	displacement_8bit
	check	displacement_size = 2
	jyes	displacement_16bit
	check	displacement_size = 4
	jno	displacement_ok
    displacement_32bit:
	asmcmd	=dd rm_operand.=displacement
	jump	displacement_ok
    displacement_16bit:
	asmcmd	=dw rm_operand.=displacement
	jump	displacement_ok
    displacement_8bit:
	asmcmd	=db rm_operand.=displacement
    displacement_ok:

	check	imm_size = 1
	jyes	immediate_8bit
	check	imm_size = 2
	jyes	immediate_16bit
	check	imm_size = 4
	jno	immediate_ok
    immediate_32bit:
	asmcmd	=dd imm
	jump	immediate_ok
    immediate_16bit:
	asmcmd	=dw imm
	jump	immediate_ok
    immediate_8bit:
	asmcmd	=db imm
    immediate_ok:

end calminstruction


iterate <instr,basecode>, add,0, or,8, adc,10h, sbb,18h, and,20h, sub,28h, xor,30h, cmp,38h

	calminstruction instr? dest*,src*

		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src

		local	opcode, rm, size

		compute opcode, basecode

		check	@dest.size = 0 & @src.size = 0
		jyes	operand_size_not_specified
		check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		jyes	operand_sizes_do_not_match

		compute size, @dest.size or @src.size

	    main:

		check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	reg_rm
		check	@src.type = 'mem' & @dest.type = 'reg'
		jyes	rm_reg
		check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	rm_imm

		asmcmd	=err 'invalid combination of operands'
		exit

	    operand_size_not_specified:
		asmcmd	=err 'operand size not specified'
		compute size, 0
		jump	main

	    operand_sizes_do_not_match:
		asmcmd	=err 'operand sizes do not match'
		compute size, 0
		jump	main

	    reg_rm:
		check	size > 1
		jno	reg_rm_store
		asmcmd	=x86.=select_operand_prefix =@dest,size
		compute opcode, opcode + 1
	    reg_rm_store:
		asmcmd	=x86.=store_instruction opcode,=@dest,=@src.=rm
		exit

	    rm_reg:
		compute opcode, opcode + 2
		check	size > 1
		jno	rm_reg_store
		asmcmd	=x86.=select_operand_prefix =@src,size
		compute opcode, opcode + 1
	    rm_reg_store:
		asmcmd	=x86.=store_instruction opcode,=@src,=@dest.=rm
		exit

	    rm_imm:
		check	size > 1
		jyes	rm_imm_word
		check	@dest.type = 'reg' & @dest.rm = 0
		jyes	al_imm

		compute opcode, opcode shr 3
		asmcmd	=x86.=store_instruction 80h,=@dest,opcode,1,=@src.=imm
		exit

	    al_imm:
		compute opcode, opcode+4
		asmcmd	=db opcode, =@src.=imm
		exit

	    rm_imm_word:

		asmcmd	=x86.=select_operand_prefix =@dest,size

		check	@src.imm eqtype 0.0
		jno	rm_imm_optimize

		asmcmd	=virtual =at 0
		asmcmd	=emit size:=@src.=imm
		asmcmd	=load =@src.=imm:size =from 0
		asmcmd	=end =virtual
		compute @src.imm, +@src.imm

	    rm_imm_optimize:
		check	@src.imm relativeto 0 & @src.imm<80h & @src.imm>=-80h
		jyes	rm_simm
		check	size = 2 & @src.imm relativeto 0 & @src.imm-10000h>=-80h & @src.imm<10000h
		jyes	rm_simm_wrap
		check	size = 4 & @src.imm relativeto 0 & @src.imm-100000000h>=-80h & @src.imm<100000000h
		jyes	rm_simm_wrap
		check	@dest.type = 'reg' & @dest.rm = 0
		jyes	ax_imm

		compute rm, opcode shr 3
		asmcmd	=x86.=store_instruction 81h,=@dest,rm,size,=@src.=imm
		exit

	    ax_imm:
		check	@dest.prefix
		jno	ax_imm_prefix_ok
		asmcmd	=db =@dest.=prefix
	      ax_imm_prefix_ok:
		compute opcode, opcode+5
		asmcmd	=db opcode
		check	size = 4
		jyes	imm32
		asmcmd	=dw =@src.=imm
		exit
	      imm32:
		asmcmd	=dd =@src.=imm
		exit

	    rm_simm_wrap:
		compute @src.imm, @src.imm - 1 shl (size shl 3)

	    rm_simm:
		compute rm, opcode shr 3
		asmcmd	=x86.=store_instruction 83h,=@dest,rm,1,=@src.=imm

	end calminstruction

end iterate

iterate <instr,postbyte>, not,2, neg,3, mul,4, div,6, idiv,7

	calminstruction instr? src*

		asmcmd	=x86.=parse_operand =@src,src

		check	@src.size = 0
		jyes	operand_size_not_specified

	    main:
		check	@src.type = 'mem' | @src.type = 'reg'
		jno	invalid_operand
		check	@src.size > 1
		jyes	rm_word

		asmcmd	=x86.=store_instruction 0F6h,=@src,postbyte
		exit

	    rm_word:
		asmcmd	=x86.=select_operand_prefix =@src,=@src.=size
		asmcmd	=x86.=store_instruction 0F7h,=@src,postbyte
		exit

	    operand_size_not_specified:
		asmcmd	=err 'operand size not specified'
		jump	main

	    invalid_operand:
		asmcmd	=err 'invalid operand'
		exit

	end calminstruction

end iterate

calminstruction mov? dest*,src*

	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0 & @dest.type <> 'sreg'
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

    main:

	check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_reg
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	mov_reg_mem
	check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_imm
	check	@src.type = 'reg' & @dest.type = 'imm'
	jyes	mov_creg_reg
	check	@src.type = 'sreg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_sreg
	check	@dest.type = 'sreg' & @dest.rm <> 1 & ( @src.type = 'reg' | @src.type = 'mem' )
	jyes	mov_sreg_rm

    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit

    operand_size_not_specified:
	asmcmd	=err 'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	compute size, 0
	jump	main

    mov_rm_reg:
	check	@src.type = 'reg' & @dest.type = 'mem' & @src.rm = 0 & @dest.address_registers eq 0
	jyes	mov_dirmem_ax
	check	size > 1
	jno	mov_reg_rm_8bit
	asmcmd	=x86.=select_operand_prefix =@dest,size
	asmcmd	=x86.=store_instruction 89h,=@dest,=@src.=rm
	exit
      mov_reg_rm_8bit:
	asmcmd	=x86.=store_instruction 88h,=@dest,=@src.=rm
	exit

    mov_reg_mem:
	check	@src.type = 'mem' & @dest.type = 'reg' & @dest.rm = 0 & @src.address_registers eq 0
	jyes	mov_ax_dirmem
	check	size > 1
	jno	mov_mem_reg_8bit
	asmcmd	=x86.=select_operand_prefix =@src,size
	asmcmd	=x86.=store_instruction 8Bh,=@src,=@dest.=rm
	exit
      mov_mem_reg_8bit:
	asmcmd	=x86.=store_instruction 8Ah,=@src,=@dest.=rm
	exit

    mov_dirmem_ax:
	check	@dest.segment_prefix = 0 | @dest.segment_prefix = 3Eh
	jyes	dest_seg_ok
	asmcmd	=db =@dest.=segment_prefix
      dest_seg_ok:
	check	@dest.mode = x86.mode
	jyes	dest_addr_size_ok
	asmcmd	=db 67h
      dest_addr_size_ok:
	check	size > 1
	jno	mov_dirmem_al
	asmcmd	=x86.=store_operand_prefix size
	asmcmd	=db 0A3h
	jump	dest_displacement
      mov_dirmem_al:
	asmcmd	=db 0A2h
      dest_displacement:
	check	@dest.mode = 16
	jyes	dest_displacement_16bit
	asmcmd	=dd =@dest.=address
	exit
      dest_displacement_16bit:
	asmcmd	=dw =@dest.=address
	exit

    mov_ax_dirmem:
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	src_seg_ok
	asmcmd	=db =@src.=segment_prefix
      src_seg_ok:
	check	@src.mode = x86.mode
	jyes	src_addr_size_ok
	asmcmd	=db 67h
      src_addr_size_ok:
	check	size > 1
	jno	mov_al_dirmem
	asmcmd	=x86.=store_operand_prefix size
	asmcmd	=db 0A1h
	jump	src_displacement
      mov_al_dirmem:
	asmcmd	=db 0A0h
      src_displacement:
	check	@src.mode = 16
	jyes	src_displacement_16bit
	asmcmd	=dd =@src.=address
	exit
      src_displacement_16bit:
	asmcmd	=dw =@src.=address
	exit

    mov_rm_imm:
	check	@dest.type = 'mem'
	jyes	mov_mem_imm
	check	@dest.type = 'reg' & 1 metadataof (1 metadataof @src.imm) relativeto x86.creg & @src.imm relativeto 1 elementof @src.imm
	jyes	mov_reg_creg

    mov_reg_imm:
	check	size > 1
	jno	mov_reg_imm_8bit
	asmcmd	=x86.=store_operand_prefix size
	asmcmd	=db 0B8h + =@dest.=rm
	check	size = 2
	jyes	src_imm_16bit
	asmcmd	=dd =@src.=imm
	exit
      src_imm_16bit:
	asmcmd	=dw =@src.=imm
	exit
      mov_reg_imm_8bit:
	asmcmd	=db 0B0h + =@dest.=rm
	asmcmd	=db =@src.=imm
	exit

    mov_mem_imm:
	check	size > 1
	jno	mov_mem_imm_8bit
	asmcmd	=x86.=select_operand_prefix =@dest,size
	asmcmd	=x86.=store_instruction 0C7h,=@dest,0,size,=@src.=imm
	exit
      mov_mem_imm_8bit:
	asmcmd	=x86.=store_instruction 0C6h,=@dest,0,1,=@src.=imm
	exit

    mov_reg_creg:
	check	@dest.size = 4
	jno	invalid_operand_size
	compute ext, 20h + 1 metadataof (1 metadataof @src.imm) - x86.creg
	compute rm, 1 metadataof @src.imm - 1 elementof (1 metadataof @src.imm)
	asmcmd	=x86.=store_instruction <0Fh,ext>,=@dest,rm
	exit

    mov_creg_reg:
	check	1 metadataof (1 metadataof @dest.imm) relativeto x86.creg & @dest.imm relativeto 1 elementof @dest.imm
	jno	invalid_combination_of_operands
	check	@src.size = 4
	jno	invalid_operand_size
	compute ext, 22h + 1 metadataof (1 metadataof @dest.imm) - x86.creg
	compute rm, 1 metadataof @dest.imm - 1 elementof (1 metadataof @dest.imm)
	asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,rm
	exit

    mov_rm_sreg:
	check	@dest.type = 'mem'
	jyes	mov_mem_sreg
      mov_reg_sreg:
	check	size > 1
	jno	invalid_operand_size
	asmcmd	=x86.=select_operand_prefix =@dest,size
	jump	mov_rm_sreg_store
      mov_mem_sreg:
	check	size and not 2
	jyes	invalid_operand_size
      mov_rm_sreg_store:
	asmcmd	=x86.=store_instruction 8Ch,=@dest,=@src.=rm
	exit

    mov_sreg_rm:
	check	size = 1
	jyes	invalid_operand_size
	asmcmd	=x86.=store_instruction 8Eh,=@src,=@dest.=rm
	exit

    invalid_operand_size:
	asmcmd	=err 'invalid operand size'
	exit

end calminstruction

calminstruction test? dest*,src*

	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

    main:

	check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	test_reg_rm
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	test_mem_reg
	check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	test_rm_imm

    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit

    operand_size_not_specified:
	asmcmd	=err 'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	compute size, 0
	jump	main

    test_reg_rm:
	check	size > 1
	jno	test_reg_rm_8bit
	asmcmd	=x86.=select_operand_prefix =@dest,size
	asmcmd	=x86.=store_instruction 85h,=@dest,=@src.=rm
	exit
      test_reg_rm_8bit:
	asmcmd	=x86.=store_instruction 84h,=@dest,=@src.=rm
	exit

    test_mem_reg:
	check	size > 1
	jno	test_mem_reg_8bit
	asmcmd	=x86.=select_operand_prefix =@src,size
	asmcmd	=x86.=store_instruction 85h,=@src,=@dest.=rm
	exit
      test_mem_reg_8bit:
	asmcmd	=x86.=store_instruction 84h,=@src,=@dest.=rm
	exit

    test_rm_imm:
	check	size > 1
	jno	test_rm_imm_8bit
	check	@dest.type = 'reg' & @dest.rm = 0
	jyes	test_ax_imm
	asmcmd	=x86.=select_operand_prefix =@dest,size
	asmcmd	=x86.=store_instruction 0F7h,=@dest,0,size,=@src.=imm
	exit

      test_ax_imm:
	asmcmd	=x86.=store_operand_prefix size
	asmcmd	=db 0A9h
	check	size = 2
	jyes	src_imm_16bit
	asmcmd	=dd =@src.=imm
	exit
      src_imm_16bit:
	asmcmd	=dw =@src.=imm
	exit

      test_rm_imm_8bit:
	check	@dest.type = 'reg' & @dest.rm = 0
	jyes	test_al_imm
	asmcmd	=x86.=store_instruction 0F6h,=@dest,0,1,=@src.=imm
	exit
      test_al_imm:
	asmcmd	=db 0A8h, =@src.=imm
	exit

end calminstruction

calminstruction xchg? dest*,src*

	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

    main:

	check	@src.type = 'reg' & @dest.type = 'reg'
	jyes	xchg_reg_reg
	check	@src.type = 'reg' & @dest.type = 'mem'
	jyes	xchg_reg_rm
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	xchg_rm_reg

    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit

    operand_size_not_specified:
	asmcmd	=err 'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	compute size, 0
	jump	main

    xchg_reg_reg:
	check	@src.rm = 0 | @dest.rm = 0
	jno	xchg_rm_reg
	check	size > 1
	jno	xchg_rm_reg_8bit
	asmcmd	=x86.=store_operand_prefix size
	local	opcode
	compute opcode, 90h + @src.rm + @dest.rm
	asmcmd	=db opcode
	exit

    xchg_reg_rm:
	check	size > 1
	jno	xchg_reg_rm_8bit
	asmcmd	=x86.=select_operand_prefix =@dest,size
	asmcmd	=x86.=store_instruction 87h,=@dest,=@src.=rm
	exit
      xchg_reg_rm_8bit:
	asmcmd	=x86.=store_instruction 86h,=@dest,=@src.=rm
	exit

    xchg_rm_reg:
	check	size > 1
	jno	xchg_rm_reg_8bit
	asmcmd	=x86.=select_operand_prefix =@src,size
	asmcmd	=x86.=store_instruction 87h,=@src,=@dest.=rm
	exit
      xchg_rm_reg_8bit:
	asmcmd	=x86.=store_instruction 86h,=@src,=@dest.=rm
	exit

end calminstruction

iterate <instr,postbyte>, inc,0 ,dec,1

	calminstruction instr? dest*

		asmcmd	=x86.=parse_operand =@dest,dest

		check	@dest.size
		jyes	main

		asmcmd	=err 'operand size not specified'

	    main:
		check	@dest.type = 'reg'
		jyes	inc_reg
		check	@dest.type = 'mem'
		jyes	inc_rm

		asmcmd	=err 'invalid operand'
		exit

	    inc_reg:
		check	@dest.size > 1
		jno	inc_rm_8bit
		asmcmd	=x86.=store_operand_prefix =@dest.=size
		local	opcode
		compute opcode, 40h + @dest.rm + postbyte shl 3
		asmcmd	=db opcode
		exit

	    inc_rm:
		check	@dest.size > 1
		jno	inc_rm_8bit
		asmcmd	=x86.=select_operand_prefix =@dest,=@dest.=size
		asmcmd	=x86.=store_instruction 0FFh,=@dest,postbyte
		exit
	      inc_rm_8bit:
		asmcmd	=x86.=store_instruction 0FEh,=@dest,postbyte

	end calminstruction

end iterate

calminstruction imul? dest*,src&

	local	size

	asmcmd	=x86.=parse_operand =@dest,dest

	match	, src
	jyes	imul_rm

	local	src1, src2

	match	src1 =, src2, src
	jyes	imul_second_source

	asmcmd	=x86.=parse_operand =@src,src

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

	check	@dest.type = 'reg' & (@src.type = 'reg' | @src.type = 'mem')
	jyes	imul_reg_rm

	check	@src.type = 'imm' & @dest.type = 'reg'
	jno	invalid_combination_of_operands

	compute @aux.type, @src.type
	compute @aux.imm, @src.imm
	compute @src.type, @dest.type
	compute @src.mod, @dest.mod
	compute @src.rm, @dest.rm

	jump	main

    imul_second_source:
	asmcmd	=x86.=parse_operand =@src,src1
	asmcmd	=x86.=parse_operand =@aux,src2

	check	@dest.size = 0 & @src.size = 0 & @aux.size = 0
	jyes	operand_size_not_specified

	compute size, @dest.size or @src.size or @aux.size

	check	(@dest.size & @dest.size <> size) | (@src.size & @src.size <> size) | (@aux.size & @aux.size <> size)
	jyes	operand_sizes_do_not_match

	jump	main

    operand_size_not_specified:
	asmcmd	=err 'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	compute size, 0
	jump	main

    main:
	check	@aux.type = 'imm' & ( @src.type = 'mem' | @src.type = 'reg' ) & @dest.type = 'reg'
	jyes	imul_reg_rm_imm

    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit

    imul_rm:
	check	@dest.size
	jyes	imul_rm_size_ok
	asmcmd	=err 'operand size not specified'
      imul_rm_size_ok:
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jno	invalid_combination_of_operands
	check	@dest.size > 1
	jno	imul_rm_8bit
	asmcmd	=x86.=select_operand_prefix =@dest,=@dest.=size
	asmcmd	=x86.=store_instruction 0F7h,=@dest,5
	exit
      imul_rm_8bit:
	asmcmd	=x86.=store_instruction 0F6h,=@dest,5
	exit

    imul_reg_rm:
	asmcmd	=x86.=select_operand_prefix =@src,size
	asmcmd	=x86.=store_instruction <0Fh,0AFh>,=@src,=@dest.=rm
	exit

    imul_reg_rm_imm:
	asmcmd	=x86.=select_operand_prefix =@src,size
	check	@aux.imm eqtype 0.0
	jno	imul_reg_rm_imm_optimize
	asmcmd	=virtual =at 0
	asmcmd	=emit size:=@aux.=imm
	asmcmd	=load =@aux.=imm:size =from 0
	asmcmd	=end =virtual
	compute @aux.imm, +@aux.imm
      imul_reg_rm_imm_optimize:
	check	@aux.imm relativeto 0 & @aux.imm < 80h & @aux.imm >= -80h
	jyes	imul_reg_rm_simm
	check	size = 2 & @aux.imm relativeto 0 & @aux.imm - 10000h >= -80h & @aux.imm < 10000h
	jyes	imul_reg_rm_simm_wrap
	check	size = 4 & @aux.imm relativeto 0 & @aux.imm - 100000000h >= -80h & @aux.imm < 100000000h
	jyes	imul_reg_rm_simm_wrap
	asmcmd	=x86.=store_instruction 69h,=@src,=@dest.=rm,size,=@aux.=imm
	exit
      imul_reg_rm_simm_wrap:
	compute @aux.imm, @aux.imm - 1 shl (size shl 3)
      imul_reg_rm_simm:
	asmcmd	=x86.=store_instruction 6Bh,=@src,=@dest.=rm,1,=@aux.=imm
	exit

end calminstruction

calminstruction x86.push_instruction size:0,src*

	local	opcode

	asmcmd	=x86.=parse_operand =@src,src

	check	size <> 0 & @src.size and not size
	jyes	invalid_operand_size
	compute size, size or @src.size
	check	size = 0 | size = 2 | size = 4
	jyes	main

    invalid_operand_size:
	asmcmd	=err 'invalid operand size'

    main:
	check	@src.type = 'mem'
	jyes	push_mem
	check	@src.type = 'reg'
	jyes	push_reg
	check	@src.type = 'sreg'
	jyes	push_sreg
	check	@src.type = 'imm'
	jyes	push_imm

	asmcmd	=err 'invalid operand'
	exit

    push_mem:
	asmcmd	=x86.=select_operand_prefix =@src,size
	asmcmd	=x86.=store_instruction 0FFh,=@src,110b
	exit

    push_reg:
	asmcmd	=x86.=store_operand_prefix size
	compute opcode, 50h + @src.rm
	asmcmd	=db opcode
	exit

    push_sreg:
	asmcmd	=x86.=store_operand_prefix size
	check	@src.rm >= 4
	jyes	push_sreg_386
	compute opcode, 6 + @src.rm shl 3
	asmcmd	=db opcode
	exit
      push_sreg_386:
	compute opcode, 0A0h + (@src.rm-4) shl 3
	asmcmd	=db 0Fh,opcode
	exit

    push_imm:
	check	size
	jyes	push_imm_size_ok
	check	x86.mode = 16
	jyes	push_imm_16bit
	compute size, 4
	jump	push_imm_size_ok
      push_imm_16bit:
	compute size, 2
      push_imm_size_ok:

	asmcmd	=x86.=store_operand_prefix size

	check	@src.imm eqtype 0.0
	jno	push_imm_optimize
	asmcmd	=virtual =at 0
	asmcmd	=emit size:=@src.=imm
	asmcmd	=load =@src.=imm:size =from 0
	asmcmd	=end =virtual
	compute @src.imm, +@src.imm
      push_imm_optimize:
	check	@src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
	jyes	push_simm
	check	size = 2 & @src.imm relativeto 0 & @src.imm - 10000h >= -80h & @src.imm < 10000h
	jyes	push_simm_wrap
	check	size = 4 & @src.imm relativeto 0 & @src.imm - 100000000h >= -80h & @src.imm < 100000000h
	jyes	push_simm_wrap
	asmcmd	=db 68h
	check	size = 2
	jyes	src_imm_16bit
	asmcmd	=dd =@src.=imm
	exit
      src_imm_16bit:
	asmcmd	=dw =@src.=imm
	exit
      push_simm_wrap:
	compute @src.imm, @src.imm - 1 shl (size shl 3)
      push_simm:
	asmcmd	=db 6Ah, =@src.=imm
	exit

end calminstruction

calminstruction x86.pop_instruction size:0,dest*

	local	opcode

	asmcmd	=x86.=parse_operand =@dest,dest

	check	size <> 0 & @dest.size and not size
	jyes	invalid_operand_size
	compute size, size or @dest.size
	check	size = 0 | size = 2 | size = 4
	jyes	main

    invalid_operand_size:
	asmcmd	=err 'invalid operand size'

    main:
	check	@dest.type = 'mem'
	jyes	pop_mem
	check	@dest.type = 'reg'
	jyes	pop_reg
	check	@dest.type = 'sreg'
	jyes	pop_sreg

    invalid_operand:
	asmcmd	=err 'invalid operand'
	exit

    pop_mem:
	asmcmd	=x86.=select_operand_prefix =@dest,size
	asmcmd	=x86.=store_instruction 8Fh,=@dest,0
	exit

    pop_reg:
	asmcmd	=x86.=store_operand_prefix size
	compute opcode, 58h + @dest.rm
	asmcmd	=db opcode
	exit

    pop_sreg:
	check	@dest.rm = 1
	jyes	invalid_operand
	asmcmd	=x86.=store_operand_prefix size
	check	@dest.rm >= 4
	jyes	pop_sreg_386
	compute opcode, 7 + @dest.rm shl 3
	asmcmd	=db opcode
	exit
      pop_sreg_386:
	compute opcode, 0A1h + (@dest.rm-4) shl 3
	asmcmd	=db 0Fh,opcode
	exit

end calminstruction

iterate reg, ax,cx,dx,bx,sp,bp,si,di, eax,ecx,edx,ebx,esp,ebp,esi,edi, es,cs,ss,ds,fs,gs
	define x86.compact.reg? {reg}
end iterate

iterate <instr,handler,size>, push,push_instruction,0, pushw,push_instruction,2, pushd,push_instruction,4, \
			      pop,pop_instruction,0, popw,pop_instruction,2, popd,pop_instruction,4

	calminstruction instr? operand

		local	head, tail

		match	head tail, operand
		jno	plain
		transform head, x86.compact
		jno	plain
		match	{head}, head
		jno	plain
	    loop:
		asmcmd	=x86.=handler size,head
		match	head tail, tail
		jno	final
		transform head, x86.compact
		jno	error
		match	{head}, head
		jyes	loop
	    error:
		asmcmd	=err 'only register operands allowed in compact syntax'
		exit
	    final:
		transform tail, x86.compact
		jno	error
		match	{operand}, tail
		jno	error
	    plain:
		asmcmd	=x86.=handler size,operand

	end calminstruction

end iterate

iterate <instr,opcode>, ret,0C2h, retn,0C2h, retf,0CAh

	calminstruction instr? operand
		match	, operand
		jyes	ret_short
		check	operand
		jno	ret_short
		asmcmd	=db opcode
		asmcmd	=dw operand
		exit
	    ret_short:
		asmcmd	=db opcode + 1
	end calminstruction

end iterate

iterate <instr,size,opcode>, retw,2,0C2h, retnw,2,0C2h, retd,4,0C2h, retnd,4,0C2h, retfw,2,0CAh, retfd,4,0CAh

	calminstruction instr? operand
		asmcmd	=x86.=store_operand_prefix size
		match	, operand
		jyes	ret_short
		asmcmd	=db opcode
		asmcmd	=dw operand
		exit
	    ret_short:
		asmcmd	=db opcode + 1
	end calminstruction

end iterate

calminstruction lea? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.type = 'mem' & @dest.type = 'reg'
	jno	invalid_combination_of_operands
	asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
	asmcmd	=x86.=store_instruction 8Dh,=@src,=@dest.=rm
	exit
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
end calminstruction

iterate <instr,opcode>, les,0C4h, lds,0C5h, lss,<0Fh,0B2h>, lfs,<0Fh,0B4h>, lgs,<0Fh,0B5h>

	calminstruction instr? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	(@dest.size = 2 & (@src.size <> 0 & @src.size <> 4)) | (@dest.size = 4 & (@src.size <> 0 & @src.size <> 6))
		jyes	invalid_operand_size
		check	@src.type = 'mem' & @dest.type = 'reg'
		jno	invalid_combination_of_operands
		asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
		asmcmd	=x86.=store_instruction opcode,=@src,=@dest.=rm
		exit
	    invalid_operand_size:
		asmcmd	=err 'invalid operand size'
		exit
	    invalid_combination_of_operands:
		asmcmd	=err 'invalid combination of operands'
	end calminstruction

end iterate

iterate <instr,postbyte>, rol,0, ror,1, rcl,2, rcr,3, shl,4, sal, 4, shr,5, sar,7

	calminstruction instr? dest*,cnt*

		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,cnt

		check	@dest.size = 0
		jyes	operand_size_not_specified
		check	@src.size and not 1
		jyes	invalid_operand_size

	    main:
		check	@src.type = 'reg' & @src.size = 1 & @src.rm = 1 & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shift_rm_cl
		check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shift_rm_imm

		asmcmd	=err 'invalid combination of operands'
		exit

	    shift_rm_cl:
		check	@dest.size > 1
		jno	shift_r8_cl
		asmcmd	=x86.=select_operand_prefix =@dest,=@dest.=size
		asmcmd	=x86.=store_instruction 0D3h,=@dest,postbyte
		exit
	      shift_r8_cl:
		asmcmd	=x86.=store_instruction 0D2h,=@dest,postbyte
		exit
	    shift_rm_imm:
		check	@dest.size > 1
		jno	shift_rm8_imm
		asmcmd	=x86.=select_operand_prefix =@dest,=@dest.=size
		check	@src.imm = 1
		jyes	shift_rm_1
		asmcmd	=x86.=store_instruction 0C1h,=@dest,postbyte,1,=@src.=imm
		exit
	      shift_rm_1:
		asmcmd	=x86.=store_instruction 0D1h,=@dest,postbyte
		exit
	      shift_rm8_imm:
		check	@src.imm = 1
		jyes	shift_rm8_1
		asmcmd	=x86.=store_instruction 0C0h,=@dest,postbyte,1,=@src.=imm
		exit
	      shift_rm8_1:
		asmcmd	=x86.=store_instruction 0D0h,=@dest,postbyte
		exit

	    operand_size_not_specified:
		asmcmd	=err 'operand size not specified'
		jump	main
	    invalid_operand_size:
		asmcmd	=err 'invalid operand size'
		jump	main

	end calminstruction

end iterate

iterate <instr,ext>, shld,0A4h, shrd,0ACh

	calminstruction instr? dest*,src*,cnt*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		asmcmd	=x86.=parse_operand =@aux,cnt
		check	@aux.size and not 1
		jyes	invalid_operand_size
		check	@dest.size and not @src.size
		jno	main
		asmcmd	=err 'operand sizes do not match'
	    main:
		check	@aux.type = 'reg' & @aux.size = 1 & @aux.rm = 1 & @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shld_cl
		check	@aux.type = 'imm' & @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shld_imm
		asmcmd	=err 'invalid combination of operands'
		exit
	    shld_cl:
		asmcmd	=x86.=select_operand_prefix =@dest,=@src.=size
		asmcmd	=x86.=store_instruction <0Fh,ext+1>,=@dest,=@src.=rm
		exit
	    shld_imm:
		asmcmd	=x86.=select_operand_prefix =@dest,=@src.=size
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@dest,=@src.=rm,1,=@aux.=imm
		exit
	    invalid_operand_size:
		asmcmd	=err 'invalid operand size'
	end calminstruction

end iterate

iterate <instr,postbyte>, bt,4, bts,5, btr,6, btc,7

	calminstruction instr? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	@src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
		jyes	bt_rm_reg
		check	@src.type = 'imm' & (@dest.type = 'mem' | @dest.type = 'reg')
		jyes	bt_rm_imm
		asmcmd	=err 'invalid combination of operands'
		exit
	    bt_rm_reg:
		local	opcode
		check	@dest.size and not @src.size
		jno	bt_rm_reg_ok
		asmcmd	=err 'operand sizes do not match'
	      bt_rm_reg_ok:
		compute opcode, 0A3h+(postbyte-4) shl 3
		asmcmd	=x86.=select_operand_prefix =@dest,=@src.=size
		asmcmd	=x86.=store_instruction <0Fh,opcode>,=@dest,=@src.=rm
		exit
	    bt_rm_imm:
		check	@src.size and not 1
		jno	bt_rm_imm_ok
		asmcmd	=err 'invalid operand size'
	      bt_rm_imm_ok:
		check	@dest.size
		jno	bt_rm_imm_prefix_ok
		asmcmd	=x86.=select_operand_prefix =@dest,=@dest.=size
	      bt_rm_imm_prefix_ok:
		asmcmd	=x86.=store_instruction <0Fh,0BAh>,=@dest,postbyte,1,=@src.=imm
	end calminstruction

end iterate

iterate <instr,ext>, bsf,0BCh, bsr,0BDh

	calminstruction instr? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	bsf_reg_rm
		asmcmd	=err 'invalid combination of operands'
		exit
	    bsf_reg_rm:
		check	@src.size and not @dest.size
		jno	bsf_reg_rm_ok
		asmcmd	=err 'operand sizes do not match'
	      bsf_reg_rm_ok:
		asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,=@dest.=rm
	end calminstruction

end iterate

iterate <instr,ext>, movzx,0B6h, movsx,0BEh

	calminstruction instr? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	@dest.size > @src.size
		jyes	size_ok
		asmcmd	=err 'operand sizes do not match'
	    size_ok:
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	operands_ok
		asmcmd	=err 'invalid combination of operands'
		exit
	    operands_ok:
		check	@src.size = 2
		jyes	movzx_word
		check	@src.size = 1 | (@src.size = 0 & @dest.size = 2)
		jyes	movzx_byte
		check	@src.size
		jyes	invalid_operand_size
		asmcmd	=err 'operand size not specified'
	    movzx_word:
		asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
		asmcmd	=x86.=store_instruction <0Fh,ext+1>,=@src,=@dest.=rm
		exit
	    movzx_byte:
		asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,=@dest.=rm
		exit
	    invalid_operand_size:
		asmcmd	=err 'invalid operand size'
	end calminstruction

end iterate

iterate <cond,code>, o,0, no,1, c,2, b,2, nae,2, nc,3, nb,3, ae,3, z,4, e,4, nz,5, ne,5, na,6, be,6, a,7, nbe,7, \
		     s,8, ns,9, p,0Ah, pe,0Ah, np,0Bh, po,0Bh, l,0Ch, nge,0Ch, nl,0Dh, ge,0Dh, ng,0Eh, le,0Eh, g,0Fh, nle,0Fh

	calminstruction set#cond? dest*
		asmcmd	=x86.=parse_operand =@dest,dest
		check	@dest.size > 1
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		check	@dest.type = 'reg' | @dest.type = 'mem'
		jyes	operand_ok
		asmcmd	=err 'invalid operand'
		exit
	    operand_ok:
		asmcmd	=x86.=store_instruction <0Fh,90h+code>,=@dest,0
	end calminstruction

end iterate

calminstruction call? dest*

	asmcmd	=x86.=parse_jump_operand =@dest,dest

	check	@dest.type = 'imm'
	jyes	call_imm
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jyes	call_rm
	check	@dest.type = 'far'
	jyes	call_direct_far

    invalid_operand:
	asmcmd	=err 'invalid operand'
	exit

    call_direct_far:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	check	@dest.size = 0
	jyes	prefix_ok
	local	size
	compute size, @dest.size - 2
	asmcmd	=x86.=store_operand_prefix size
      prefix_ok:
	check	@dest.size and not 4 & @dest.size and not 6
	jyes	invalid_operand
	asmcmd	=db 9Ah
	check	(@dest.size = 0 & x86.mode = 16) | @dest.size = 4
	jyes	far_dword
	asmcmd	=dd =@dest.=offset
	asmcmd	=dw =@dest.=segment
	exit
      far_dword:
	asmcmd	=dw =@dest.=offset,=@dest.=segment
	exit

    call_rm:
	check	@dest.size = 6
	jyes	call_rm_pword
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
	asmcmd	=err 'operand size not specified'
	exit

      call_rm_pword:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	asmcmd	=x86.=select_operand_prefix =@dest,4
      call_rm_far:
	asmcmd	=x86.=store_instruction 0FFh,=@dest,11b
	exit

      call_rm_dword:
	check	@dest.jump_type | @dest.type = 'reg'
	jno	call_rm_dword_auto
	check	@dest.jump_type = 'far'
	jyes	call_rm_dword_far
      call_rm_dword_near:
	asmcmd	=x86.=select_operand_prefix =@dest,4
	jump	call_rm_near
      call_rm_dword_auto:
	check	x86.mode = 16
	jno	call_rm_dword_near
      call_rm_dword_far:
	asmcmd	=x86.=select_operand_prefix =@dest,2
	jump	call_rm_far

      call_rm_word:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	asmcmd	=x86.=select_operand_prefix =@dest,2
      call_rm_near:
	asmcmd	=x86.=store_instruction 0FFh,=@dest,10b
	exit

    call_imm:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	check	@dest.size = 2
	jyes	call_imm_word
	check	@dest.size = 4
	jno	invalid_operand
	asmcmd	=x86.=store_operand_prefix 4
	asmcmd	=db 0E8h
	compute @dest.imm, @dest.imm-($+4)
	asmcmd	=dd =@dest.=imm
	exit
      call_imm_word:
	asmcmd	=x86.=store_operand_prefix 2
	asmcmd	=db 0E8h
	compute @dest.imm, @dest.imm-($+2)
	check	@dest.imm relativeto 0
	jno	store_word
	compute @dest.imm, @dest.imm and 0FFFFh
      store_word:
	asmcmd	=dw =@dest.=imm
	exit

end calminstruction

calminstruction jmp? dest*

	asmcmd	=x86.=parse_jump_operand =@dest,dest

	check	@dest.type = 'imm'
	jyes	jmp_imm
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jyes	jmp_rm
	check	@dest.type = 'far'
	jyes	jmp_direct_far

    invalid_operand:
	asmcmd	=err 'invalid operand'
	exit

    jmp_direct_far:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	check	@dest.size = 0
	jyes	prefix_ok
	local	size
	compute size, @dest.size - 2
	asmcmd	=x86.=store_operand_prefix size
      prefix_ok:
	check	@dest.size and not 4 & @dest.size and not 6
	jyes	invalid_operand
	asmcmd	=db 0EAh
	check	(@dest.size = 0 & x86.mode = 16) | @dest.size = 4
	jyes	far_dword
	asmcmd	=dd =@dest.=offset
	asmcmd	=dw =@dest.=segment
	exit
      far_dword:
	asmcmd	=dw =@dest.=offset,=@dest.=segment
	exit

    jmp_rm:
	check	@dest.size = 6
	jyes	jmp_rm_pword
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
	asmcmd	=err 'operand size not specified'
	exit

      jmp_rm_pword:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	asmcmd	=x86.=select_operand_prefix =@dest,4
      jmp_rm_far:
	asmcmd	=x86.=store_instruction 0FFh,=@dest,101b
	exit

      jmp_rm_dword:
	check	@dest.jump_type | @dest.type = 'reg'
	jno	jmp_rm_dword_auto
	check	@dest.jump_type = 'far'
	jyes	jmp_rm_dword_far
      jmp_rm_dword_near:
	asmcmd	=x86.=select_operand_prefix =@dest,4
	jump	jmp_rm_near
      jmp_rm_dword_auto:
	check	x86.mode = 16
	jno	jmp_rm_dword_near
      jmp_rm_dword_far:
	asmcmd	=x86.=select_operand_prefix =@dest,2
	jump	jmp_rm_far

      jmp_rm_word:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	asmcmd	=x86.=select_operand_prefix =@dest,2
      jmp_rm_near:
	asmcmd	=x86.=store_instruction 0FFh,=@dest,100b
	exit

    jmp_imm:
	asmcmd	=x86.=store_operand_prefix =@dest.=size
	check	@dest.jump_type = 'near'
	jyes	jmp_imm_near
	check	@dest.jump_type = 'short'
	jyes	jmp_imm_short_verify
	check	@dest.jump_type
	jyes	invalid_operand
	check	~ $ relativeto 0 & @dest.imm relativeto 0
	jno	jmp_optimize
	compute @dest.imm, @dest.imm + $ - 0 scaleof $
	asmcmd	=err 'invalid address'
      jmp_optimize:
	check	@dest.unresolved
	jyes	jmp_imm_short
	check	@dest.imm relativeto $
	jno	jmp_imm_near
	check	(@dest.imm-($+2)) < 80h & (@dest.imm-($+2)) >= -80h
	jyes	jmp_imm_short
	check	(@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h
	jyes	jmp_imm_short
      jmp_imm_near:
	check	@dest.size = 2
	jyes	jmp_imm_word
	check	@dest.size = 4
	jno	invalid_operand
	asmcmd	=db 0E9h
	compute @dest.imm, @dest.imm-($+4)
	asmcmd	=dd =@dest.=imm
	exit
      jmp_imm_word:
	asmcmd	=db 0E9h
	compute @dest.imm, @dest.imm-($+2)
	check	@dest.imm relativeto 0
	jno	store_word
	compute @dest.imm, @dest.imm and 0FFFFh
      store_word:
	asmcmd	=dw =@dest.=imm
	exit
      jmp_imm_short_verify:
	check	(@dest.imm-($+2)) < 80h & (@dest.imm-($+2)) >= -80h
	jyes	jmp_imm_short
	check	(@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h
	jyes	jmp_imm_short
	asmcmd	=db 0EBh, ?
	jump	relative_jump_out_of_range
      jmp_imm_short:
	asmcmd	=db 0EBh
	compute @dest.imm, (@dest.imm-($+1)) and 0FFh
	asmcmd	=db =@dest.=imm
	exit
      relative_jump_out_of_range:
	asmcmd	=err 'relative jump out of range'
	exit

end calminstruction

iterate <instr,opcode>, jo,70h, jno,71h, jc,72h, jb,72h, jnae,72h, jnc,73h, jnb,73h, jae,73h, jz,74h, je,74h, jnz,75h, jne,75h, jna,76h, jbe,76h, ja,77h, jnbe,77h, \
			js,78h, jns,79h, jp,7Ah, jpe,7Ah, jnp,7Bh, jpo,7Bh, jl,7Ch, jnge,7Ch, jnl,7Dh, jge,7Dh, jng,7Eh, jle,7Eh, jg,7Fh, jnle,7Fh

	calminstruction instr? dest*

		asmcmd	=x86.=parse_jump_operand =@dest,dest

		check	@dest.type <> 'imm' | @dest.jump_type = 'far'
		jyes	invalid_operand

		asmcmd	=x86.=store_operand_prefix =@dest.=size

		check	~ $ relativeto 0 & @dest.imm relativeto 0
		jno	optimize
		compute @dest.imm, @dest.imm + $ - 0 scaleof $
		asmcmd	=err 'invalid address'
	    optimize:

		check	@dest.jump_type <> 'near' & ( @dest.unresolved | ( @dest.imm relativeto $ & @dest.imm-($+2) < 80h & @dest.imm-($+2) >= -80h ) )
		jyes	short
		check	@dest.jump_type <> 'near' & @dest.imm relativeto $ & ( (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h )
		jyes	short
		check	@dest.jump_type = 'short'
		jyes	relative_jump_out_of_range

		asmcmd	=db 0Fh,10h+opcode

		check	@dest.size = 2
		jyes	relative_word
		compute @dest.imm, @dest.imm-($+4)
		asmcmd	=dd =@dest.=imm
		exit
	      relative_word:
		compute @dest.imm, @dest.imm-($+2)
		check	@dest.imm relativeto 0
		jno	store_word
		compute @dest.imm, @dest.imm and 0FFFFh
	      store_word:
		asmcmd	=dw =@dest.=imm
		exit

	    short:
		compute @dest.imm, (@dest.imm-($+2)) and 0FFh
		asmcmd	=db opcode,=@dest.=imm
		exit

	    relative_jump_out_of_range:
		asmcmd	=db ?,?
		asmcmd	=err 'relative jump out of range'
		exit

	    invalid_operand:
		asmcmd	=err 'invalid operand'
		exit

	end calminstruction
end iterate

iterate <instr,opcode,len>, loopnz,0E0h,0, loopne,0E0h,0, loopz,0E1h,0, loope,0E1h,0, loop,0E2h,0, \
			    loopnzw,0E0h,2, loopnew,0E0h,2, loopzw,0E1h,2, loopew,0E1h,2, loopw,0E2h,2, \
			    loopnzd,0E0h,4, loopned,0E0h,4, loopzd,0E1h,4, looped,0E1h,4, loopd,0E2h,4, \
			    jcxz,0E3h,2, jecxz,0E3h,4
	calminstruction instr? dest*

		asmcmd	=x86.=parse_jump_operand =@dest,dest

		check	@dest.type = 'imm' & ( @dest.jump_type = 'short' | ~ @dest.jump_type )
		jno	invalid_operand

		check	len shl 3 and not x86.mode
		jno	address_prefix_ok
		asmcmd	=db 67h
	    address_prefix_ok:
		check	@dest.size shl 3 <> x86.mode
		jno	operand_prefix_ok
		asmcmd	=db 66h
	    operand_prefix_ok:

		asmcmd	=db opcode

		check	@dest.imm-($+1) < 80h & @dest.imm-($+1) >= -80h
		jyes	relative_offset_ok
		check	(@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h
		jyes	relative_offset_ok
		asmcmd	=db ?
		asmcmd	=err 'relative jump out of range'
		exit
	    relative_offset_ok:
		compute @dest.imm, (@dest.imm-($+1)) and 0FFh
		asmcmd	=db =@dest.=imm
		exit

	    invalid_operand:
		asmcmd	=err 'invalid operand'
		exit

	end calminstruction
end iterate

iterate <instr,opcode>, daa,27h, das,2Fh, aaa,37h, aas,3Fh, nop,90h, int3,0CCh, into,0CEh, int1,0F1h, iret,0CFh, salc,0D6h, \
			hlt,0F4h, cmc,0F5h, clc,0F8h, stc,0F9h, cli,0FAh, sti,0FBh, cld,0FCh, std,0FDh, \
			pusha,60h, popa,61h, pushf,9Ch, popf,9Dh, sahf,9Eh, lahf,9Fh, leave,0C9h, \
			insb,6Ch, outsb,6Eh, movsb,0A4h, cmpsb,0A6h, stosb,0AAh, lodsb,0ACh, scasb,0AEh, xlatb,0D7h, \
			clts,<0Fh,06h>, loadall,<0Fh,07h>

	define x86.instr? db opcode

	calminstruction instr?
		assemble x86.instr?
	end calminstruction

end iterate

define x86.o16? x86.store_operand_prefix 2
define x86.o32? x86.store_operand_prefix 4

iterate <instr,opcode>, cbw,98h, cwd,99h, iretw,0CFh, pushaw,60h, popaw,61h, pushfw,9Ch, popfw,9Dh, \
			insw,6Dh, outsw,6Fh, movsw,0A5h, cmpsw,0A7h, stosw,0ABh, lodsw,0ADh, scasw,0AFh

	define x86.instr? db opcode

	calminstruction instr?
		assemble x86.o16
		assemble x86.instr?
	end calminstruction

end iterate

iterate <instr,opcode>, cwde,98h, cdq,99h, iretd,0CFh, pushad,60h, popad,61h, pushfd,9Ch, popfd,9Ch, \
			insd,6Dh, outsd,6Fh, movsd,0A5h, cmpsd,0A7h, stosd,0ABh, lodsd,0ADh, scasd,0AFh

	define x86.instr? db opcode

	calminstruction instr?
		assemble x86.o32
		assemble x86.instr?
	end calminstruction

end iterate

iterate <prefix,opcode>, lock,0F0h, repnz,0F2h, repne,0F2h, rep,0F3h, repz,0F3h, repe,0F3h

	define x86.prefix? db opcode

	calminstruction prefix? instr&
		assemble x86.prefix?
		assemble instr
	end calminstruction

end iterate

calminstruction int? number*
	asmcmd	=db 0CDh,number
end calminstruction

calminstruction aam? number:10
	asmcmd	=db 0D4h,number
end calminstruction

calminstruction aad? number:10
	asmcmd	=db 0D5h,number
end calminstruction

if defined SSE2

	purge movsd?, cmpsd?

end if

calminstruction movs? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	local	size
	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match
	compute size, @dest.size or @src.size
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @dest.type = 'mem' & @dest.mod = 0 & ( (@src.mode = 16 & @src.rm = 4 & @dest.mode = 16 & @dest.rm = 5) | (@src.mode = 32 & @src.rm = 6 & @dest.mode = 32 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	asmcmd	=db =@src.=segment_prefix
    segment_prefix_ok:
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	check	size > 1
	jyes	movs_word
	assemble x86.movsb
	exit
    movs_word:
	asmcmd	=x86.=store_operand_prefix size
	assemble x86.movsw
	exit
    operand_size_not_specified:
	asmcmd	=err 'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit
end calminstruction

calminstruction cmps? src*,dest*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	local	size
	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match
	compute size, @dest.size or @src.size
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @dest.type = 'mem' & @dest.mod = 0 & ( (@src.mode = 16 & @src.rm = 4 & @dest.mode = 16 & @dest.rm = 5) | (@src.mode = 32 & @src.rm = 6 & @dest.mode = 32 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	asmcmd	=db =@src.=segment_prefix
    segment_prefix_ok:
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	check	size > 1
	jyes	cmps_word
	assemble x86.cmpsb
	exit
    cmps_word:
	asmcmd	=x86.=store_operand_prefix size
	assemble x86.cmpsw
	exit
    operand_size_not_specified:
	asmcmd	=err 'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit
end calminstruction

calminstruction stos? dest*
	asmcmd	=x86.=parse_operand =@dest,dest
	check	@dest.size
	jyes	size_ok
	asmcmd	=err 'operand size not specified'
    size_ok:
	check	@dest.type = 'mem' & @dest.mod = 0 & ( (@dest.mode = 16 & @dest.rm = 5) | (@dest.mode = 32 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_operand
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	check	@dest.size > 1
	jyes	stos_word
	assemble x86.stosb
	exit
    stos_word:
	asmcmd	=x86.=store_operand_prefix =@dest.=size
	assemble x86.stosw
	exit
    invalid_operand:
	asmcmd	=err 'invalid operand'
	exit
end calminstruction

calminstruction lods? src*
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.size
	jyes	size_ok
	asmcmd	=err 'operand size not specified'
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & ( (@src.mode = 16 & @src.rm = 4) | (@src.mode = 32 & @src.rm = 6) )
	jno	invalid_operand
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	asmcmd	=db =@src.=segment_prefix
    segment_prefix_ok:
	check	@src.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	check	@src.size > 1
	jyes	lods_word
	assemble x86.lodsb
	exit
    lods_word:
	asmcmd	=x86.=store_operand_prefix =@src.=size
	assemble x86.lodsw
	exit
    invalid_operand:
	asmcmd	=err 'invalid operand'
	exit
end calminstruction

calminstruction scas? dest*
	asmcmd	=x86.=parse_operand =@dest,dest
	check	@dest.size
	jyes	size_ok
	asmcmd	=err 'operand size not specified'
    size_ok:
	check	@dest.type = 'mem' & @dest.mod = 0 & ( (@dest.mode = 16 & @dest.rm = 5) | (@dest.mode = 32 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_operand
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	check	@dest.size > 1
	jyes	scas_word
	assemble x86.scasb
	exit
    scas_word:
	asmcmd	=x86.=store_operand_prefix =@dest.=size
	assemble x86.scasw
	exit
    invalid_operand:
	asmcmd	=err 'invalid operand'
	exit
end calminstruction

calminstruction ins? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@dest.size
	jyes	size_ok
	asmcmd	=err 'operand size not specified'
	compute size, 0
    size_ok:
	check	@src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'mem' & @dest.mod = 0 & ( (@dest.mode = 16 & @dest.rm = 5) | (@dest.mode = 32 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
	jno	invalid_combination_of_operands
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	check	@dest.size > 1
	jyes	ins_word
	assemble x86.insb
	exit
    ins_word:
	asmcmd	=x86.=store_operand_prefix =@dest.=size
	assemble x86.insw
	exit
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit
end calminstruction

calminstruction outs? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.size
	jyes	size_ok
	asmcmd	=err 'operand size not specified'
    size_ok:
	check	@dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'mem' & @src.mod = 0 & ( (@src.mode = 16 & @src.rm = 4) | (@src.mode = 32 & @src.rm = 6) )
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	asmcmd	=db =@src.=segment_prefix
    segment_prefix_ok:
	check	@src.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	check	@src.size > 1
	jyes	outs_word
	assemble x86.outsb
	exit
    outs_word:
	asmcmd	=x86.=store_operand_prefix =@src.=size
	assemble x86.outsw
	exit
    operand_size_not_specified:
	asmcmd	=err 'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	asmcmd	=err 'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
	exit
end calminstruction

calminstruction xlat? src*
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.size > 1
	jno	size_ok
	asmcmd	=err 'invalid operand size'
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & ( (@src.mode = 16 & @src.rm = 7) | (@src.mode = 32 & @src.rm = 3) )
	jno	invalid_operand
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	segment_prefix_ok
	asmcmd	=db =@src.=segment_prefix
    segment_prefix_ok:
	check	@src.mode = x86.mode
	jyes	address_prefix_ok
	asmcmd	=db 67h
    address_prefix_ok:
	assemble x86.xlatb
	exit
    invalid_operand:
	asmcmd	=err 'invalid operand'
	exit
end calminstruction

calminstruction in? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@dest.size
	jyes	size_ok
	asmcmd	=err 'operand size not specified'
    size_ok:
	check	@src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'reg' & @dest.rm = 0
	jyes	in_ax_dx
	check	@src.type = 'imm' & @dest.type = 'reg' & @dest.rm = 0
	jyes	in_ax_imm
	asmcmd	=err 'invalid combination of operands'
	exit
    in_ax_dx:
	check	@dest.size > 1
	jno	in_al_dx
	asmcmd	=x86.=store_operand_prefix =@dest.=size
	asmcmd	=db 0EDh
	exit
    in_al_dx:
	asmcmd	=db 0ECh
	exit
    in_ax_imm:
	check	@dest.size > 1
	jno	in_al_imm
	asmcmd	=x86.=store_operand_prefix =@dest.=size
	asmcmd	=db 0E5h,=@src.=imm
	exit
    in_al_imm:
	asmcmd	=db 0E4h,=@src.=imm
	exit
end calminstruction

calminstruction out? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.size
	jyes	size_ok
	asmcmd	=err 'operand size not specified'
    size_ok:
	check	@dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'reg' & @src.rm = 0
	jyes	out_dx_ax
	check	@dest.type = 'imm' & @src.type = 'reg' & @src.rm = 0
	jyes	out_imm_ax
	asmcmd	=err 'invalid combination of operands'
	exit
    out_dx_ax:
	check	@src.size > 1
	jno	out_dx_al
	asmcmd	=x86.=store_operand_prefix =@src.=size
	asmcmd	=db 0EFh
	exit
    out_dx_al:
	asmcmd	=db 0EEh
	exit
    out_imm_ax:
	check	@src.size > 1
	jno	out_imm_al
	asmcmd	=x86.=store_operand_prefix =@src.=size
	asmcmd	=db 0E7h,=@dest.=imm
	exit
    out_imm_al:
	asmcmd	=db 0E6h,=@dest.=imm
	exit
end calminstruction

calminstruction enter? alloc*,nesting*
	asmcmd	=x86.=parse_operand =@src,alloc
	asmcmd	=x86.=parse_operand =@aux,nesting
	check	(@src.size and not 2) | (@aux.size and not 1)
	jno	size_ok
	asmcmd	=err 'invalid operand size'
    size_ok:
	check	@src.type = 'imm' & @aux.type = 'imm'
	jyes	operand_ok
	asmcmd	=err 'invalid operand'
	exit
    operand_ok:
	asmcmd	=db 0C8h
	asmcmd	=dw =@src.=imm
	asmcmd	=db =@aux.=imm
end calminstruction

calminstruction bound? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.type = 'mem' & @dest.type = 'reg'
	jno	invalid_combination_of_operands
	check	@src.size and not @dest.size
	jno	size_ok
	asmcmd	=err 'operand sizes do not match'
    size_ok:
	asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
	asmcmd	=x86.=store_instruction 62h,=@src,=@dest.=rm
	exit
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
end calminstruction

calminstruction arpl? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
	jno	invalid_combination_of_operands
	check	@src.size = 2
	jno	invalid_operand_size
	check	@dest.size and not @src.size
	jno	size_ok
	asmcmd	=err 'operand sizes do not match'
	jump	size_ok
    invalid_operand_size:
	asmcmd	=err 'invalid operand size'
    size_ok:
	asmcmd	=x86.=store_instruction 63h,=@dest,=@src.=rm
	exit
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
end calminstruction

iterate <instr,ext,postbyte>, lldt,0,2, ltr,0,3, verr,0,4, verw,0,5, lmsw,1,6

	calminstruction instr? dest*
		asmcmd	=x86.=parse_operand =@dest,dest
		check	@dest.size and not 2
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		check	@dest.type = 'reg' | @dest.type = 'mem'
		jyes	operand_ok
		asmcmd	=err 'invalid operand'
		exit
	    operand_ok:
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@dest,postbyte
	end calminstruction

end iterate

iterate <instr,ext,postbyte>, sldt,0,0, str,0,1, smsw,1,4

	calminstruction instr? dest*
		asmcmd	=x86.=parse_operand =@dest,dest
		check	@dest.type = 'reg'
		jyes	select_operand_prefix
		check	@dest.size and not 2
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		check	@dest.type = 'mem'
		jyes	store_instruction
		asmcmd	=err 'invalid combination of operands'
		exit
	    select_operand_prefix:
		asmcmd	=x86.=select_operand_prefix =@dest,=@dest.=size
	    store_instruction:
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@dest,postbyte
	end calminstruction

end iterate

iterate <instr,postbyte>, lgdt,2, lidt,3, sgdt,0, sidt,1

	calminstruction instr? dest*
		asmcmd	=x86.=parse_operand =@dest,dest
		check	@dest.type = 'mem'
		jyes	operand_ok
		asmcmd	=err 'invalid operand'
		exit
	    operand_ok:
		check	@dest.size = 6
		jyes	o32
		check	@dest.size = 5
		jyes	o16
		check	@dest.size
		jno	store_instruction
		asmcmd	=err 'invalid operand size'
		jump	store_instruction
	    o16:
		asmcmd	=x86.=select_operand_prefix =@dest,2
		jump	store_instruction
	    o32:
		asmcmd	=x86.=select_operand_prefix =@dest,4
	    store_instruction:
		asmcmd	=x86.=store_instruction <0Fh,1>,=@dest,postbyte
	end calminstruction

end iterate

iterate <instr,ext>, lar,2, lsl,3

	calminstruction instr? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jno	invalid_combination_of_operands
		check	@src.size and not 2
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,=@dest.=rm
		exit
	    invalid_combination_of_operands:
		asmcmd	=err 'invalid combination of operands'
	end calminstruction

end iterate

calminstruction xbts? dest*,src*,offs*,len*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	asmcmd	=x86.=parse_operand =@src2,offs
	asmcmd	=x86.=parse_operand =@aux,len
	check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg') & \
		@src2.type = 'reg' & @src2.rm = 0 & @aux.type = 'reg' & @aux.size = 1 & @aux.rm = 1
	jno	invalid_combination_of_operands
	check	@src.size and not @dest.size | @src2.size <> @dest.size
	jyes	operand_sizes_no_not_match
	check	@dest.size > 1
	jyes	size_ok
	asmcmd	=err 'invalid operand size'
	jump	size_ok
    operand_sizes_no_not_match:
	asmcmd	=err 'operand sizes do not match'
    size_ok:
	asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
	asmcmd	=x86.=store_instruction <0Fh,0A6h>,=@src,=@dest.=rm
	exit
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
end calminstruction

calminstruction ibts? dest*,offs*,len*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	asmcmd	=x86.=parse_operand =@src2,offs
	asmcmd	=x86.=parse_operand =@aux,len
	check	@src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg') & \
		@src2.type = 'reg' & @src2.rm = 0 & @aux.type = 'reg' & @aux.size = 1 & @aux.rm = 1
	jno	invalid_combination_of_operands
	check	@dest.size and not @src.size | @src2.size <> @src.size
	jyes	operand_sizes_no_not_match
	check	@src.size > 1
	jyes	size_ok
	asmcmd	=err 'invalid operand size'
	jump	size_ok
    operand_sizes_no_not_match:
	asmcmd	=err 'operand sizes do not match'
    size_ok:
	asmcmd	=x86.=select_operand_prefix =@dest,=@src.=size
	asmcmd	=x86.=store_instruction <0Fh,0A7h>,=@dest,=@src.=rm
	exit
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
end calminstruction
