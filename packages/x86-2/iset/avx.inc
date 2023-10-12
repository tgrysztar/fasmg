
element AVX.reg

repeat 16, i:0
	element ymm#i? : AVX.reg + i
end repeat

element AVX_512.reg

element AVX_512.r128 : AVX_512.reg + 16
element AVX_512.r256 : AVX_512.reg + 32
element AVX_512.r512 : AVX_512.reg + 64

repeat 32, i:0
	element zmm#i? : AVX_512.r512 + i
end repeat

repeat 16, i:16
	element xmm#i? : AVX_512.r128 + i
	element ymm#i? : AVX_512.r256 + i
end repeat

element AVX_512.maskreg

repeat 8, i:0
	element k#i? : AVX_512.maskreg + i
end repeat

VEX_0F_W0 = 1
VEX_66_0F_W0 = 1 + 1 shl 8
VEX_F3_0F_W0 = 1 + 10b shl 8
VEX_F2_0F_W0 = 1 + 11b shl 8

VEX_0F38_W0 = 10b
VEX_66_0F38_W0 = 10b + 1 shl 8
VEX_F3_0F38_W0 = 10b + 10b shl 8
VEX_F2_0F38_W0 = 10b + 11b shl 8

VEX_0F3A_W0 = 11b
VEX_66_0F3A_W0 = 11b + 1 shl 8
VEX_F3_0F3A_W0 = 11b + 10b shl 8
VEX_F2_0F3A_W0 = 11b + 11b shl 8

VEX_0F_W1 = VEX_0F_W0 or 8000h
VEX_66_0F_W1 = VEX_66_0F_W0 or 8000h
VEX_F3_0F_W1 = VEX_F3_0F_W0 or 8000h
VEX_F2_0F_W1 = VEX_F2_0F_W0 or 8000h

VEX_0F38_W1 = VEX_0F38_W0 or 8000h
VEX_66_0F38_W1 = VEX_66_0F38_W0 or 8000h
VEX_F3_0F38_W1 = VEX_F3_0F38_W0 or 8000h
VEX_F2_0F38_W1 = VEX_F2_0F38_W0 or 8000h

VEX_0F3A_W1 = VEX_0F3A_W0 or 8000h
VEX_66_0F3A_W1 = VEX_66_0F3A_W0 or 8000h
VEX_F3_0F3A_W1 = VEX_F3_0F3A_W0 or 8000h
VEX_F2_0F3A_W1 = VEX_F2_0F3A_W0 or 8000h

EVEX_AS_VEX = 0
EVEX_W1 = 1 shl 15
EVEX_REQUIRED = 1 shl 10
EVEX_FORBIDDEN = 1 shl 2
EVEX_VL = 1 shl 22
EVEX_VL_AVX2 = 1 shl 32
EVEX_BW = 1 shl 23
EVEX_DQ = 1 shl 24

iterate context, @dest,@src,@src2,@aux

	namespace context

		define mask
		define evex_b
		define memsize
		define broadcast
		define rounding
		define visize

		calminstruction AVX_512.parse_operand#context operand

			call	x86.parse_operand#context, operand

			check	type = 'reg' & size = 1 & rm >= 4 & (~ defined x86.REX_FORBIDDEN | rm and x86.REX_FORBIDDEN)
			jyes	invalid_operand
			check	type = 'imm' & size = 0
			jno	export_common

			check	imm eq 1 elementof imm & 1 metadataof imm relativeto SSE.reg
			jyes	xmm_register
			check	imm eq 1 elementof imm & 1 metadataof imm relativeto AVX.reg
			jyes	ymm_register
			check	1 metadataof (1 metadataof imm) relativeto AVX_512.reg & imm eq 1 elementof imm
			jyes	xyzmm_register
			check	imm eq 1 elementof imm & 1 metadataof imm relativeto AVX_512.maskreg
			jyes	mask_register
			exit

		    invalid_operand:
			err	'invalid operand'

		    xmm_register:

			compute rm, 1 metadataof imm - SSE.reg
			compute size, 16

			jump	export_mmreg

		    ymm_register:

			compute rm, 1 metadataof imm - AVX.reg
			compute size, 32

			jump	export_mmreg

		    mask_register:

			compute rm, 1 metadataof imm - AVX_512.maskreg
			compute size, 8
			compute type, 'maskreg'

			jump	export_reg

		    xyzmm_register:

			compute rm, 1 metadataof imm - 1 elementof (1 metadataof imm)
			compute size, 1 metadataof (1 metadataof imm) - AVX_512.reg

		    export_mmreg:

			compute type, 'mmreg'

		    export_reg:

			compute mod, 11b

		    export_common:

			compute mask, 0
			compute evex_b, 0
			compute memsize, 0

		end calminstruction

		calminstruction AVX_512.parse_k1z_operand#context operand

			local	k1, z
			transform operand
			match	operand {k1} { =z? }, operand
			jyes	k1z
			match	operand {k1}, operand
			jyes	k1
			call	AVX_512.parse_operand#context, operand
			exit
		    k1z:
			compute z, 80h
			jump	masked
		    k1:
			compute z, 0
		    masked:
			call	AVX_512.parse_operand#context, operand
			check	z & type = 'mem'
			jyes	invalid_mask
			check	k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
			jno	invalid_mask
			compute mask, (1 metadataof k1 - AVX_512.maskreg) or z
			exit
		    invalid_mask:
			err	'invalid mask'

		end calminstruction

		calminstruction AVX_512.parse_k1_operand#context operand

			local	k1
			transform operand
			match	operand {k1}, operand
			jyes	k1
			call	AVX_512.parse_operand#context, operand
			exit
		    k1:
			call	AVX_512.parse_operand#context, operand
			check	k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
			jno	invalid_mask
			compute mask, 1 metadataof k1 - AVX_512.maskreg
			exit
		    invalid_mask:
			err	'invalid mask'

		end calminstruction

		calminstruction AVX_512.parse_bcst_operand#context operand,unit

			transform operand
			match	operand {broadcast}, operand
			jyes	mem_bcst
			call	AVX_512.parse_operand#context, operand
			exit

		    invalid_operand:
			err	'invalid operand'

		    mem_bcst:
			call	AVX_512.parse_operand#context, operand

			check	type = 'mem'
			jno	invalid_operand

			compute memsize, unit
			check	memsize
			jyes	implied_unit
			check	size
			jno	operand_size_not_specified
			compute memsize, size
			jump	unit_ok
		    operand_size_not_specified:
			err	'operand size not specified'
			exit
		    implied_unit:
			check	size and not memsize
			jno	unit_ok
			err	'invalid operand size'
			exit
		    unit_ok:

			match	=1to2?, broadcast
			jyes	bcst_2
			match	=1to4?, broadcast
			jyes	bcst_4
			match	=1to8?, broadcast
			jyes	bcst_8
			match	=1to16?, broadcast
			jyes	bcst_16
			err	'invalid broadcast'
			exit
		    bcst_2:
			compute broadcast, 2
			jump	bcst_ok
		    bcst_4:
			compute broadcast, 4
			jump	bcst_ok
		    bcst_8:
			compute broadcast, 8
			jump	bcst_ok
		    bcst_16:
			compute broadcast, 16
		    bcst_ok:
			compute size, memsize * broadcast
			compute evex_b, 1

		end calminstruction

		calminstruction AVX_512.parse_er#context operand,vsize:64

			check	type = 'mem' | size <> vsize
			jyes	invalid_operand

			match	{ =rn?-=sae? }, operand
			jyes	rounding_0
			match	{ =rd?-=sae? }, operand
			jyes	rounding_1
			match	{ =ru?-=sae? }, operand
			jyes	rounding_2
			match	{ =rz?-=sae? }, operand
			jyes	rounding_3
		    invalid_operand:
			err	'invalid operand'
			exit
		    rounding_0:
			compute rounding, 0
			jump	rounding_ok
		    rounding_1:
			compute rounding, 1
			jump	rounding_ok
		    rounding_2:
			compute rounding, 2
			jump	rounding_ok
		    rounding_3:
			compute rounding, 3
			jump	rounding_ok

		    rounding_ok:
			compute evex_b, 1

		end calminstruction

		calminstruction AVX_512.parse_sae#context operand

			check	type = 'mem'
			jyes	invalid_operand

			match	{ =sae? }, operand
			jno	invalid_operand

			compute evex_b, 1
			compute rounding, -1

			exit

		    invalid_operand:
			err	'invalid operand'

		end calminstruction

		calminstruction AVX_512.parse_vsib_operand#context operand

			local	i, pre, suf

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

			jump	invalid_operand

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

			match	pre suf, address
			jno	no_address_size_prefix
			transform pre, x86
			jno	no_address_size_prefix
			match	:pre, pre
			jno	no_address_size_prefix
			arrange address, suf
			check	pre = 4 | pre = 8
			jno	invalid_address_size
			compute mode, pre shl 3
		      no_address_size_prefix:

			compute mode, 0
			compute scale, 0
			compute index, 0
			compute base, 0
			compute auto_relative, 0

			check	size
			jyes	size_override
			compute size, sizeof address
		      size_override:

			compute address, address
			compute base_registers, 0
			compute index_registers, 0
			compute i, 1
		      extract_registers:
			check	i > elementsof address
			jyes	registers_extracted
			check	i metadataof address relativeto SSE.reg | i metadataof address relativeto AVX.reg | 1 metadataof (i metadataof address) relativeto AVX_512.reg
			jyes	index_term
			check	i metadataof address relativeto x86.r32 | i metadataof address relativeto x86.r64
			jno	next_term
			compute base_registers, base_registers + i elementof address * i scaleof address
			jump	next_term
		      index_term:
			compute index_registers, index_registers + i elementof address * i scaleof address
		      next_term:
			compute i, i+1
			jump	extract_registers
		      registers_extracted:
			compute displacement, address - base_registers - index_registers
			compute auto_relative, 0

			check	elementsof index_registers = 1
			jno	invalid_address
			compute scale, 1 scaleof index_registers
			compute index, 0 scaleof (1 metadataof index_registers)
			check	scale and (scale-1) | scale > 8
			jyes	invalid_address
			check	1 metadataof index_registers relativeto SSE.reg
			jyes	xmm_index
			check	1 metadataof index_registers relativeto AVX.reg
			jyes	ymm_index
			compute visize, 1 metadataof (1 metadataof index_registers) - AVX_512.reg
			jump	index_ok
		      ymm_index:
			compute visize, 32
			jump	index_ok
		      xmm_index:
			compute visize, 16
		      index_ok:

			compute rm, 4
			check	elementsof base_registers = 1 & 1 scaleof base_registers = 1
			jyes	base_and_index
			check	elementsof base_registers = 0
			jno	invalid_address
			compute base, 5
			compute displacement_size, 4
			compute mod, 0
			compute mode, x86.mode
			check	mode > 16
			jyes	ready
			compute mode, 32
			jump	ready
		      base_and_index:
			compute base, 0 scaleof (1 metadataof base_registers)
			check	mode & mode <> 0 scaleof (1 metadataof (1 metadataof base_registers)) shl 3
			jyes	invalid_address
			compute mode, 0 scaleof (1 metadataof (1 metadataof base_registers)) shl 3

		      setup_displacement:
			check	displacement relativeto 0
			jno	displacement_32bit
			check	displacement = 0 & rm and 111b <> 5 & (rm <> 4 | base and 111b <> 5)
			jyes	displacement_empty
			check	displacement < 80h & displacement >= -80h
			jyes	displacement_8bit
			check	displacement - 1 shl mode >= -80h & displacement < 1 shl mode
			jyes	displacement_8bit_wrap
		      displacement_32bit:
			compute displacement_size, 4
			compute mod, 2
			jump	ready
		      displacement_8bit_wrap:
			compute displacement, displacement - 1 shl mode
		      displacement_8bit:
			compute displacement_size, 1
			compute mod, 1
			jump	ready
		      index_only:
			compute displacement_size, 4
			compute mod, 0
			jump	ready
		      displacement_empty:
			compute displacement_size, 0
			compute mod, 0

		  ready:

			compute mask, 0
			compute evex_b, 0
			compute memsize, 0

			exit

		  invalid_operand:
			err	'invalid operand'
			exit
		  invalid_address:
			err	'invalid address'
			exit
		  invalid_address_size:
			err	'invalid address size'
			exit

		end calminstruction

		calminstruction AVX_512.parse_k1_vsib_operand#context operand

			local	k1
			transform operand
			match	operand {k1}, operand
			jyes	k1
			call	AVX_512.parse_vsib_operand#context, operand
			exit
		    k1:
			call	AVX_512.parse_vsib_operand#context, operand
			check	k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
			jno	invalid_mask
			compute mask, 1 metadataof k1 - AVX_512.maskreg
			exit
		    invalid_mask:
			err	'invalid mask'

		end calminstruction

		calminstruction AVX.store_instruction#context vsize*,vex_mpw*,opcode*,reg*,vreg:0,imm_size:0,immediate
			xcall	AVX_512.store_instruction#context, vsize,vex_mpw,(EVEX_FORBIDDEN),opcode,(0),reg,vreg,imm_size,immediate
		end calminstruction

		calminstruction AVX_512.store_instruction#context vsize*,vex_mpw*,evex_f*,opcode*,mask*,reg*,vreg:0,imm_size:0,immediate

			check	segment_prefix
			jno	segment_prefix_ok

			check	mode = 64
			jyes	segment_in_long_mode
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
			jump	store_segment_prefix
		    segment_in_long_mode:
			check	segment_prefix < 64h
			jyes	segment_prefix_ok
		    store_segment_prefix:
			emit	1, segment_prefix
		    segment_prefix_ok:

			check	mod <> 11b & mode <> x86.mode
			jno	addressing_prefix_ok
			check	mode = 64 | (mode = 16 & x86.mode = 64)
			jno	store_addressing_prefix
			err	'illegal addressing mode'
		    store_addressing_prefix:
			emit	1, 67h
		    addressing_prefix_ok:

			compute evex, vex_mpw
			compute evex_flags, evex_f

			check	x86.simd >= x86.AVX512F.simd
			jyes	avx_level_ok
			compute evex_flags, evex_flags or EVEX_FORBIDDEN
		    avx_level_ok:

			check	evex_b
			jno	evex_L'L
			compute evex, evex or evex_b shl 20
		    evex_L'L:
			check	mod = 11b & evex_b & rounding >= 0
			jyes	evex_rounding
			check	vsize = 64
			jyes	evex_L'
			check	evex_flags and EVEX_VL & x86.ext and x86.AVX512VL.ext = 0
			jno	evex_L
			compute evex_flags, evex_flags or EVEX_FORBIDDEN
		    evex_L:
			check	vsize = 32
			jno	evex_mask
			compute evex, evex or 1 shl 21
			check	evex_flags and EVEX_VL_AVX2
			jno	evex_mask
			call	x86.require.AVX2
			jump	evex_mask
		    evex_L':
			compute evex, evex or 1 shl 22
			jump	evex_mask
		    evex_rounding:
			compute evex, evex or rounding shl 21

		    evex_mask:
			check	mask
			jno	evex_X
			compute evex, evex or mask shl 16

		    evex_X:
			check	rm and 10000b | (mod <> 11b & mode > 16 & rm = 4 & index and 1000b)
			jno	evex_B
			compute evex, evex or 1 shl 6
		    evex_B:
			check	rm and 1000b | (mod <> 11b & mode > 16 & rm = 4 & base and 1000b)
			jno	evex_R'
			compute evex, evex or 1 shl 5
		    evex_R':
			check	reg and 10000b
			jno	evex_R
			compute evex, evex or 1 shl 4
		    evex_R:
			check	reg and 1000b
			jno	evex_V'
			compute evex, evex or 1 shl 7
		    evex_V':
			check	vreg and 10000b
			jno	evex_vvvv
			compute evex, evex or 1 shl 19
		    evex_vvvv:
			compute evex, evex or (vreg and 1111b) shl 11

			check	x86.mode < 64 & evex and 00001000_01000000_11110000b
			jno	allowed
			err	'instruction requires long mode'
		    allowed:

			local	evex_displacement_size, compressed_displacement

			check	displacement_size
			jno	no_displacement_compression
			compute displacement, displacement
			check	(mode > 16 & rm = 5) | (mode = 16 & rm = 6)
			jyes	no_displacement_compression
			check	memsize
			jyes	displacement_compression
			compute memsize, vsize
		    displacement_compression:
			check	displacement relativeto 0 & displacement mod? memsize = 0
			jno	displacement_incompressible
			compute compressed_displacement, displacement / memsize
			check	compressed_displacement < 80h & compressed_displacement >= -80h
			jyes	displacement_compressed
			check	compressed_displacement - 1 shl mode >= -80h & compressed_displacement < 1 shl mode
			jno	displacement_incompressible
			compute compressed_displacement, compressed_displacement - 1 shl mode
		    displacement_compressed:
			compute evex_displacement_size, 1
			jump	choose_prefix
		    displacement_incompressible:
			compute evex_displacement_size, 4
			check	mode > 16
			jyes	choose_prefix
			compute evex_displacement_size, 2
			jump	choose_prefix
		    no_displacement_compression:
			compute evex_displacement_size, displacement_size

		    choose_prefix:
			check	evex_flags and EVEX_REQUIRED | evex and 11011111_00000000_00010000b | rm and 10000b
			jyes	evex_required
			check	~ evex_flags and EVEX_FORBIDDEN & evex_displacement_size + 1 < displacement_size
			jyes	evex
			jump	vex
		    evex_required:
			check	evex_flags and EVEX_FORBIDDEN
			jno	evex
			err	'invalid operand'

		    vex:
			call	x86.require.AVX
			compute vex, evex and 11111011_11111111b or (evex and 1 shl 21) shr (21-10)
			check	vex and 10000000_01111111b <> 1
			jyes	vex_3byte
		    vex_2byte:
			emit	1, 0C5h
			emit	1, ((vex and 10000000b) or ((vex shr 8) and 1111111b)) xor 11111000b
			jump	evex_done
		    vex_3byte:
			emit	1, 0C4h
			emit	1, (vex and 11111111b) xor 11100000b
			emit	1, (vex shr 8) xor 01111000b
			jump	evex_done

		    evex:
			call	x86.require.AVX512F
			check	evex_flags and EVEX_BW
			jno	bw_ok
			call	x86.require.AVX512BW
		      bw_ok:
			check	evex_flags and EVEX_DQ
			jno	dq_ok
			call	x86.require.AVX512DQ
		      dq_ok:
			compute evex, evex or 1 shl 10
			check	evex_flags and EVEX_W1
			jno	evex_4byte
			compute evex, evex or 1 shl 15
		    evex_4byte:
			emit	4, 62h + (evex xor 00001000_01111000_11110000b) shl 8
			check	mod <> 11b & mod <> 0 & evex_displacement_size > 0
			jno	evex_done
			compute displacement_size, evex_displacement_size
			check	evex_displacement_size = 1
			jyes	evex_compressed_displacement
			compute mod, 2
			jump	evex_done
		    evex_compressed_displacement:
			compute displacement, compressed_displacement
			compute mod, 1
		    evex_done:

			asm	db opcode
			emit	1, mod shl 6 + (reg and 111b) shl 3 + rm and 111b

			check	mod <> 11b & rm = 4 & mode <> 16
			jno	sib_ok
			emit	1, (bsf scale) shl 6 + (index and 111b) shl 3 + base and 111b
		    sib_ok:

			check	displacement_size = 1
			jyes	displacement_8bit
			check	displacement_size = 2
			jyes	displacement_16bit
			check	displacement_size = 4 | displacement_size = 8
			jno	displacement_ok

			check	auto_relative
			jno	auto_relative_ok
			check	imm_size < 8
			jyes	adjust_auto_relative_displacement
			compute displacement, displacement - ($ + 4 + 4)
			jump	auto_relative_ok
		      adjust_auto_relative_displacement:
			compute displacement, displacement - ($ + 4 + imm_size)
		      auto_relative_ok:

			check	mode = 64 & displacement relativeto 0
			jno	displacement_ready
			check	displacement - 1 shl 64 >= -80000000h & displacement < 1 shl 64
			jyes	adjust_displacement_wrap
			check	displacement >= -80000000h & displacement < 80000000h
			jyes	displacement_ready
			err	'address value out of signed range'
		      adjust_displacement_wrap:
			compute displacement, displacement - 1 shl 64
		      displacement_ready:

			call	dword, displacement

			jump	displacement_ok
		    displacement_16bit:
			call	word, displacement
			jump	displacement_ok
		    displacement_8bit:
			emit	1, displacement
		    displacement_ok:

			check	imm_size = 1
			jyes	immediate_8bit
			check	imm_size = 2
			jyes	immediate_16bit
			check	imm_size = 4
			jyes	immediate_32bit
			check	imm_size = 8
			jno	immediate_ok
			call	x86.simm32, immediate
			jump	immediate_ok
		    immediate_32bit:
			compute imm, +immediate
			call	dword, imm
			jump	immediate_ok
		    immediate_16bit:
			compute imm, +immediate
			call	word, imm
			jump	immediate_ok
		    immediate_8bit:
			compute imm, +immediate
			emit	1, imm
		    immediate_ok:

		end calminstruction

	end namespace

end iterate

macro AVX_512.basic_instruction_bcst_er vex_mpw,evex_f,opcode,unit,dest,src,src_er&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	match src2=,er, src_er
		AVX_512.parse_operand@src2 src2
		AVX_512.parse_er@src2 er
	else
		AVX_512.parse_bcst_operand@src2 src_er,unit
	end match
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if @src.size <> @dest.size | @src2.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_bcst_sae vex_mpw,evex_f,opcode,unit,dest,src,src_sae&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	match src2=,sae, src_sae
		AVX_512.parse_operand@src2 src2
		AVX_512.parse_sae@src2 sae
	else
		AVX_512.parse_bcst_operand@src2 src_sae,unit
	end match
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if @src.size <> @dest.size | @src2.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_bcst_sae_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_bcst_operand@src2 src2,unit
	match sae=,imm, aux
		AVX_512.parse_sae@src2 sae
		x86.parse_operand@aux imm
	else
		x86.parse_operand@aux aux
	end match
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
		if @src.size <> @dest.size | @src2.size and not @dest.size | @aux.size and not 1
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_bcst vex_mpw,evex_f,opcode,unit,dest,src,src2
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_bcst_operand@src2 src2,unit
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if @src.size <> @dest.size | @src2.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_bcst_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_bcst_operand@src2 src2,unit
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
		if @aux.size and not 1
			err 'invalid operand size'
		else if @src.size <> @dest.size | @src2.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_er vex_mpw,evex_f,opcode,unit,dest,src,src_er&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	match src2=,er, src_er
		AVX_512.parse_operand@src2 src2
		AVX_512.parse_er@src2 er,(unit-1) and not 15 + 16
	else
		AVX_512.parse_operand@src2 src_er
	end match
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) )
			err 'invalid operand size'
		else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
			err 'operand sizes do not match'
		end if
		@src2.memsize = unit
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_sae vex_mpw,evex_f,opcode,unit,dest,src,src_sae&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	match src2=,sae, src_sae
		AVX_512.parse_operand@src2 src2
		AVX_512.parse_sae@src2 sae
	else
		AVX_512.parse_operand@src2 src_sae
	end match
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) )
			err 'invalid operand size'
		else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
			err 'operand sizes do not match'
		end if
		@src2.memsize = unit
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_sae_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	match sae=,imm, aux
		AVX_512.parse_sae@src2 sae
		x86.parse_operand@aux imm
	else
		x86.parse_operand@aux aux
	end match
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
		if ( unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) ) ) | @aux.size and not 1
			err 'invalid operand size'
		else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
			err 'operand sizes do not match'
		end if
		@src2.memsize = unit
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction vex_mpw,evex_f,opcode,unit,dest,src,src2
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) )
			err 'invalid operand size'
		else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
			err 'operand sizes do not match'
		end if
		@src2.memsize = unit
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.basic_instruction_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
		if ( unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) ) ) | @aux.size and not 1
			err 'invalid operand size'
		else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
			err 'operand sizes do not match'
		end if
		@src2.memsize = unit
		AVX_512.store_instruction@src2 @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.single_source_instruction_bcst_er vex_mpw,evex_f,opcode,unit,dest,src_er&
	AVX_512.parse_k1z_operand@dest dest
	match src=,er, src_er
		AVX_512.parse_operand@src src
		AVX_512.parse_er@src er
	else
		AVX_512.parse_bcst_operand@src src_er,unit
	end match
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.single_source_instruction_bcst_sae vex_mpw,evex_f,opcode,unit,dest,src_sae&
	AVX_512.parse_k1z_operand@dest dest
	match src=,sae, src_sae
		AVX_512.parse_operand@src src
		AVX_512.parse_sae@src sae
	else
		AVX_512.parse_bcst_operand@src src_sae,unit
	end match
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.single_source_instruction_bcst_sae_imm8 vex_mpw,evex_f,opcode,unit,dest,src,aux&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_bcst_operand@src src,unit
	match sae=,imm, aux
		AVX_512.parse_sae@src sae
		x86.parse_operand@aux imm
	else
		x86.parse_operand@aux aux
	end match
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
		if @aux.size and not 1
			err 'invalid operand size'
		else if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.single_source_instruction_bcst vex_mpw,evex_f,opcode,unit,dest,src&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_bcst_operand@src src,unit
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.single_source_instruction vex_mpw,evex_f,opcode,unit,dest,src
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src.type = 'mem' & @src.size and not unit) )
			err 'invalid operand size'
		else if @src.size and not @dest.size & (unit = 0 | @src.type = 'mmreg')
			err 'operand sizes do not match'
		end if
		@src.memsize = unit
		AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro AVX_512.single_source_instruction_imm8 vex_mpw,evex_f,opcode,unit,dest,src,aux
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		if @aux.size and not 1
			err 'invalid operand size'
		else if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src.type = 'mem' & @src.size and not unit) )
			err 'invalid operand size'
		else if @src.size and not @dest.size & (unit = 0 | @src.type = 'mmreg')
			err 'operand sizes do not match'
		end if
		@src.memsize = unit
		AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,opcode>, add,58h, mul,59h, sub,5Ch, div,5Eh

	macro v#instr#pd? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst_er VEX_66_0F_W0,EVEX_W1+EVEX_VL,opcode,8,dest,src,src2
	end macro

	macro v#instr#ps? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst_er VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
	end macro

	macro v#instr#sd? dest*,src*,src2*&
		AVX_512.basic_instruction_er VEX_F2_0F_W0,EVEX_W1,opcode,8,dest,src,src2
	end macro

	macro v#instr#ss? dest*,src*,src2*&
		AVX_512.basic_instruction_er VEX_F3_0F_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, min,5Dh, max,5Fh

	macro v#instr#pd? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst_sae VEX_66_0F_W0,EVEX_W1+EVEX_VL,opcode,8,dest,src,src2
	end macro

	macro v#instr#ps? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst_sae VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
	end macro

	macro v#instr#sd? dest*,src*,src2*&
		AVX_512.basic_instruction_sae VEX_F2_0F_W0,EVEX_W1,opcode,8,dest,src,src2
	end macro

	macro v#instr#ss? dest*,src*,src2*&
		AVX_512.basic_instruction_sae VEX_F3_0F_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, unpckh,15h, unpckl,14h

	macro v#instr#pd? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_W1+EVEX_VL,opcode,8,dest,src,src2
	end macro

	macro v#instr#ps? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, addsub,0D0h, hadd,7Ch, hsub,7Dh

	macro v#instr#pd? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F_W0,EVEX_FORBIDDEN,opcode,0,dest,src,src2
	end macro

	macro v#instr#ps? dest*,src*,src2*
		AVX_512.basic_instruction VEX_F2_0F_W0,EVEX_FORBIDDEN,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, rsqrt,52h, rcp,53h

	macro v#instr#ps? dest*,src*
		AVX_512.single_source_instruction VEX_0F_W0,EVEX_FORBIDDEN,opcode,0,dest,src
	end macro

	macro v#instr#ss? dest*,src*,src2*
		AVX_512.basic_instruction VEX_F3_0F_W0,EVEX_FORBIDDEN,opcode,4,dest,src,src2
	end macro

end iterate

macro vsqrtpd? dest*,src*&
	AVX_512.single_source_instruction_bcst_er VEX_66_0F_W0,EVEX_W1+EVEX_VL,51h,8,dest,src
end macro

macro vsqrtps? dest*,src*&
	AVX_512.single_source_instruction_bcst_er VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,51h,4,dest,src
end macro

macro vsqrtsd? dest*,src*,src2*&
	AVX_512.basic_instruction_er VEX_F2_0F_W0,EVEX_W1,51h,8,dest,src,src2
end macro

macro vsqrtss? dest*,src*,src2*&
	AVX_512.basic_instruction_er VEX_F3_0F_W0,EVEX_AS_VEX,51h,4,dest,src,src2
end macro

macro vroundpd? dest*,src*,aux*
	AVX_512.single_source_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,9,0,dest,src,aux
end macro

macro vroundps? dest*,src*,aux*
	AVX_512.single_source_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,8,0,dest,src,aux
end macro

macro vroundsd? dest*,src*,src2*,aux*
	AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,0Bh,8,dest,src,src2,aux
end macro

macro vroundss? dest*,src*,src2*,aux*
	AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,0Ah,4,dest,src,src2,aux
end macro

macro vshufpd? dest*,src*,src2*,aux*&
	AVX_512.basic_instruction_bcst_imm8 VEX_66_0F_W0,EVEX_W1+EVEX_VL,0C6h,8,dest,src,src2,aux
end macro

macro vshufps? dest*,src*,src2*,aux*&
	AVX_512.basic_instruction_bcst_imm8 VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,0C6h,4,dest,src,src2,aux
end macro

iterate <instr,opcode>, blendps,0Ch, blendpd,0Dh

	macro v#instr? dest*,src*,src2*,imm*
		AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,opcode,0,dest,src,src2,imm
	end macro

end iterate

iterate <instr,opcode>, blendvps,4Ah, blendvpd,4Bh

	macro v#instr? dest*,src*,src2*,mask*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		AVX_512.parse_operand@aux mask
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'mmreg'
			if @src.size <> @dest.size | @src2.size and not @dest.size | @aux.size <> @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @dest.size,VEX_66_0F3A_W0,EVEX_FORBIDDEN,opcode,0,@dest.rm,@src.rm,1,(@aux.rm and 1111b) shl 4
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vbroadcastss? dest*,src*
	require AVX2+
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
		if (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 4)
			err 'invalid operand size'
		end if
		@src2.memsize = 4
		AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,18h,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vbroadcastsd? dest*,src*
	require AVX2+
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
		if @dest.size = 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 8)
			err 'invalid operand size'
		end if
		@src.memsize = 8
		AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_W1+EVEX_VL,19h,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,opcode,opcode_g,msize>, vpbroadcastd,58h,7Ch,4, vpbroadcastq,59h,7Ch,8

	macro instr? dest*,src*
		require AVX2+
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if (@src.type='mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not msize)
				err 'invalid operand size'
			end if
			@src.memsize = msize
			AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX,opcode,@dest.mask,@dest.rm
		else if @dest.type = 'mmreg' & @src.type = 'reg'
			if @src.size <> msize & (@src.size <> 4 | msize = 8)
				err 'invalid operand size'
			end if
			@src.memsize = msize
			if msize = 8
				AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode_g,@dest.mask,@dest.rm
			else
				AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode_g,@dest.mask,@dest.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vbroadcastf128? dest*,src*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & @src.type = 'mem'
		if @dest.size <> 32 | @src.size and not 16
			err 'invalid operand size'
		end if
		AVX_512.store_instruction@src 32,VEX_66_0F38_W0,EVEX_FORBIDDEN,1Ah,0,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw,opcode,msize>, vbroadcastf32x4,VEX_66_0F38_W0,1Ah,16, vbroadcastf64x4,VEX_66_0F38_W1,1Bh,32, \
				      vbroadcasti32x4,VEX_66_0F38_W0,5Ah,16, vbroadcasti64x4,VEX_66_0F38_W1,5Bh,32

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @dest.size <= msize | @src.size and not msize
				err 'invalid operand size'
			end if
			@src.memsize = msize
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode,unit>, vshuff32x4,VEX_66_0F3A_W0,23h,4, vshuff64x2,VEX_66_0F3A_W1,23h,4, \
				     vshufi32x4,VEX_66_0F3A_W0,43h,4, vshufi64x2,VEX_66_0F3A_W1,43h,4

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_bcst_operand@src2 src2,unit
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @dest.size < 32 | @aux.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vextractps? dest*,src*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if @dest.size and not 4 | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.memsize = 4
		AVX_512.store_instruction@dest 16,VEX_66_0F3A_W0,EVEX_AS_VEX,17h,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vinsertps? dest*,src*,src2*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
		if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'mmreg' & @src2.size <> 16) | (@src2.type = 'mem' & @src2.size and not 4) | @aux.size and not 1
			err 'invalid operand size'
		end if
		@src2.memsize = 4
		AVX_512.store_instruction@src2 16,VEX_66_0F3A_W0,EVEX_AS_VEX,21h,0,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vextractf128? dest*,src*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if @dest.size and not 16 | @src.size <> 32 | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX_512.store_instruction@dest 32,VEX_66_0F3A_W0,EVEX_FORBIDDEN,19h,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vinsertf128? dest*,src*,src2*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
		if @dest.size <> 32 | @src.size <> 32 | @src2.size and not 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX_512.store_instruction@src2 32,VEX_66_0F3A_W0,EVEX_FORBIDDEN,18h,0,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro


iterate <instr,vex_mpw,opcode,msize>, vextractf32x4,VEX_66_0F3A_W0,19h,16, vextractf64x4,VEX_66_0F3A_W1,1Bh,32, \
				      vextracti32x4,VEX_66_0F3A_W0,39h,16, vextracti64x4,VEX_66_0F3A_W1,3Bh,32

	macro instr? dest*,src*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not msize | @src.size <= msize | @aux.size and not 1
				err 'invalid operand size'
			end if
			@dest.memsize = msize
			AVX_512.store_instruction@dest @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode,msize>, vinsertf32x4,VEX_66_0F3A_W0,18h,16, vinsertf64x4,VEX_66_0F3A_W1,1Ah,32, \
				      vinserti32x4,VEX_66_0F3A_W0,38h,16, vinserti64x4,VEX_66_0F3A_W1,3Ah,32

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <= msize | @src.size <= msize | @src2.size and not msize | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src2.memsize = msize
			AVX_512.store_instruction@src2 @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_mpw,unit>, vcmpps,VEX_0F_W0,VEX_0F_W0,4, vcmppd,VEX_66_0F_W0,VEX_66_0F_W1,8

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_bcst_operand@src2 src2,unit
		match sae=,imm, aux
			AVX_512.parse_sae@src2 sae
			x86.parse_operand@aux imm
		else
			x86.parse_operand@aux aux
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,vex_mpw,EVEX_FORBIDDEN,0C2h,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,evex_mpw,EVEX_REQUIRED+EVEX_VL,0C2h,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_mpw,unit>, vcmpss,VEX_F3_0F_W0,VEX_F3_0F_W0,4, vcmpsd,VEX_F2_0F_W0,VEX_F2_0F_W1,8

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		match sae=,imm, aux
			AVX_512.parse_sae@src2 sae
			x86.parse_operand@aux imm
		else
			x86.parse_operand@aux aux
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @dest.size <> 16 | (@src2.type = 'mem' & @src2.size and not unit)
				err 'invalid operand size'
			else if @dest.size <> @src.size | (@src2.type = 'mmreg' & @src2.size <> @dest.size)
				err 'operand sizes do not match'
			end if
			@src2.memsize = unit
			AVX_512.store_instruction@src2 @src.size,vex_mpw,EVEX_FORBIDDEN,0C2h,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @src.size <> 16 | (@src2.type = 'mem' & @src2.size and not unit) | @aux.size and not 1
				err 'invalid operand size'
			else if @src2.type = 'mmreg' & @src2.size <> @src.size
				err 'operand sizes do not match'
			end if
			@src2.memsize = unit
			AVX_512.store_instruction@src2 @src.size,evex_mpw,EVEX_REQUIRED,0C2h,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <cond,code>, eq,0, lt,1, le,2, unord,3, neq,4, nlt,5, nle,6, ord,7, \
		     eq_uq,8, nge,9, ngt,0Ah, false,0Bh, neq_qq,0Ch, ge,0Dh, gt,0Eh, true,0Fh, \
		     eq_os,10h, lt_oq,11h, le_oq,12h, unord_s,13h, neq_us,14h, nlt_uq,15h, nle_uq,16h, ord_s,17h, \
		     eq_us,18h, nge_uq,19h, ngt_uq,1Ah, false_os,1Bh, neq_os,1Ch, ge_oq,1Dh, gt_oq,1Eh, true_us,1Fh

	macro vcmp#cond#pd? dest*,src*,src2*&
		vcmppd dest,src,src2,code
	end macro

	macro vcmp#cond#ps? dest*,src*,src2*&
		vcmpps dest,src,src2,code
	end macro

	macro vcmp#cond#sd? dest*,src*,src2*&
		vcmpsd dest,src,src2,code
	end macro

	macro vcmp#cond#ss? dest*,src*,src2*&
		vcmpss dest,src,src2,code
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode,unit>, vcomiss,VEX_0F_W0,EVEX_AS_VEX,2Fh,4, vcomisd,VEX_66_0F_W0,EVEX_W1,2Fh,8, vucomiss,VEX_0F_W0,EVEX_AS_VEX,2Eh,4, vucomisd,VEX_66_0F_W0,EVEX_W1,2Eh,8

	macro instr? dest*,src_sae*&
		AVX_512.parse_k1z_operand@dest dest
		match src=,sae, src_sae
			AVX_512.parse_operand@src src
			AVX_512.parse_sae@src sae
		else
			AVX_512.parse_operand@src src_sae
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src.type = 'mem' & @src.size and not unit) )
				err 'invalid operand size'
			else if @src.size and not @dest.size & (unit = 0 | @src.type = 'mmreg')
				err 'operand sizes do not match'
			end if
			@src.memsize = unit
			AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, kandw,41h, kandnw,42h, knotw,44h, korw,45h, kxnorw,46h, kxorw,47h

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
			AVX.store_instruction@src2 32,VEX_0F_W0,opcode,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, knotw,44h, kortestw,98h

	macro instr? dest*,src*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'maskreg' & @src.type = 'maskreg'
			AVX.store_instruction@src 16,VEX_0F_W0,opcode,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro kmovw? dest*,src*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'maskreg' & (@src.type = 'maskreg' | @src.type = 'mem')
		if @src.type = 'mem' & @src.size and not 2
			err 'invalid operand size'
		end if
		AVX.store_instruction@src 16,VEX_0F_W0,90h,@dest.rm
	else if @dest.type = 'mem' & @src.type = 'maskreg'
		if @dest.size and not 2
			err 'invalid operand size'
		end if
		AVX.store_instruction@dest 16,VEX_0F_W0,91h,@src.rm
	else if @dest.type = 'maskreg' & @src.type = 'reg'
		if @src.size <> 4
			err 'invalid operand size'
		end if
		AVX.store_instruction@src 16,VEX_0F_W0,92h,@dest.rm
	else if @dest.type = 'reg' & @src.type = 'maskreg'
		if @dest.size <> 4
			err 'invalid operand size'
		end if
		AVX.store_instruction@src 16,VEX_0F_W0,93h,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw,opcode>, kshiftrw,VEX_66_0F3A_W1,30h, kshiftlw,VEX_66_0F3A_W1,32h

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction@src 16,vex_mpw,opcode,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro kunpckbw? dest*,src*,src2*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
		AVX.store_instruction@src2 32,VEX_66_0F_W0,4Bh,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,evex_f,opcode>, vcvtdq2pd,EVEX_AS_VEX+EVEX_VL,0E6h, vcvtudq2pd,EVEX_REQUIRED+EVEX_VL,7Ah

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_bcst_operand@src src,4
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
				err 'invalid operand size'
			end if
			if @src.memsize = 0
				@src.memsize = @dest.size shr 1
			end if
			AVX_512.store_instruction@src @dest.size,VEX_F3_0F_W0,evex_f,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode>, vcvtpd2dq,VEX_F2_0F_W0,EVEX_W1+EVEX_VL,0E6h, vcvtpd2ps,VEX_66_0F_W0,EVEX_W1+EVEX_VL,5Ah, vcvtpd2udq,VEX_0F_W1,EVEX_REQUIRED+EVEX_VL,79h

	macro instr? dest*,src_er*&
		AVX_512.parse_k1z_operand@dest dest
		match src=,er, src_er
			AVX_512.parse_operand@src src
			AVX_512.parse_er@src er
		else
			AVX_512.parse_bcst_operand@src src_er,8
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size = 0
				if @dest.size = 16
					err 'operand size not specified'
				else
					@src.size = 64
				end if
			end if
			if (@src.size shr 1 - 1) and not 15 + 16 <> @dest.size | @src.size > 64
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src @src.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode>, vcvtps2pd,VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,5Ah

	macro instr? dest*,src_sae*&
		AVX_512.parse_k1z_operand@dest dest
		match src=,sae, src_sae
			AVX_512.parse_operand@src src
			AVX_512.parse_sae@src sae
		else
			AVX_512.parse_bcst_operand@src src_sae,4
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
				err 'invalid operand size'
			end if
			if @src.memsize = 0
				@src.memsize = @dest.size shr 1
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode>, vcvttpd2dq,VEX_66_0F_W0,EVEX_W1+EVEX_VL,0E6h, vcvttpd2udq,VEX_0F_W1,EVEX_REQUIRED+EVEX_VL,78h

	macro instr? dest*,src_sae*&
		AVX_512.parse_k1z_operand@dest dest
		match src=,sae, src_sae
			AVX_512.parse_operand@src src
			AVX_512.parse_sae@src sae
		else
			AVX_512.parse_bcst_operand@src src_sae,8
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size = 0
				if @dest.size = 16
					err 'operand size not specified'
				else
					@src.size = 64
				end if
			end if
			if (@src.size shr 1 - 1) and not 15 + 16 <> @dest.size | @src.size > 64
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src @src.size,vex_mpw,evex_f,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode>, vcvtdq2ps,VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,5Bh, vcvtudq2ps,VEX_F2_0F_W0,EVEX_REQUIRED+EVEX_VL,7Ah, \
				       vcvtps2dq,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,5Bh, vcvtps2udq,VEX_0F_W0,EVEX_REQUIRED+EVEX_VL,79h

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst_er vex_mpw,evex_f,opcode,4,dest,src
	end macro

end iterate


iterate <instr,vex_mpw,evex_f,opcode>, vcvttps2dq,VEX_F3_0F_W0,EVEX_AS_VEX+EVEX_VL,5Bh, vcvttps2udq,VEX_0F_W0,EVEX_REQUIRED+EVEX_VL,78h

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst_sae vex_mpw,evex_f,opcode,4,dest,src
	end macro

end iterate

macro vcvtph2ps? dest*,src_sae*&
	AVX_512.parse_k1z_operand@dest dest
	match src=,sae, src_sae
		AVX_512.parse_operand@src src
		AVX_512.parse_sae@src sae
	else
		AVX_512.parse_operand@src src_sae
	end match
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
			err 'invalid operand size'
		end if
		if @src.memsize = 0
			@src.memsize = @dest.size shr 1
		end if
		AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,13h,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vcvtps2ph? dest*,src*,aux*&
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	match sae=,imm, aux
		AVX_512.parse_sae@dest sae
		x86.parse_operand@aux imm
	else
		x86.parse_operand@aux aux
	end match
	if (@dest.type = 'mem' | @dest.type = 'mmreg') & @src.type = 'mmreg'
		if (@dest.type = 'mem' & @dest.size and not (@src.size shr 1)) | (@dest.type = 'mmreg' & (@src.size shr 1 - 1) and not 15 + 16 <> @dest.size)
			err 'invalid operand size'
		end if
		if @dest.memsize = 0
			@dest.memsize = @src.size shr 1
		end if
		AVX_512.store_instruction@dest @src.size,VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_VL,1Dh,@dest.mask,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mp,evex_f,opcode,msize>, vcvtsd2si,VEX_F2_0F,EVEX_AS_VEX,2Dh,8, vcvtss2si,VEX_F3_0F,EVEX_AS_VEX,2Dh,4, \
					    vcvtsd2usi,VEX_F2_0F,EVEX_REQUIRED,79h,8, vcvtss2usi,VEX_F3_0F,EVEX_REQUIRED,79h,4

	macro instr? dest*,src_er*&
		x86.parse_operand@dest dest
		match src=,er, src_er
			AVX_512.parse_operand@src src
			AVX_512.parse_er@src er,16
		else
			AVX_512.parse_operand@src src_er
		end match
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@dest.size <> 4 & @dest.size <> 8) | (@src.type = 'mem' & @src.size and not msize) | (@src.type = 'mmreg' & @src.size <> 16)
				err 'invalid operand size'
			end if
			if @dest.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX_512.store_instruction@src 16,vex_mp#_W1,evex_f,opcode,0,@dest.rm
			else
				AVX_512.store_instruction@src 16,vex_mp#_W0,evex_f,opcode,0,@dest.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mp,evex_f,opcode,msize>, vcvttsd2si,VEX_F2_0F,EVEX_AS_VEX,2Ch,8, vcvttss2si,VEX_F3_0F,EVEX_AS_VEX,2Ch,4, \
					    vcvttsd2usi,VEX_F2_0F,EVEX_REQUIRED,78h,8, vcvttss2usi,VEX_F3_0F,EVEX_REQUIRED,78h,4

	macro instr? dest*,src_sae*&
		x86.parse_operand@dest dest
		match src=,sae, src_sae
			AVX_512.parse_operand@src src
			AVX_512.parse_sae@src sae
		else
			AVX_512.parse_operand@src src_sae
		end match
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@dest.size <> 4 & @dest.size <> 8) | (@src.type = 'mem' & @src.size and not msize) | (@src.type = 'mmreg' & @src.size <> 16)
				err 'invalid operand size'
			end if
			if @dest.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX_512.store_instruction@src 16,vex_mp#_W1,evex_f,opcode,0,@dest.rm
			else
				AVX_512.store_instruction@src 16,vex_mp#_W0,evex_f,opcode,0,@dest.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vcvtsd2ss? dest*,src*,src2*&
	AVX_512.basic_instruction_er VEX_F2_0F_W0,EVEX_W1,5Ah,8,dest,src,src2
end macro

macro vcvtss2sd? dest*,src*,src2*&
	AVX_512.basic_instruction_sae VEX_F3_0F_W0,EVEX_AS_VEX,5Ah,4,dest,src,src2
end macro

iterate <instr,evex_f,opcode>, vcvtsi2sd,EVEX_AS_VEX,2Ah, vcvtusi2sd,EVEX_REQUIRED,7Bh

	macro instr? dest*,src*,src_er*&
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		match src2=,er, src_er
			AVX_512.parse_operand@src2 src2
			AVX_512.parse_er@src2 er,8
		else
			AVX_512.parse_operand@src2 src_er
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem')
			if @src.size = 0
				err ' operand size not specified'
			else if @dest.size <> 16 | @src.size <> 16 | (@src2.size <> 4 & @src2.size <> 8)
				err 'invalid operand size'
			end if
			if @src2.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX_512.store_instruction@src2 16,VEX_F2_0F_W1,evex_f,opcode,0,@dest.rm,@src.rm
			else
				AVX_512.store_instruction@src2 16,VEX_F2_0F_W0,evex_f,opcode,0,@dest.rm,@src.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,evex_f,opcode>, vcvtsi2ss,EVEX_AS_VEX,2Ah, vcvtusi2ss,EVEX_REQUIRED,7Bh

	macro instr? dest*,src*,src_er*&
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		match src2=,er, src_er
			AVX_512.parse_operand@src2 src2
			AVX_512.parse_er@src2 er,@src2.size
		else
			AVX_512.parse_operand@src2 src_er
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem')
			if @src.size = 0
				err ' operand size not specified'
			else if @dest.size <> 16 | @src.size <> 16 | (@src2.size <> 4 & @src2.size <> 8)
				err 'invalid operand size'
			end if
			if @src2.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX_512.store_instruction@src2 16,VEX_F3_0F_W1,evex_f,opcode,0,@dest.rm,@src.rm
			else
				AVX_512.store_instruction@src2 16,VEX_F3_0F_W0,evex_f,opcode,0,@dest.rm,@src.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode_rm,opcode_mr>, vmovapd,VEX_66_0F_W0,EVEX_W1+EVEX_VL,28h,29h, vmovaps,VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,28h,29h, \
						    vmovupd,VEX_66_0F_W0,EVEX_W1+EVEX_VL,10h,11h, vmovups,VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,10h,11h, \
						    vmovdqa32,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh, vmovdqa64,VEX_66_0F_W1,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh, \
						    vmovdqu32,VEX_F3_0F_W0,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh, vmovdqu64,VEX_F3_0F_W1,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode_rm,@dest.mask,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@dest @src.size,vex_mpw,evex_f,opcode_mr,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vmovd? dest*,src*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & (@src.type = 'reg' | @src.type = 'mem')
		if @dest.size <> 16 | @src.size and not 4
			err 'invalid operand size'
		end if
		@src.memsize = 4
		AVX_512.store_instruction@src 16,VEX_66_0F_W0,EVEX_AS_VEX,6Eh,0,@dest.rm
	else if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg'
		if @dest.size and not 4 | @src.size <> 16
			err 'operand sizes do not match'
		end if
		@dest.memsize = 4
		AVX_512.store_instruction@dest 16,VEX_66_0F_W0,EVEX_AS_VEX,7Eh,0,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vmovq? dest*,src*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
		if @dest.size <> 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' and @src.size and not 8)
			err 'invalid operand size'
		end if
		@src.memsize = 8
		AVX_512.store_instruction@src 16,VEX_F3_0F_W0,EVEX_W1,7Eh,0,@dest.rm
	else if @dest.type = 'mem' & @src.type = 'mmreg'
		if @dest.size and not 8 | @src.size <> 16
			err 'invalid operand size'
		end if
		@dest.memsize = 8
		AVX_512.store_instruction@dest 16,VEX_66_0F_W0,EVEX_W1,0D6h,0,@src.rm
	else if @dest.type = 'mmreg' & @src.type = 'reg'
		if @dest.size <> 16 | @src.size <> 8
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		AVX_512.store_instruction@src 16,VEX_66_0F_W1,EVEX_W1,6Eh,0,@dest.rm
	else if @dest.type = 'reg' & @src.type = 'mmreg'
		if @dest.size <> 8 | @src.size <> 16
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		AVX_512.store_instruction@dest 16,VEX_66_0F_W1,EVEX_W1,7Eh,0,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vmovddup? dest*,src*
	AVX_512.parse_k1z_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
		if @src.type = 'mem' & @dest.size = 16
			if @src.size and not 8
				err 'invalid operand size'
			end if
			@src.memsize = 8
		else
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			@src.memsize = @dest.size
		end if
		AVX_512.store_instruction@src @dest.size,VEX_F2_0F_W0,EVEX_W1+EVEX_VL,12h,@dest.mask,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,opcode>, vmovhlps,12h, vmovlhps,16h

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mmreg'
			if @dest.size <> 16
				err 'invalid operand size'
			else if @src.size <> @dest.size | @src2.size <> @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 16,VEX_0F_W0,EVEX_AS_VEX,opcode,0,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode>, vmovhpd,VEX_66_0F_W0,EVEX_W1,16h, vmovhps,VEX_0F_W0,EVEX_AS_VEX,16h, vmovlpd,VEX_66_0F_W0,EVEX_W1,12h, vmovlps,VEX_0F_W0,EVEX_AS_VEX,12h

	macro instr? dest*,src*,src2
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		match , src2
			if @dest.type = 'mem' & @src.type = 'mmreg'
				if @dest.size and not 8 | @src.size <> 16
					err 'invalid operand size'
				end if
				@dest.memsize = 8
				AVX_512.store_instruction@dest 16,vex_mpw,evex_f,opcode+1,0,@src.rm
			else
				err 'invalid combination of operands'
			end if
		else
			AVX_512.parse_operand@src2 src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
				if @dest.size <> 16 | @src.size <> 16 | @src2.size and not 8
					err 'invalid operand size'
				end if
				@src2.memsize = 8
				AVX_512.store_instruction@src2 16,vex_mpw,evex_f,opcode,0,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end match
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode>, vmovntdq,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,0E7h, vmovntpd,VEX_66_0F_W0,EVEX_W1+EVEX_VL,2Bh, vmovntps,VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,2Bh

	macro instr? dest*,src*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@dest @src.size,vex_mpw,evex_f,opcode,0,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vmovntdqa? dest*,src*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & @src.type = 'mem'
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,2Ah,0,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw>, vmovmskpd,VEX_66_0F_W0, vmovmskps,VEX_0F_W0

	macro instr? dest*,src*
		x86.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'reg' & @src.type = 'mmreg'
			if @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src @src.size,vex_mpw,EVEX_FORBIDDEN,50h,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,msize>, vmovsd,VEX_F2_0F_W0,EVEX_W1,8, vmovss,VEX_F3_0F_W0,EVEX_AS_VEX,4

	macro instr? dest*,src*,src2
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		match , src2
			if @dest.type = 'mmreg' & @src.type = 'mem'
				if @dest.size <> 16 | @src.size and not msize
					err 'invalid operand size'
				end if
				@src.memsize = msize
				AVX_512.store_instruction@src 16,vex_mpw,evex_f,10h,@dest.mask,@dest.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg'
				if @dest.size and not msize | @src.size <> 16
					err 'invalid operand size'
				end if
				@dest.memsize = msize
				AVX_512.store_instruction@dest 16,vex_mpw,evex_f,11h,@dest.mask,@src.rm
			else
				err 'invalid combination of operands'
			end if
		else
			AVX_512.parse_operand@src2 src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mmreg'
				if @dest.size <> 16 | @src.size <> 16 | @src2.size <> 16
					err 'invalid operand size'
				end if
				AVX_512.store_instruction@src2 16,vex_mpw,evex_f,10h,@dest.mask,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end match
	end macro

end iterate

macro vmovshdup? dest*,src*
	AVX_512.single_source_instruction VEX_F3_0F_W0,EVEX_AS_VEX+EVEX_VL,16h,0,dest,src
end macro

macro vmovsldup? dest*,src*
	AVX_512.single_source_instruction VEX_F3_0F_W0,EVEX_AS_VEX+EVEX_VL,12h,0,dest,src
end macro

iterate <instr,unit,evex_f,opcode_rrm,opcode_rri>, vpermilps,4,EVEX_AS_VEX+EVEX_VL,0Ch,4, vpermilpd,8,EVEX_W1+EVEX_VL,0Dh,5

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_bcst_operand@src src,unit
		AVX_512.parse_bcst_operand@src2 src2,unit
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @dest.size,VEX_66_0F38_W0,evex_f,opcode_rrm,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @src2.type = 'imm'
			if @src2.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,VEX_66_0F3A_W0,evex_f,opcode_rri,@dest.mask,@dest.rm,,1,@src2.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpaddd,0FEh, vpsubd,0FAh, vpunpckhdq,6Ah, vpunpckldq,62h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpaddq,0D4h, vpmuludq,0F4h, vpsubq,0FBh, vpunpckhqdq,6Dh, vpunpcklqdq,6Ch

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_W1+EVEX_VL+EVEX_VL_AVX2,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpandd,0DBh, vpandnd,0DFh, vpord,0EBh, vpxord,0EFh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpandq,0DBh, vpandnq,0DFh, vporq,0EBh, vpxorq,0EFh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpmaxsd,3Dh, vpmaxud,3Fh, vpminsd,39h, vpminud,3Bh, vpmulld,40h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpmuldq,28h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_W1+EVEX_VL+EVEX_VL_AVX2,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpmuldq,28h, vpmaxsq,3Dh, vpmaxuq,3Fh, vpminsq,39h, vpminuq,3Bh, vpmullq,40h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpabsd,1Eh

	macro instr? dest*,src*
		AVX_512.single_source_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,opcode,4,dest,src
	end macro

end iterate

iterate <instr,opcode>, vpabsq,1Fh

	macro instr? dest*,src*
		AVX_512.single_source_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src
	end macro

end iterate

iterate <instr,vex_mpw>, vpshufd,VEX_66_0F_W0

	macro instr? dest*,src*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,70h,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpsllvd,47h, vpsrlvd,45h, vpsravd,46h

	macro instr? dest*,src*,src2*
		require AVX2+
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,evex_f,opcode>, vpsllvq,EVEX_AS_VEX+EVEX_VL,47h, vpsrlvq,EVEX_AS_VEX+EVEX_VL,45h, vpsravq,EVEX_REQUIRED+EVEX_VL,46h

	macro instr? dest*,src*,src2*
		require AVX2+
		AVX_512.basic_instruction_bcst VEX_66_0F38_W1,evex_f,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpermi2d,4,VEX_66_0F38_W0,76h, vpermi2q,8,VEX_66_0F38_W1,76h, \
				     vpermt2d,4,VEX_66_0F38_W0,7Eh, vpermt2q,8,VEX_66_0F38_W1,7Eh, \
				     vprorvd,4,VEX_66_0F38_W0,14h, vprorvq,8,VEX_66_0F38_W1,14h, \
				     vprolvd,4,VEX_66_0F38_W0,15h, vprolvq,8,VEX_66_0F38_W1,15h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src,src2
	end macro

end iterate

iterate <instr,unit,vex_mpw,postbyte>, vprord,4,VEX_66_0F_W0,0, vprorq,8,VEX_66_0F_W1,0, vprold,4,VEX_66_0F_W0,1, vprolq,8,VEX_66_0F_W1,1

	macro instr? dest*,src*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_bcst_operand@src src,unit
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @src.size and not @dest.size | @aux.size and not 1
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,72h,@dest.mask,postbyte,@dest.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode_rrm,opcode,postbyte>, vpslld,0F2h,72h,6, vpsrad,0E2h,72h,4, vpsrld,0D2h,72h,2

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src2 src2
		if @src2.type = 'imm'
			AVX_512.parse_bcst_operand@src src,4
		else
			AVX_512.parse_operand@src src
		end if
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			@src2.memsize = 16
			if @src2.size and not @src2.memsize
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,opcode_rrm,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @src2.type = 'imm'
			if @src2.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @src.type = 'mem'
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL+EVEX_VL_AVX2,opcode,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			else
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,opcode,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode_rrm,opcode,postbyte>, vpsllq,0F3h,73h,6, vpsraq,0E2h,72h,4, vpsrlq,0D3h,73h,2

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src2 src2
		if @src2.type = 'imm'
			AVX_512.parse_bcst_operand@src src,8
		else
			AVX_512.parse_operand@src src
		end if
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			@src2.memsize = 16
			if @src2.size and not @src2.memsize
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if `instr = 'vpsraq'
				AVX_512.store_instruction@src2 @dest.size,VEX_66_0F_W1,EVEX_REQUIRED+EVEX_VL,opcode_rrm,@dest.mask,@dest.rm,@src.rm
			else
				AVX_512.store_instruction@src2 @dest.size,VEX_66_0F_W0,EVEX_W1+EVEX_VL+EVEX_VL_AVX2,opcode_rrm,@dest.mask,@dest.rm,@src.rm
			end if
		else if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @src2.type = 'imm'
			if @src2.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			if @src.type = 'mem' | `instr = 'vpsraq'
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W1,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			else
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W0,EVEX_W1+EVEX_VL+EVEX_VL_AVX2,opcode,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpcmpeqd,76h, vpcmpgtd,66h

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_bcst_operand@src2 src2,4
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,VEX_66_0F_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,opcode,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpcmpeqq,29h, vpcmpgtq,37h

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_bcst_operand@src2 src2,8
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,VEX_66_0F38_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,opcode,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vptestnmd,4,VEX_F3_0F38_W0,27h, vptestnmq,8,VEX_F3_0F38_W1,27h, vptestmd,4,VEX_66_0F38_W0,27h, vptestmq,8,VEX_66_0F38_W1,27h

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_bcst_operand@src2 src2,unit
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpcmpd,4,VEX_66_0F3A_W0,1Fh, vpcmpud,4,VEX_66_0F3A_W0,1Eh, vpcmpq,8,VEX_66_0F3A_W1,1Fh, vpcmpuq,8,VEX_66_0F3A_W1,1Eh

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_bcst_operand@src2 src2,unit
		x86.parse_operand@aux aux
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @src2.size and not @src.size | @aux.size and not 1
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode,msize>, vpmovsxbd,21h,4, vpmovsxbq,22h,2, vpmovsxwd,23h,8, vpmovsxwq,24h,4, vpmovsxdq,25h,8, \
			      vpmovzxbd,31h,4, vpmovzxbq,32h,2, vpmovzxwd,33h,8, vpmovzxwq,34h,4, vpmovzxdq,35h,8

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			@src.memsize = msize * (@dest.size shr 4)
			if (@src.type = 'mmreg' & @src.size <> (@src.memsize-1) and not 15 + 16) | (@src.type = 'mem' & @src.size and not @src.memsize)
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpermq,0, vpermpd,1

	macro instr? dest*,src*,aux*
		require AVX2+
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_bcst_operand@src src,8
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @dest.size < 32 | @aux.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,VEX_66_0F3A_W1,EVEX_AS_VEX+EVEX_VL,opcode,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpermd,36h, vpermps,16h

	macro instr? dest*,src*,src2*
		require AVX2+
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_bcst_operand@src2 src2,4
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @dest.size < 32
				err 'invalid operand size'
			else if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

	macro vperm2f128? dest*,src*,src2*,imm*
		AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,6,32,dest,src,src2,imm
	end macro


iterate <instr,lcode>, vfmaddsub,6, vfmsubadd,7, vfmaddsub,8, vfmsub,0Ah, vfnmadd,0Ch, vfnmsub,0Eh

	iterate <order,hcode>, 132,90h, 213,0A0h, 231,0B0h

		macro instr#order#pd? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst_er VEX_66_0F38_W1,EVEX_AS_VEX+EVEX_VL,hcode+lcode,8,dest,src,src2
		end macro

		macro instr#order#ps? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst_er VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,hcode+lcode,4,dest,src,src2
		end macro

		if lcode > 7

			macro instr#order#sd? dest*,src*,src2*&
				AVX_512.basic_instruction_er VEX_66_0F38_W1,EVEX_AS_VEX,hcode+lcode+1,8,dest,src,src2
			end macro

			macro instr#order#ss? dest*,src*,src2*&
				AVX_512.basic_instruction_er VEX_66_0F38_W0,EVEX_AS_VEX,hcode+lcode+1,4,dest,src,src2
			end macro

		end if

	end iterate

end iterate

iterate <instr,unit,vex_mpw,opcode>, valignd,4,VEX_66_0F3A_W0,3, vpternlogd,4,VEX_66_0F3A_W0,25h, vpternlogq,8,VEX_66_0F3A_W1,25h

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_imm8 vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src,src2,aux
	end macro

end iterate

iterate <instr,opcode>, valignq,3

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2,aux
	end macro

end iterate

iterate <instr,opcode>, vblendmps,65h, vpblendmd,64h

	macro instr? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vblendmpd,65h, vpblendmq,64h

	macro instr? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vrcp14ps,4,VEX_66_0F38_W0,4Ch, vrcp14pd,8,VEX_66_0F38_W1,4Ch, vrsqrt14ps,4,VEX_66_0F38_W0,4Eh, vrsqrt14pd,8,VEX_66_0F38_W1,4Eh

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vrcp14ss,4,VEX_66_0F38_W0,4Dh, vrcp14sd,8,VEX_66_0F38_W1,4Dh, vrsqrt14ss,4,VEX_66_0F38_W0,4Fh, vrsqrt14sd,8,VEX_66_0F38_W1,4Fh

	macro instr? dest*,src*&
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcompressps,VEX_66_0F_W0,8Ah, vcompresspd,VEX_66_0F_W1,8Ah, vpcompressd,VEX_66_0F38_W0,8Bh, vpcompressq,VEX_66_0F38_W1,8Bh

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@dest @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vexpandps,VEX_66_0F38_W0,88h, vexpandpd,VEX_66_0F38_W1,88h, vpexpandd,VEX_66_0F38_W0,89h, vpexpandq,VEX_66_0F38_W1,89h

	macro instr? dest*,src*
		AVX_512.single_source_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src
	end macro

end iterate

iterate <instr,opcode>, fixupimm,54h

	macro v#instr#pd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2,aux
	end macro

	macro v#instr#ps? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_VL,opcode,4,dest,src,src2,aux
	end macro

	macro v#instr#sd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED,opcode+1,8,dest,src,src2,aux
	end macro

	macro v#instr#ss? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED,opcode+1,4,dest,src,src2,aux
	end macro

end iterate

iterate <instr,opcode>, getexp,42h

	macro v#instr#pd? dest*,src*&
		AVX_512.single_source_instruction_bcst_sae VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src
	end macro

	macro v#instr#ps? dest*,src*&
		AVX_512.single_source_instruction_bcst_sae VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,4,dest,src
	end macro

	macro v#instr#sd? dest*,src*,src2*&
		AVX_512.basic_instruction_sae VEX_66_0F38_W1,EVEX_REQUIRED,opcode+1,8,dest,src,src2
	end macro

	macro v#instr#ss? dest*,src*,src2*&
		AVX_512.basic_instruction_sae VEX_66_0F38_W0,EVEX_REQUIRED,opcode+1,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode_ps,opcode_pd,opcode_ss,opcode_sd>, getmant,26h,26h,27h,27h, rndscale,8,9,0Ah,0Bh

	macro v#instr#pd? dest*,src*,aux*&
		AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_VL,opcode_pd,8,dest,src,aux
	end macro

	macro v#instr#ps? dest*,src*,aux*&
		AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_VL,opcode_ps,4,dest,src,aux
	end macro

	macro v#instr#sd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED,opcode_sd,8,dest,src,src2,aux
	end macro

	macro v#instr#ss? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED,opcode_ss,4,dest,src,src2,aux
	end macro

end iterate

iterate <instr,unit,vex_mpw>, vscalefpd,8,VEX_66_0F38_W1, vscalefps,4,VEX_66_0F38_W0

	macro instr? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst_er vex_mpw,EVEX_REQUIRED+EVEX_VL,2Ch,unit,dest,src,src2
	end macro

end iterate

iterate <instr,unit,vex_mpw>, vscalefsd,8,VEX_66_0F38_W1, vscalefss,4,VEX_66_0F38_W0

	macro instr? dest*,src*,src2*&
		AVX_512.basic_instruction_er vex_mpw,EVEX_REQUIRED,2Dh,unit,dest,src,src2
	end macro

end iterate

iterate <instr,ratio,opcode>, vpmovusdb,4,11h, vpmovsdb,4,21h, vpmovdb,4,31h, \
			      vpmovusqb,8,12h, vpmovsqb,8,22h, vpmovqb,8,32h, \
			      vpmovusdw,2,13h, vpmovsdw,2,23h, vpmovdw,2,33h, \
			      vpmovusqw,4,14h, vpmovsqw,4,24h, vpmovqw,4,34h, \
			      vpmovusqd,2,15h, vpmovsqd,2,25h, vpmovqd,2,35h

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
			@dest.memsize = @src.size / ratio
			if (@dest.type = 'mmreg' & @dest.size <> (@dest.memsize-1) and not 15 + 16) | (@dest.type = 'mem' & @dest.size and not @dest.memsize)
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@dest @src.size,VEX_F3_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode,asize>, vpgatherdd,90h,4, vpgatherqd,91h,8, vgatherdps,92h,4, vgatherqps,93h,8

	macro instr? dest*,src*,aux
		match , aux
			AVX_512.parse_k1_operand@dest dest
			AVX_512.parse_vsib_operand@src src
			if @dest.type = 'mmreg' & @dest.mask & @src.type = 'mem'
				if @src.size and not 4 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize)
					err 'invalid operand size'
				else if @dest.rm = @src.index
					err 'disallowed combination of registers'
				end if
				@src.memsize = 4
				AVX_512.store_instruction@src @src.visize,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.index and 10000b
			else
				err 'invalid combination of operands'
			end if
		else
			require AVX2+
			AVX_512.parse_operand@dest dest
			AVX_512.parse_vsib_operand@src src
			AVX_512.parse_operand@aux aux
			if @dest.type = 'mmreg' & @src.type = 'mem' & @aux.type = 'mmreg'
				if @src.size and not 4 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize)
					err 'invalid operand size'
				else if @aux.size <> @dest.size
					err 'operand sizes do not match'
				else if @dest.rm = @aux.rm | @dest.rm = @src.index | @aux.rm = @src.index
					err 'disallowed combination of registers'
				end if
				AVX.store_instruction@src @src.visize,VEX_66_0F38_W0,opcode,@dest.rm,@aux.rm
			else
				err 'invalid combination of operands'
			end if
		end match
	end macro

end iterate

iterate <instr,opcode,asize>, vpgatherdq,90h,4, vpgatherqq,91h,8, vgatherdpd,92h,4, vgatherqpd,93h,8

	macro instr? dest*,src*,aux
		match , aux
			AVX_512.parse_k1_operand@dest dest
			AVX_512.parse_vsib_operand@src src
			if @dest.type = 'mmreg' & @dest.mask & @src.type = 'mem'
				if @src.size and not 8 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize * 2) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize * 2)
					err 'invalid operand size'
				else if @dest.rm = @src.index
					err 'disallowed combination of registers'
				end if
				@src.memsize = 8
				AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@dest.rm,@src.index and 10000b
			else
				err 'invalid combination of operands'
			end if
		else
			require AVX2+
			AVX_512.parse_operand@dest dest
			AVX_512.parse_vsib_operand@src src
			AVX_512.parse_operand@aux aux
			if @dest.type = 'mmreg' & @src.type = 'mem' & @aux.type = 'mmreg'
				if @src.size and not 8 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize * 2) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize * 2)
					err 'invalid operand size'
				else if @aux.size <> @dest.size
					err 'operand sizes do not match'
				else if @dest.rm = @aux.rm | @dest.rm = @src.index | @aux.rm = @src.index
					err 'disallowed combination of registers'
				end if
				AVX.store_instruction@src @dest.size,VEX_66_0F38_W1,opcode,@dest.rm,@aux.rm
			else
				err 'invalid combination of operands'
			end if
		end match
	end macro

end iterate

iterate <instr,opcode,asize>, vpscatterdd,0A0h,4, vpscatterqd,0A1h,8, vscatterdps,0A2h,4, vscatterqps,0A3h,8

	macro instr? dest*,src*
		AVX_512.parse_k1_vsib_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mem' & @dest.mask & @src.type = 'mmreg'
			if @dest.size and not 4 | (@src.size > 16 & @src.size * (asize shr 2) > @dest.visize) | (@dest.visize > 16 & @src.size * (asize shr 2) < @dest.visize)
				err 'invalid operand size'
			else if @src.rm = @dest.index
				err 'disallowed combination of registers'
			end if
			@dest.memsize = 4
			AVX_512.store_instruction@dest @dest.visize,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@src.rm,@dest.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode,asize>, vpscatterdq,0A0h,4, vpscatterqq,0A1h,8, vscatterdpd,0A2h,4, vscatterqpd,0A3h,8

	macro instr? dest*,src*
		AVX_512.parse_k1_vsib_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mem' & @dest.mask & @src.type = 'mmreg'
			if @dest.size and not 8 | (@src.size > 16 & @src.size * (asize shr 2) > @dest.visize * 2) | (@dest.visize > 16 & @src.size * (asize shr 2) < @dest.visize * 2)
				err 'invalid operand size'
			else if @src.rm = @dest.index
				err 'disallowed combination of registers'
			end if
			@dest.memsize = 8
			AVX_512.store_instruction@dest @src.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@src.rm,@dest.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <t,vex_mpw,msize>, d,VEX_66_0F_W1,4, q,VEX_0F_W1,8

	iterate <instr,opcode>, kand,41h, kandn,42h, knot,44h, kor,45h, kxnor,46h, kxor,47h, kadd,4Ah

		macro instr#t? dest*,src*,src2*
			require AVX512BW
			AVX_512.parse_operand@dest dest
			AVX_512.parse_operand@src src
			AVX_512.parse_operand@src2 src2
			if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
				AVX.store_instruction@src2 32,vex_mpw,opcode,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, knot,44h, kortest,98h, ktest,99h

		macro instr#t? dest*,src*
			require AVX512BW
			AVX_512.parse_operand@dest dest
			AVX_512.parse_operand@src src
			if @dest.type = 'maskreg' & @src.type = 'maskreg'
				AVX.store_instruction@src 16,vex_mpw,opcode,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro kmov#t? dest*,src*
		require AVX512BW
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'maskreg' & (@src.type = 'maskreg' | @src.type = 'mem')
			if @src.type = 'mem' & @src.size and not msize
				err 'invalid operand size'
			end if
			AVX.store_instruction@src 16,vex_mpw,90h,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'maskreg'
			if @dest.size and not msize
				err 'invalid operand size'
			end if
			AVX.store_instruction@dest 16,vex_mpw,91h,@src.rm
		else if @dest.type = 'maskreg' & @src.type = 'reg'
			if (msize < 8 & @src.size <> 4) | (msize = 8 & @src.size <> 8)
				err 'invalid operand size'
			else if msize = 8 & x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction@src 16,vex_mpw,92h,@dest.rm
		else if @dest.type = 'reg' & @src.type = 'maskreg'
			if (msize < 8 & @dest.size <> 4) | (msize = 8 & @dest.size <> 8)
				err 'invalid operand size'
			else if msize = 8 & x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction@src 16,vex_mpw,93h,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, kshiftrd,VEX_66_0F3A_W0,31h, kshiftrq,VEX_66_0F3A_W1,31h, \
				kshiftld,VEX_66_0F3A_W0,33h, kshiftlq,VEX_66_0F3A_W1,33h

	macro instr? dest*,src*,aux*
		require AVX512BW
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction@src 16,vex_mpw,opcode,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw>, kunpckwd,VEX_0F_W0, kunpckdq,VEX_0F_W1

	macro instr? dest*,src*,src2*
		require AVX512BW
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
			AVX.store_instruction@src2 32,vex_mpw,4Bh,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode_rm,opcode_mr>, vmovdqu8,VEX_F2_0F_W0,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh, vmovdqu16,VEX_F2_0F_W1,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh

	macro instr? dest*,src*
		require AVX512BW
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,evex_f,opcode_rm,@dest.mask,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@dest @src.size,vex_mpw,evex_f,opcode_mr,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpabsb,1Ch, vpabsw,1Dh

	macro instr? dest*,src*
		AVX_512.single_source_instruction VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,0,dest,src
	end macro

end iterate

iterate <instr,opcode>, vpacksswb,63h, vpackuswb,67h, vpaddb,0FCh, vpaddw,0FDh, vpaddsb,0ECh, vpaddsw,0EDh, vpaddusb,0DCh, vpaddusw,0DDh, vpavgb,0E0h, vpavgw,0E3h, \
			vpmaddwd,0F5h, vpmaxsw,0EEh, vpmaxub,0DEh, vpminsw,0EAh, vpminub,0DAh, vpmulhuw,0E4h, vpmulhw,0E5h, vpmullw,0D5h, \
			vpsadbw,0F6h, vpsubb,0F8h, vpsubw,0F9h, vpsubsb,0E8h, vpsubsw,0E9h, vpsubusb,0D8h, vpsubusw,0D9h, \
			vpunpckhbw,68h, vpunpckhwd,69h, vpunpcklbw,60h, vpunpcklwd,61h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpackssdw,6Bh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpackusdw,2Bh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpalignr,0Fh

	macro instr? dest*,src*,src2*,imm*
		AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,0,dest,src,src2,imm
	end macro

end iterate

iterate <instr,opcode>, vpmaddubsw,4, vpmaxsb,3Ch, vpmaxuw,3Eh, vpminsb,38h, vpminuw,3Ah, vpmulhrsw,0Bh, vpshufb,0

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpcmpeqb,74h, vpcmpeqw,75h, vpcmpgtb,64h, vpcmpgtw,65h

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,VEX_66_0F_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,opcode,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vpextrb? dest*,src*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if (@dest.type = 'reg' & @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | (@dest.type = 'mem' & @dest.size and not 1) | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.memsize = 1
		AVX_512.store_instruction@dest 16,VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_BW,14h,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpextrw? dest*,src*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if @dest.type = 'reg' & @src.type = 'mmreg' & @aux.type = 'imm'
		if @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8) | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX_512.store_instruction@src 16,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_BW,0C5h,0,@dest.rm,,1,@aux.imm
	else if @dest.type = 'mem' & @src.type = 'mmreg' & @aux.type = 'imm'
		if  @dest.size and not 2 | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.memsize = 2
		AVX_512.store_instruction@dest 16,VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_BW,15h,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw,opcode,msize>, vpinsrb,VEX_66_0F3A_W0,20h,1, vpinsrw,VEX_66_0F_W0,0C4h,2

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'reg' & @src2.size <> 4) | (@src2.type = 'mem' & @src2.size and not msize) | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src2.memsize = msize
			AVX_512.store_instruction@src2 16,vex_mpw,EVEX_AS_VEX+EVEX_BW,opcode,0,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode,msize>, vpmovsxbw,20h,8, vpmovzxbw,30h,8

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			@src.memsize = msize * (@dest.size shr 4)
			if (@src.type = 'mmreg' & @src.size <> (@src.memsize-1) and not 15 + 16) | (@src.type = 'mem' & @src.size and not @src.memsize)
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw>, vpshufhw,VEX_F3_0F_W0, vpshuflw,VEX_F2_0F_W0

	macro instr? dest*,src*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,70h,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,postbyte>, vpslldq,7, vpsrldq,3

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @src.type = 'mem'
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,73h,0,postbyte,@dest.rm,1,@aux.imm
			else
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,73h,0,postbyte,@dest.rm,1,@aux.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode_rrm,opcode,postbyte>, vpsllw,0F1h,71h,6, vpsraw,0E1h,71h,4, vpsrlw,0D1h,71h,2

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			@src2.memsize = 16
			if @src2.size and not @src2.memsize
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode_rrm,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @src2.type = 'imm'
			if @src2.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @src.type = 'mem'
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			else
				AVX_512.store_instruction@src @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_VL_AVX2+EVEX_BW,opcode,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vdbpsadbw,VEX_66_0F3A_W0,42h

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_imm8 vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,0,dest,src,src2,aux
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpblendmb,VEX_66_0F38_W0,66h, vpblendmw,VEX_66_0F38_W1,66h

	macro instr? dest*,src*,src2*&
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode,opcode_g,msize>, vpbroadcastb,78h,7Ah,1, vpbroadcastw,79h,7Bh,2

	macro instr? dest*,src*
		require AVX2+
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if (@src.type='mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not msize)
				err 'invalid operand size'
			end if
			@src.memsize = msize
			AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL+EVEX_BW,opcode,@dest.mask,@dest.rm
		else if @dest.type = 'mmreg' & @src.type = 'reg'
			if @src.size <> msize & (@src.size <> 4 | msize = 8)
				err 'invalid operand size'
			end if
			@src.memsize = msize
			if msize = 8
				AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode_g,@dest.mask,@dest.rm
			else
				AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode_g,@dest.mask,@dest.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpcmpb,VEX_66_0F3A_W0,3Fh, vpcmpub,VEX_66_0F3A_W0,3Eh, vpcmpw,VEX_66_0F3A_W1,3Fh, vpcmpuw,VEX_66_0F3A_W1,3Eh

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		x86.parse_operand@aux aux
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @src2.size and not @src.size | @aux.size and not 1
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpermw,VEX_66_0F38_W1,8Dh, vpermi2w,VEX_66_0F38_W1,75h, vpermt2w,VEX_66_0F38_W1,7Dh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpmovb2m,VEX_F3_0F38_W0,29h, vpmovw2m,VEX_F3_0F38_W1,29h

	macro instr? dest*,src*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'maskreg' & @src.type = 'mmreg'
			AVX_512.store_instruction@src @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpmovm2b,VEX_F3_0F38_W0,28h, vpmovm2w,VEX_F3_0F38_W1,28h

	macro instr? dest*,src*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & @src.type = 'maskreg'
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,ratio,opcode>, vpmovuswb,2,10h, vpmovswb,2,20h, vpmovwb,2,30h

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
			@dest.memsize = @src.size / ratio
			if (@dest.type = 'mmreg' & @dest.size <> (@dest.memsize-1) and not 15 + 16) | (@dest.type = 'mem' & @dest.size and not @dest.memsize)
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@dest @src.size,VEX_F3_0F38_W0,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpsllvw,12h, vpsrlvw,10h, vpsravw,11h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vptestnmb,VEX_F3_0F38_W0,26h, vptestnmw,VEX_F3_0F38_W1,26h, vptestmb,VEX_66_0F38_W0,26h, vptestmw,VEX_66_0F38_W1,26h

	macro instr? dest*,src*,src2*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_BW,opcode,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vtestps,0Eh, vtestpd,0Fh, vptest,17h

	macro instr? dest*,src*
		AVX_512.single_source_instruction VEX_66_0F38_W0,EVEX_FORBIDDEN,opcode,0,dest,src
	end macro

end iterate

iterate <instr,opcode>, kandb,41h, kandnb,42h, knotb,44h, korb,45h, kxnorb,46h, kxorb,47h, kaddb,4Ah

	macro instr? dest*,src*,src2*
		require AVX512DQ
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
			AVX.store_instruction@src2 32,VEX_66_0F_W0,opcode,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, knotb,44h, kortestb,98h, ktestb,99h

	macro instr? dest*,src*
		require AVX512DQ
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'maskreg' & @src.type = 'maskreg'
			AVX.store_instruction@src 16,VEX_66_0F_W0,opcode,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro kmovb? dest*,src*
	require AVX512DQ
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'maskreg' & (@src.type = 'maskreg' | @src.type = 'mem')
		if @src.type = 'mem' & @src.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction@src 16,VEX_66_0F_W0,90h,@dest.rm
	else if @dest.type = 'mem' & @src.type = 'maskreg'
		if @dest.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction@dest 16,VEX_66_0F_W0,91h,@src.rm
	else if @dest.type = 'maskreg' & @src.type = 'reg'
		if @src.size <> 4
			err 'invalid operand size'
		end if
		AVX.store_instruction@src 16,VEX_66_0F_W0,92h,@dest.rm
	else if @dest.type = 'reg' & @src.type = 'maskreg'
		if @dest.size <> 4
			err 'invalid operand size'
		end if
		AVX.store_instruction@src 16,VEX_66_0F_W0,93h,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro kaddw? dest*,src*,src2*
	require AVX512DQ
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
		AVX.store_instruction@src2 32,VEX_0F_W0,opcode,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro ktestw? dest*,src*
	require AVX512DQ
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'maskreg' & @src.type = 'maskreg'
		AVX.store_instruction@src 16,VEX_0F_W0,opcode,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw,opcode>, kshiftrb,VEX_66_0F3A_W0,30h, kshiftlb,VEX_66_0F3A_W0,32h

	macro instr? dest*,src*,aux*
		require AVX512DQ
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction@src 16,vex_mpw,opcode,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, and,54h, andn,55h, or,56h, xor,57h

	macro v#instr#pd? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_W1+EVEX_VL+EVEX_DQ,opcode,8,dest,src,src2
	end macro

	macro v#instr#ps? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_0F_W0,EVEX_AS_VEX+EVEX_VL+EVEX_DQ,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vbroadcastf32x2,19h, vbroadcasti32x2,59h

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @dest.size = 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 8)
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction@src @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode,msize>, vbroadcastf32x8,VEX_66_0F38_W0,1Bh,32, vbroadcastf64x2,VEX_66_0F38_W1,1Ah,16, \
				      vbroadcasti32x8,VEX_66_0F38_W0,5Bh,32, vbroadcasti64x2,VEX_66_0F38_W1,5Ah,16

	macro instr? dest*,src*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @dest.size <= msize | @src.size and not msize
				err 'invalid operand size'
			end if
			@src.memsize = msize
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvtpd2qq,VEX_66_0F_W1,7Bh, vcvtpd2uqq,VEX_66_0F_W1,79h, \
				vcvtqq2pd,VEX_F3_0F_W1,0E6h, vcvtuqq2pd,VEX_F3_0F_W1,7Ah

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst_er vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,8,dest,src
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvttpd2qq,VEX_66_0F_W1,7Ah, vcvttpd2uqq,VEX_66_0F_W1,78h

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst_sae vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,8,dest,src
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvtps2qq,VEX_66_0F_W0,7Bh, vcvtps2uqq,VEX_66_0F_W0,79h

	macro instr? dest*,src_er*&
		AVX_512.parse_k1z_operand@dest dest
		match src=,er, src_er
			AVX_512.parse_operand@src src
			AVX_512.parse_er@src er,32
		else
			AVX_512.parse_bcst_operand@src src_er,4
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
				err 'invalid operand size'
			end if
			if @src.memsize = 0
				@src.memsize = @dest.size shr 1
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvtqq2ps,VEX_0F_W1,5Bh, vcvtuqq2ps,VEX_F2_0F_W1,7Ah

	macro instr? dest*,src_er*&
		AVX_512.parse_k1z_operand@dest dest
		match src=,er, src_er
			AVX_512.parse_operand@src src
			AVX_512.parse_er@src er
		else
			AVX_512.parse_bcst_operand@src src_er,8
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size = 0
				if @dest.size = 16
					err 'operand size not specified'
				else
					@src.size = 64
				end if
			end if
			if (@src.size shr 1 - 1) and not 15 + 16 <> @dest.size | @src.size > 64
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvttps2qq,VEX_66_0F_W0,7Ah, vcvttps2uqq,VEX_66_0F_W0,78h

	macro instr? dest*,src_sae*&
		AVX_512.parse_k1z_operand@dest dest
		match src=,sae, src_sae
			AVX_512.parse_operand@src src
			AVX_512.parse_sae @src,sae
		else
			AVX_512.parse_bcst_operand@src src_sae,4
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
				err 'invalid operand size'
			end if
			if @src.memsize = 0
				@src.memsize = @dest.size shr 1
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vcvtph2ps? dest*,src*
	require F16C
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
		if (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size*2 <> @dest.size)
			err 'invalid operand size'
		end if
		AVX.store_instruction@src @dest.size,VEX_66_0F38_W0,13h,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vcvtps2ph? dest*,src*,round*
	require F16C
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux round
	if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if (@dest.type = 'mmreg' & @dest.size <> 16) | (@dest.type = 'mem' & @dest.size*2 <> @src.size) | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction@dest @src.size,VEX_66_0F3A_W0,1Dh,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw,opcode,msize>, vextractf32x8,VEX_66_0F3A_W0,1Bh,32, vextractf64x2,VEX_66_0F3A_W1,19h,16, \
				      vextracti32x8,VEX_66_0F3A_W0,3Bh,32, vextracti64x2,VEX_66_0F3A_W1,39h,16

	macro instr? dest*,src*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not msize | @src.size <= msize | @aux.size and not 1
				err 'invalid operand size'
			end if
			@dest.memsize = msize
			AVX_512.store_instruction@dest @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,@dest.mask,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode,msize>, vinsertf32x8,VEX_66_0F3A_W0,1Ah,32, vinsertf64x2,VEX_66_0F3A_W1,18h,16, \
				      vinserti32x8,VEX_66_0F3A_W0,3Ah,32, vinserti64x2,VEX_66_0F3A_W1,38h,16

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		x86.parse_operand@aux aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <= msize | @src.size <= msize | @src2.size and not msize | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src2.memsize = msize
			AVX_512.store_instruction@src2 @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw>, vfpclasspd,8,VEX_66_0F3A_W1, vfpclassps,4,VEX_66_0F3A_W0

	macro instr? dest*,src*,aux*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_bcst_operand@src src,unit
		x86.parse_operand@aux aux
		if @dest.type = 'maskreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @src.size = 0
				err 'operand size not specified'
			else if (@src.size <> 16 & @src.size <> 32 & @src.size <> 64) | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,66h,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw>, vfpclasssd,8,VEX_66_0F3A_W1, vfpclassss,4,VEX_66_0F3A_W0

	macro instr? dest*,src*,aux*
		AVX_512.parse_k1_operand@dest dest
		AVX_512.parse_operand@src src
		x86.parse_operand@aux aux
		if @dest.type = 'maskreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if (@src.type = 'mem' & @src.size and not unit) | (@src.type = 'mmreg' & @src.size <> 16) | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src.memsize = 16
			AVX_512.store_instruction@src @src.size,vex_mpw,EVEX_REQUIRED+EVEX_DQ,67h,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vpextrd? dest*,src*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if (@dest.type = 'reg' & @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | (@dest.type = 'mem' & @dest.size and not 4) | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.memsize = 4
		AVX_512.store_instruction@dest 16,VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_DQ,16h,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpinsrd? dest*,src*,src2*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
		if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'reg' & @src2.size <> 4) | (@src2.type = 'mem' & @src2.size and not 4) | @aux.size and not 1
			err 'invalid operand size'
		end if
		@src2.memsize = 4
		AVX_512.store_instruction@src2 16,VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_DQ,22h,0,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpextrq? dest*,src*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if @dest.size and not 8 | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		@dest.memsize = 8
		AVX_512.store_instruction@dest 16,VEX_66_0F3A_W1,EVEX_AS_VEX+EVEX_DQ,16h,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpinsrq? dest*,src*,src2*,aux*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@src2 src2
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
		if @dest.size <> 16 | @src.size <> 16 | @src2.size and not 8 | @aux.size and not 1
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		@src2.memsize = 8
		AVX_512.store_instruction@src2 16,VEX_66_0F3A_W1,EVEX_AS_VEX+EVEX_DQ,22h,0,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpmullq? dest*,src*,src2*
	AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,40h,8,dest,src,src2
end macro

iterate <instr,vex_mpw,opcode>, vpmovm2d,VEX_F3_0F38_W0,38h, vpmovm2q,VEX_F3_0F38_W1,38h

	macro instr? dest*,src*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & @src.type = 'maskreg'
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpmovd2m,VEX_F3_0F38_W0,39h, vpmovq2m,VEX_F3_0F38_W1,39h

	macro instr? dest*,src*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'maskreg' & @src.type = 'mmreg'
			AVX_512.store_instruction@src @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, range,50h

	macro v#instr#pd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,8,dest,src,src2,aux
	end macro

	macro v#instr#ps? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,opcode,4,dest,src,src2,aux
	end macro

	macro v#instr#sd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_DQ,opcode+1,8,dest,src,src2,aux
	end macro

	macro v#instr#ss? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_DQ,opcode+1,4,dest,src,src2,aux
	end macro

end iterate

macro vreducepd? dest*,src*,aux*&
	AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,56h,8,dest,src,aux
end macro

macro vreduceps? dest*,src*,aux*&
	AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_VL+EVEX_DQ,56h,4,dest,src,aux
end macro

macro vreducesd? dest*,src*,src2*,aux*&
	AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_DQ,57h,8,dest,src,src2,aux
end macro

macro vreducess? dest*,src*,src2*,aux*&
	AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_DQ,57h,4,dest,src,src2,aux
end macro


iterate <instr,opcode>, pand,0DBh, pandn,0DFh, por,0EBh, pxor,0EFh

	macro v#instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, phaddw,1, phaddd,2, phaddsw,3, phsubw,5, phsubd,6, phsubsw,7, \
			psignb,8, psignw,9, psignd,0Ah

	macro v#instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, mpsadbw,42h, pblendd,02h

	macro v#instr? dest*,src*,src2*,imm*
		AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,opcode,0,dest,src,src2,imm
	end macro

end iterate

iterate <instr,opcode>, pblendw,0Eh

	macro v#instr? dest*,src*
		AVX_512.single_source_instruction VEX_66_0F38_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,opcode,0,dest,src
	end macro

end iterate

macro vpblendvb? dest*,src*,src2*,mask*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	AVX_512.parse_operand@aux mask
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'mmreg'
		if @src.size <> @dest.size | @src2.size and not @dest.size | @aux.size <> @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src2 @dest.size,VEX_66_0F3A_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,4Ch,0,@dest.rm,@src.rm,1,(@aux.rm and 1111b) shl 4
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpmovmskb? dest*,src*
	x86.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'reg' & @src.type = 'mmreg'
		if (@dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8))
			err 'invalid operand size'
		end if
		AVX_512.store_instruction@src @src.size,VEX_66_0F_W0,EVEX_FORBIDDEN+EVEX_VL_AVX2,0D7h,0,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vdppd? dest*,src*,src2*,imm*
	AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,41h,16,dest,src,src2,imm
end macro

macro vdpps? dest*,src*,src2*,imm*
	AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,40h,16,dest,src,src2,imm
end macro

macro vlddqu? dest*,src*
	AVX_512.parse_operand@dest dest
	x86.parse_operand@src src
	if @dest.type = 'mmreg' & @src.type = 'mem'
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src @dest.size,VEX_F2_0F_W0,EVEX_FORBIDDEN,0F0h,0,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,postbyte>, vldmxcsr,2, vstmxcsr,3

	macro instr? src*
		x86.parse_operand@src src
		if @src.type = 'mem'
			if @src.size and not 4
				err 'invalid operand size'
			end if
			AVX_512.store_instruction@src 16,VEX_0F_W0,EVEX_FORBIDDEN,0AEh,0,postbyte
		else
			err 'invalid operand'
		end if
	end macro

end iterate

macro vmaskmovdqu? dest*,src*
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & @src.type = 'mmreg'
		if @dest.size <> 16 | @src.size <> 16
			err 'invalid operand size'
		end if
		AVX_512.store_instruction@src 16,VEX_66_0F_W0,EVEX_FORBIDDEN,0F7h,0,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,opcode>, vmaskmovps,2Ch, vmaskmovpd,2Dh

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src2 @dest.size,VEX_66_0F38_W0,EVEX_FORBIDDEN,opcode,0,@dest.rm,@src.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg' & @src2.type = 'mmreg'
			if @src.size <> @src2.size | @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@dest @dest.size,VEX_66_0F38_W0,EVEX_FORBIDDEN,opcode+2,0,@src2.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,w>, vpmaskmovd,0, vpmaskmovq,1

	macro instr? dest*,src*,src2*
		require AVX2+
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX.store_instruction@src2 @dest.size,VEX_66_0F38_W#w,8Ch,@dest.rm,@src.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg' & @src2.type = 'mmreg'
			if @src.size <> @src2.size | @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX.store_instruction@dest @src.size,VEX_66_0F38_W#w,8Eh,@src2.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vbroadcasti128? dest*,src*
	require AVX2+
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	if @dest.type = 'mmreg' & @src.type = 'mem'
		if @dest.size <> 32 | @src.size and not 16
			err 'invalid operand size'
		end if
		AVX.store_instruction@src 32,VEX_66_0F38_W0,5Ah,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vextracti128? dest*,src*,aux*
	require AVX2+
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	x86.parse_operand@aux aux
	if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if @dest.size and not 16 | @src.size <> 32 | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction@dest 32,VEX_66_0F3A_W0,39h,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vinserti128? dest*,src*,src2*,aux*
	require AVX2+
	AVX_512.parse_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
		if @dest.size <> 32 | @src.size <> 32 | @src2.size and not 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction@src2 32,VEX_66_0F3A_W0,38h,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vperm2i128? dest*,src*,src2*,imm*
	require AVX2+
	AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,46h,32,dest,src,src2,imm
end macro

macro vzeroall?
	require AVX+
	db 0C5h,11111100b,77h
end macro

macro vzeroupper?
	require AVX+
	db 0C5h,11111000b,77h
end macro

macro xsaveopt? src*
	require AVX+
	x86.parse_operand@src src
	if @src.type = 'mem'
		x86.store_instruction@src <0Fh,0AEh>,6
	else
		err 'invalid operand'
	end if
end macro

iterate <instr,vex_mpw,opcode>, vpbroadcastmb2q,VEX_F3_0F38_W1,2Ah, vpbroadcastmw2d,VEX_F3_0F38_W0,3Ah

	macro instr? dest*,src*
		require AVX512CD
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		if @dest.type = 'mmreg' & @src.type = 'maskreg'
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpconflictd,4,VEX_66_0F38_W0,0C4h, vpconflictq,8,VEX_66_0F38_W1,0C4h, vplzcntd,4,VEX_66_0F38_W0,44h, vplzcntq,8,VEX_66_0F38_W1,44h

	macro instr? dest*,src*&
		require AVX512CD
		AVX_512.single_source_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src
	end macro

end iterate

iterate <instr,lcode>, vfmaddsub,6, vfmsubadd,7, vfmadd,8, vfmsub,0Ah, vfnmadd,0Ch, vfnmsub,0Eh

	iterate <order,hcode>, 132,90h, 213,0A0h, 231,0B0h

		macro instr#order#pd? dest*,src*,src2*
			require FMA
			AVX_512.basic_instruction VEX_66_0F38_W1,EVEX_FORBIDDEN,hcode+lcode,0,dest,src,src2
		end macro

		macro instr#order#ps? dest*,src*,src2*
			require FMA
			AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_FORBIDDEN,hcode+lcode,0,dest,src,src2
		end macro

		if lcode > 7

			macro instr#order#sd? dest*,src*,src2*
				require FMA
				AVX_512.basic_instruction VEX_66_0F38_W1,EVEX_FORBIDDEN,hcode+lcode+1,8,dest,src,src2
			end macro

			macro instr#order#ss? dest*,src*,src2*
				require FMA
				AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_FORBIDDEN,hcode+lcode+1,4,dest,src,src2
			end macro

		end if

	end iterate

end iterate

iterate <instr,opcode>, vpmadd52luq,0B4h, vpmadd52huq,0B5h

	macro instr? dest*,src*,src2*
		require AVX512_IFMA
		AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vexp2ps,4,VEX_66_0F38_W0,0C8h, vexp2pd,4,VEX_66_0F38_W1,0C8h, \
				     vrcp28ps,4,VEX_66_0F38_W0,0CAh, vrcp28pd,8,VEX_66_0F38_W1,0CAh, vrsqrt28ps,4,VEX_66_0F38_W0,0CCh, vrsqrt28pd,8,VEX_66_0F38_W1,0CCh

	macro instr? dest*,src_sae*&
		require AVX512ER
		AVX_512.parse_k1z_operand@dest dest
		match src=,sae, src_sae
			AVX_512.parse_operand@src src
			AVX_512.parse_sae@src sae
		else
			AVX_512.parse_bcst_operand@src src_sae,unit
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @dest.size <> 64
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@src @dest.size,vex_mpw,EVEX_REQUIRED,opcode,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vrcp28ss,4,VEX_66_0F38_W0,0CBh, vrcp28sd,8,VEX_66_0F38_W1,0CBh, vrsqrt28ss,4,VEX_66_0F38_W0,0CDh, vrsqrt28sd,8,VEX_66_0F38_W1,0CDh

	macro instr? dest*,src*,src2*&
		require AVX512ER
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src,src2
	end macro

end iterate

iterate <instr,postbyte>, gatherpf0,1 ,gatherpf1,2 ,scatterpf0,5, scatterpf1,6

	macro v#instr#dps? src*
		require AVX512PF
		AVX_512.parse_k1_vsib_operand@src src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 4 | @src.visize <> 64
				err 'invalid operand size'
			end if
			@src.memsize = 4
			AVX_512.store_instruction@src 64,VEX_66_0F38_W0,EVEX_REQUIRED,0C6h,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro v#instr#qps? src*
		require AVX512PF
		AVX_512.parse_k1_vsib_operand@src src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 8 | @src.visize <> 64
				err 'invalid operand size'
			end if
			@src.memsize = 4
			AVX_512.store_instruction@src 64,VEX_66_0F38_W0,EVEX_REQUIRED,0C7h,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro v#instr#dpd? src*
		require AVX512PF
		AVX_512.parse_k1_vsib_operand@src src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 4 | @src.visize <> 32
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction@src 64,VEX_66_0F38_W1,EVEX_REQUIRED,0C6h,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro v#instr#qpd? src*
		require AVX512PF
		AVX_512.parse_k1_vsib_operand@src src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 8	| @src.visize <> 64
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction@src 64,VEX_66_0F38_W1,EVEX_REQUIRED,0C7h,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpopcntb,VEX_66_0F38_W0,54h, vpopcntw,VEX_66_0F38_W1,54h

	macro instr? dest*,src*
		require AVX512_BITALG
		AVX_512.single_source_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src
	end macro

end iterate

macro vpshufbitqmb? dest*,src*,src2*
	require AVX512_BITALG
	AVX_512.parse_k1_operand@dest dest
	AVX_512.parse_operand@src src
	AVX_512.parse_operand@src2 src2
	if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if @src2.size and not @src.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction@src2 @src.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,8Fh,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,unit,vex_mpw,opcode>, vpopcntd,4,VEX_66_0F38_W0,55h, vpopcntq,8,VEX_66_0F38_W1,55h


	macro instr? dest*,src*
		require AVX512_VPOPCNTDQ
		AVX_512.single_source_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src
	end macro

end iterate

iterate <instr,opcode>, vpdpbusd,50h, vpdpbusds,51h, vpdpwssd,52h, vpdpwssds,53h

	macro instr? dest*,src*,src2*
		require AVX512_VNNI
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vp4dpwssd,52h, vp4dpwssds,53h

	macro instr? dest*,src*,src2*
		require AVX512_4VNNIW
		AVX_512.parse_k1z_operand@dest dest
		match rsrc+=3, src
			AVX_512.parse_operand@src rsrc
		else
			AVX_512.parse_operand@src src
		end match
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
			if @dest.size <> 64 | @src2.size and not 16
				err 'invalid operand size'
			else if @dest.size <> @src.size
				err 'operand sizes do not match'
			end if
			@src2.memsize = 0
			AVX_512.store_instruction@src2 @dest.size,VEX_F2_0F38_W0,EVEX_REQUIRED,opcode,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate
