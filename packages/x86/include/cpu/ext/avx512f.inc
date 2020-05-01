
if ~ defined AVX_512

	restore AVX_512 ; this ensures that symbol cannot be forward-referenced
	AVX_512 = 1

	include 'avx2.inc'

	element AVX_512.reg

	element AVX_512.r128 : AVX_512.reg + 16
	element AVX_512.r256 : AVX_512.reg + 32
	element AVX_512.r512 : AVX_512.reg + 64

	repeat 8, i:0
		element zmm#i? : AVX_512.r512 + i
	end repeat

	if defined xmm8
		repeat 16, i:16
			element xmm#i? : AVX_512.r128 + i
			element ymm#i? : AVX_512.r256 + i
		end repeat
		repeat 8+16, i:8
			element zmm#i? : AVX_512.r512 + i
		end repeat
	end if

	element AVX_512.maskreg

	repeat 8, i:0
		element k#i? : AVX_512.maskreg + i
	end repeat

	define x86.dqqword? :64
	define x86.zword? :64

	EVEX_AS_VEX = 0
	EVEX_W1 = 1 shl 15
	EVEX_REQUIRED = 1 shl 10
	EVEX_FORBIDDEN = 1 shl 2
	EVEX_VL = 1 shl 22

	AVX512VL = 0

	calminstruction AVX_512.parse_operand namespace, operand

		local	size, type, mod, rm, imm
		local	i, sym, cmd

		asmcmd	=x86.=parse_operand namespace, operand

		arrange type, namespace.=type
		arrange size, namespace.=size
		arrange imm, namespace.=imm

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
		asmcmd	=err 'invalid operand'

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

		arrange sym, namespace.=mod
		publish sym, mod

		arrange sym, namespace.=rm
		publish sym, rm

		arrange sym, namespace.=type
		publish sym, type

		arrange sym, namespace.=size
		publish sym, size

	    export_common:

		compute i, 0

		arrange sym, namespace.=mask
		publish sym, i

		arrange sym, namespace.=evex_b
		publish sym, i

		arrange sym, namespace.=memsize
		publish sym, i

	end calminstruction

	calminstruction AVX_512.parse_operand_k1z namespace,operand

		local	k1, z, mask, sym
		transform operand
		match	operand {k1} { =z? }, operand
		jyes	k1z
		match	operand {k1}, operand
		jyes	k1
		asmcmd	=AVX_512.=parse_operand namespace, operand
		exit
	    k1z:
		compute z, 80h
		jump	masked
	    k1:
		compute z, 0
	    masked:
		asmcmd	=AVX_512.=parse_operand namespace, operand
		arrange sym, namespace.=type
		check	z & sym = 'mem'
		jyes	invalid_mask
		check	k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
		jno	invalid_mask
		compute mask, (1 metadataof k1 - AVX_512.maskreg) or z
		arrange sym, namespace.=mask
		publish sym, mask
		exit
	    invalid_mask:
		asmcmd	=err 'invalid mask'

	end calminstruction

	calminstruction AVX_512.parse_operand_k1 namespace,operand

		local	k1, mask, sym
		transform operand
		match	operand {k1}, operand
		jyes	k1
		asmcmd	=AVX_512.=parse_operand namespace, operand
		exit
	    k1:
		asmcmd	=AVX_512.=parse_operand namespace, operand
		arrange sym, namespace.=type
		check	k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
		jno	invalid_mask
		compute mask, 1 metadataof k1 - AVX_512.maskreg
		arrange sym, namespace.=mask
		publish sym, mask
		exit
	    invalid_mask:
		asmcmd	=err 'invalid mask'

	end calminstruction

	calminstruction AVX_512.parse_operand_bcst namespace,operand,unit

		local	broadcast, memsize, size, b
		local	sym

		transform operand
		match	operand {broadcast}, operand
		jyes	mem_bcst
		asmcmd	=AVX_512.=parse_operand namespace, operand
		exit

	    invalid_operand:
		asmcmd	=err 'invalid operand'

	    mem_bcst:
		asmcmd	=AVX_512.=parse_operand namespace, operand
		arrange sym, namespace.=type
		check	sym = 'mem'
		jno	invalid_operand

		arrange size, namespace.=size
		compute memsize, unit
		check	memsize
		jyes	implied_unit
		check	size
		jno	operand_size_not_specified
		compute memsize, size
		jump	unit_ok
	    operand_size_not_specified:
		asmcmd	=err 'operand size not specified'
		exit
	    implied_unit:
		check	size and not memsize
		jno	unit_ok
		asmcmd	=err 'invalid operand size'
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
		asmcmd	=err 'invalid broadcast'
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
		compute b, 1

		arrange sym, namespace.=memsize
		publish sym, memsize

		arrange sym, namespace.=size
		publish sym, size

		arrange sym, namespace.=broadcast
		publish sym, broadcast

		arrange sym, namespace.=evex_b
		publish sym, b

	end calminstruction

	calminstruction AVX_512.parse_er namespace,operand,vsize:64

		local	type, size, rounding, b
		local	sym

		arrange type, namespace.=type
		arrange size, namespace.=size
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
		asmcmd	=err 'invalid operand'
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
		compute b, 1

		arrange sym, namespace.=rounding
		publish sym, rounding

		arrange sym, namespace.=evex_b
		publish sym, b

	end calminstruction

	calminstruction AVX_512.parse_sae namespace,operand

		local	type, rounding, b
		local	sym

		arrange type, namespace.=type
		check	type = 'mem'
		jyes	invalid_operand

		match	{ =sae? }, operand
		jno	invalid_operand

		compute b, 1
		compute rounding, -1

		arrange sym, namespace.=rounding
		publish sym, rounding

		arrange sym, namespace.=evex_b
		publish sym, b

		exit

	    invalid_operand:
		asmcmd	=err 'invalid operand'

	end calminstruction

	calminstruction AVX_512.store_instruction vsize*,vex_mpw*,evex_f*,opcode*,rm_operand*,mask*,reg*,vreg:0,imm_size:0,imm

		local	evex, evex_flags
		local	segment_prefix, evex_b, rounding, memsize
		local	mode, mod, rm
		local	scale, index, base
		local	displacement, displacement_size, auto_relative
		local	evex_displacement_size, compressed_displacement
		local	sym

		arrange segment_prefix, rm_operand.=segment_prefix
		arrange evex_b, rm_operand.=evex_b
		arrange rounding, rm_operand.=rounding
		arrange memsize, rm_operand.=memsize

		arrange mode, rm_operand.=mode
		arrange mod, rm_operand.=mod
		arrange rm, rm_operand.=rm

		arrange scale, rm_operand.=scale
		arrange index, rm_operand.=index
		arrange base, rm_operand.=base

		arrange displacement_size, rm_operand.=displacement_size
		arrange displacement, rm_operand.=displacement
		arrange auto_relative, rm_operand.=auto_relative

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
		asmcmd	=db segment_prefix
	    segment_prefix_ok:

		check	mod <> 11b & mode <> x86.mode
		jno	addressing_prefix_ok
		check	mode = 64 | (mode = 16 & x86.mode = 64)
		jno	store_addressing_prefix
		asmcmd	=err 'illegal addressing mode'
	    store_addressing_prefix:
		asmcmd	=db 67h
	    addressing_prefix_ok:

		compute evex, vex_mpw
		compute evex_flags, evex_f

		check	evex_b
		jno	evex_L'L
		compute evex, evex or evex_b shl 20
	    evex_L'L:
		check	mod = 11b & evex_b & rounding >= 0
		jyes	evex_rounding
		check	vsize = 64
		jyes	evex_L'
		check	evex_flags and EVEX_VL & AVX512VL = 0
		jno	evex_L
		compute evex_flags, evex_flags or EVEX_FORBIDDEN
	    evex_L:
		check	vsize = 32
		jno	evex_mask
		compute evex, evex or 1 shl 21
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
		asmcmd	=err 'instruction requires long mode'
	    allowed:

		check	displacement_size
		jno	no_displacement_compression
		compute displacement, displacement
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
		asmcmd	=err 'invalid operand'

	    vex:
		local	byte2, byte3
		compute vex, evex and 11111011_11111111b or (evex and 1 shl 21) shr (21-10)
		check	vex and 10000000_01111111b <> 1
		jyes	vex_3byte
	    vex_2byte:
		compute byte2, ((vex and 10000000b) or ((vex shr 8) and 1111111b)) xor 11111000b
		asmcmd	=db 0C5h,byte2
		jump	evex_done
	    vex_3byte:
		compute byte2, (vex and 11111111b) xor 11100000b
		compute byte3, (vex shr 8) xor 01111000b
		asmcmd	=db 0C4h,byte2,byte3
		jump	evex_done

	    evex:
		compute evex, evex or 1 shl 10
		check	evex_flags and EVEX_W1
		jno	evex_4byte
		compute evex, evex or 1 shl 15
	    evex_4byte:
		compute evex, 62h + (evex xor 00001000_01111000_11110000b) shl 8
		asmcmd	=dd evex
		check	mod <> 11b & mod <> 0 & evex_displacement_size > 0
		jno	evex_done
		compute displacement_size, evex_displacement_size
		check	evex_displacement_size = 1
		jyes	evex_compressed_displacement
		compute mod, 2
		jump	evex_done
	    evex_compressed_displacement:
		arrange sym, rm_operand.=displacement
		publish sym, compressed_displacement
		compute mod, 1
	    evex_done:

		local	modrm, sib

		compute modrm, mod shl 6 + (reg and 111b) shl 3 + rm and 111b
		asmcmd	=db opcode, modrm

		check	mod <> 11b & rm = 4 & mode <> 16
		jno	sib_ok
		compute sib, (bsf scale) shl 6 + (index and 111b) shl 3 + base and 111b
		asmcmd	=db sib
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
		asmcmd	=err 'address value out of signed range'
	      adjust_displacement_wrap:
		compute displacement, displacement - 1 shl 64
	      displacement_ready:

		arrange sym, rm_operand.=displacement
		publish sym, displacement

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
		jyes	immediate_32bit
		check	imm_size = 8
		jno	immediate_ok
		asmcmd	=x86.=simm32 imm
		jump	immediate_ok
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

	macro AVX_512.basic_instruction_bcst_er vex_mpw,evex_f,opcode,unit,dest,src,src_er&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		match src2=,er, src_er
			AVX_512.parse_operand @src2,src2
			AVX_512.parse_er @src2,er
		else
			AVX_512.parse_operand_bcst @src2,src_er,unit
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_bcst_sae vex_mpw,evex_f,opcode,unit,dest,src,src_sae&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		match src2=,sae, src_sae
			AVX_512.parse_operand @src2,src2
			AVX_512.parse_sae @src2,sae
		else
			AVX_512.parse_operand_bcst @src2,src_sae,unit
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_bcst_sae_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand_bcst @src2,src2,unit
		match sae=,imm, aux
			AVX_512.parse_sae @src2,sae
			x86.parse_operand @aux,imm
		else
			x86.parse_operand @aux,aux
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @src.size <> @dest.size | @src2.size and not @dest.size | @aux.size and not 1
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_bcst vex_mpw,evex_f,opcode,unit,dest,src,src2
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand_bcst @src2,src2,unit
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_bcst_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand_bcst @src2,src2,unit
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_er vex_mpw,evex_f,opcode,unit,dest,src,src_er&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		match src2=,er, src_er
			AVX_512.parse_operand @src2,src2
			AVX_512.parse_er @src2,er,(unit-1) and not 15 + 16
		else
			AVX_512.parse_operand @src2,src_er
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) )
				err 'invalid operand size'
			else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
				err 'operand sizes do not match'
			end if
			@src2.memsize = unit
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_sae vex_mpw,evex_f,opcode,unit,dest,src,src_sae&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		match src2=,sae, src_sae
			AVX_512.parse_operand @src2,src2
			AVX_512.parse_sae @src2,sae
		else
			AVX_512.parse_operand @src2,src_sae
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) )
				err 'invalid operand size'
			else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
				err 'operand sizes do not match'
			end if
			@src2.memsize = unit
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_sae_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		match sae=,imm, aux
			AVX_512.parse_sae @src2,sae
			x86.parse_operand @aux,imm
		else
			x86.parse_operand @aux,aux
		end match
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if ( unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) ) ) | @aux.size and not 1
				err 'invalid operand size'
			else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
				err 'operand sizes do not match'
			end if
			@src2.memsize = unit
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction vex_mpw,evex_f,opcode,unit,dest,src,src2
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) )
				err 'invalid operand size'
			else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
				err 'operand sizes do not match'
			end if
			@src2.memsize = unit
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.basic_instruction_imm8 vex_mpw,evex_f,opcode,unit,dest,src,src2,aux&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if ( unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src2.type = 'mem' & @src2.size and not unit) ) ) | @aux.size and not 1
				err 'invalid operand size'
			else if @dest.size <> @src.size | (@src2.size and not @dest.size & (unit = 0 | @src2.type = 'mmreg'))
				err 'operand sizes do not match'
			end if
			@src2.memsize = unit
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.single_source_instruction_bcst_er vex_mpw,evex_f,opcode,unit,dest,src_er&
		AVX_512.parse_operand_k1z @dest,dest
		match src=,er, src_er
			AVX_512.parse_operand @src,src
			AVX_512.parse_er @src,er
		else
			AVX_512.parse_operand_bcst @src,src_er,unit
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.single_source_instruction_bcst_sae vex_mpw,evex_f,opcode,unit,dest,src_sae&
		AVX_512.parse_operand_k1z @dest,dest
		match src=,sae, src_sae
			AVX_512.parse_operand @src,src
			AVX_512.parse_sae @src,sae
		else
			AVX_512.parse_operand_bcst @src,src_sae,unit
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.single_source_instruction_bcst_sae_imm8 vex_mpw,evex_f,opcode,unit,dest,src,aux&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand_bcst @src,src,unit
		match sae=,imm, aux
			AVX_512.parse_sae @src,sae
			x86.parse_operand @aux,imm
		else
			x86.parse_operand @aux,aux
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.single_source_instruction_bcst vex_mpw,evex_f,opcode,unit,dest,src&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand_bcst @src,src,unit
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX_512.single_source_instruction vex_mpw,evex_f,opcode,unit,dest,src
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src.type = 'mem' & @src.size and not unit) )
				err 'invalid operand size'
			else if @src.size and not @dest.size & (unit = 0 | @src.type = 'mmreg')
				err 'operand sizes do not match'
			end if
			@src.memsize = unit
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
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

	macro vshufpd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_imm8 VEX_66_0F_W0,EVEX_W1+EVEX_VL,0C6h,8,dest,src,src2,aux
	end macro

	macro vshufps? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_imm8 VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,0C6h,4,dest,src,src2,aux
	end macro

	macro vbroadcastss? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 4)
				err 'invalid operand size'
			end if
			@src2.memsize = 4
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,18h,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vbroadcastsd? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @dest.size = 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 8)
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_W1+EVEX_VL,19h,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode,opcode_g,msize>, vpbroadcastd,58h,7Ch,4, vpbroadcastq,59h,7Ch,8

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
				if (@src.type='mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not msize)
					err 'invalid operand size'
				end if
				@src.memsize = msize
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX,opcode,@src,@dest.mask,@dest.rm
			else if @dest.type = 'mmreg' & @src.type = 'reg'
				if @src.size <> msize & (@src.size <> 4 | msize = 8)
					err 'invalid operand size'
				end if
				@src.memsize = msize
				if msize = 8
					AVX_512.store_instruction @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode_g,@src,@dest.mask,@dest.rm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode_g,@src,@dest.mask,@dest.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode,msize>, vbroadcastf32x4,VEX_66_0F38_W0,1Ah,16, vbroadcastf64x4,VEX_66_0F38_W1,1Bh,32, \
					      vbroadcasti32x4,VEX_66_0F38_W0,5Ah,16, vbroadcasti64x4,VEX_66_0F38_W1,5Bh,32

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & @src.type = 'mem'
				if @dest.size <= msize | @src.size and not msize
					err 'invalid operand size'
				end if
				@src.memsize = msize
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode,unit>, vshuff32x4,VEX_66_0F3A_W0,23h,4, vshuff64x2,VEX_66_0F3A_W1,23h,4, \
					     vshufi32x4,VEX_66_0F3A_W0,43h,4, vshufi64x2,VEX_66_0F3A_W1,43h,4

		macro instr? dest*,src*,src2*,aux*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,unit
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
				if @dest.size < 32 | @aux.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vextractps? dest*,src*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not 4 | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			@dest.memsize = 4
			AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,17h,@dest,0,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vinsertps? dest*,src*,src2*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'mmreg' & @src2.size <> 16) | (@src2.type = 'mem' & @src2.size and not 4) | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src2.memsize = 4
			AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,21h,@src2,0,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,vex_mpw,opcode,msize>, vextractf32x4,VEX_66_0F3A_W0,19h,16, vextractf64x4,VEX_66_0F3A_W1,1Bh,32, \
					      vextracti32x4,VEX_66_0F3A_W0,39h,16, vextracti64x4,VEX_66_0F3A_W1,3Bh,32

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			x86.parse_operand @aux,aux
			if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
				if @dest.size and not msize | @src.size <= msize | @aux.size and not 1
					err 'invalid operand size'
				end if
				@dest.memsize = msize
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode,msize>, vinsertf32x4,VEX_66_0F3A_W0,18h,16, vinsertf64x4,VEX_66_0F3A_W1,1Ah,32, \
					      vinserti32x4,VEX_66_0F3A_W0,38h,16, vinserti64x4,VEX_66_0F3A_W1,3Ah,32

		macro instr? dest*,src*,src2*,aux*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
				if @dest.size <= msize | @src.size <= msize | @src2.size and not msize | @aux.size and not 1
					err 'invalid operand size'
				end if
				@src2.memsize = msize
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_mpw,unit>, vcmpps,VEX_0F_W0,VEX_0F_W0,4, vcmppd,VEX_66_0F_W0,VEX_66_0F_W1,8

		macro instr? dest*,src*,src2*,aux*&
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,unit
			match sae=,imm, aux
				AVX_512.parse_sae @src2,sae
				x86.parse_operand @aux,imm
			else
				x86.parse_operand @aux,aux
			end match
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
				if @aux.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_FORBIDDEN,0C2h,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
			else if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
				if @aux.size and not 1
					err 'invalid operand size'
				else if @src2.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,evex_mpw,EVEX_REQUIRED+EVEX_VL,0C2h,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_mpw,unit>, vcmpss,VEX_F3_0F_W0,VEX_F3_0F_W0,4, vcmpsd,VEX_F2_0F_W0,VEX_F2_0F_W1,8

		macro instr? dest*,src*,src2*,aux*&
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			match sae=,imm, aux
				AVX_512.parse_sae @src2,sae
				x86.parse_operand @aux,imm
			else
				x86.parse_operand @aux,aux
			end match
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
				if @dest.size <> 16 | (@src2.type = 'mem' & @src2.size and not unit)
					err 'invalid operand size'
				else if @dest.size <> @src.size | (@src2.type = 'mmreg' & @src2.size <> @dest.size)
					err 'operand sizes do not match'
				end if
				@src2.memsize = unit
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_FORBIDDEN,0C2h,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
			else if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
				if @src.size <> 16 | (@src2.type = 'mem' & @src2.size and not unit) | @aux.size and not 1
					err 'invalid operand size'
				else if @src2.type = 'mmreg' & @src2.size <> @src.size
					err 'operand sizes do not match'
				end if
				@src2.memsize = unit
				AVX_512.store_instruction @src.size,evex_mpw,EVEX_REQUIRED,0C2h,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
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
			AVX_512.parse_operand_k1z @dest,dest
			match src=,sae, src_sae
				AVX_512.parse_operand @src,src
				AVX_512.parse_sae @src,sae
			else
				AVX_512.parse_operand @src,src_sae
			end match
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if unit & ( @dest.size <> (unit-1) and not 15 + 16 | (@src.type = 'mem' & @src.size and not unit) )
					err 'invalid operand size'
				else if @src.size and not @dest.size & (unit = 0 | @src.type = 'mmreg')
					err 'operand sizes do not match'
				end if
				@src.memsize = unit
				AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, kandw,41h, kandnw,42h, knotw,44h, korw,45h, kxnorw,46h, kxorw,47h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
				AVX.store_instruction 32,VEX_0F_W0,opcode,@src2,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, knotw,44h, kortestw,98h

		macro instr? dest*,src*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'maskreg' & @src.type = 'maskreg'
				AVX.store_instruction 16,VEX_0F_W0,opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro kmovw? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'maskreg' & (@src.type = 'maskreg' | @src.type = 'mem')
			if @src.type = 'mem' & @src.size and not 2
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_0F_W0,90h,@src,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'maskreg'
			if @dest.size and not 2
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_0F_W0,91h,@dest,@src.rm
		else if @dest.type = 'maskreg' & @src.type = 'reg'
			if @src.size <> 4
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_0F_W0,92h,@src,@dest.rm
		else if @dest.type = 'reg' & @src.type = 'maskreg'
			if @dest.size <> 4
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_0F_W0,93h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,vex_mpw,opcode>, kshiftrw,VEX_66_0F3A_W1,30h, kshiftlw,VEX_66_0F3A_W1,32h

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			x86.parse_operand @aux,aux
			if @dest.type = 'maskreg' & @src.type = 'maskreg' & @aux.type = 'imm'
				if @aux.size and not 1
					err 'invalid operand size'
				end if
				AVX.store_instruction 16,vex_mpw,opcode,@src,@dest.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro kunpckbw? dest*,src*,src2*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
			AVX.store_instruction 32,VEX_66_0F_W0,4Bh,@src2,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,evex_f,opcode>, vcvtdq2pd,EVEX_AS_VEX+EVEX_VL,0E6h, vcvtudq2pd,EVEX_REQUIRED+EVEX_VL,7Ah

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand_bcst @src,src,4
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
					err 'invalid operand size'
				end if
				if @src.memsize = 0
					@src.memsize = @dest.size shr 1
				end if
				AVX_512.store_instruction @dest.size,VEX_F3_0F_W0,evex_f,opcode,@src,@dest.mask,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vcvtpd2dq,VEX_F2_0F_W0,EVEX_W1+EVEX_VL,0E6h, vcvtpd2ps,VEX_66_0F_W0,EVEX_W1+EVEX_VL,5Ah, vcvtpd2udq,VEX_0F_W1,EVEX_REQUIRED+EVEX_VL,79h

		macro instr? dest*,src_er*&
			AVX_512.parse_operand_k1z @dest,dest
			match src=,er, src_er
				AVX_512.parse_operand @src,src
				AVX_512.parse_er @src,er
			else
				AVX_512.parse_operand_bcst @src,src_er,8
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
				AVX_512.store_instruction @src.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vcvtps2pd,VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,5Ah

		macro instr? dest*,src_sae*&
			AVX_512.parse_operand_k1z @dest,dest
			match src=,sae, src_sae
				AVX_512.parse_operand @src,src
				AVX_512.parse_sae @src,sae
			else
				AVX_512.parse_operand_bcst @src,src_sae,4
			end match
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
					err 'invalid operand size'
				end if
				if @src.memsize = 0
					@src.memsize = @dest.size shr 1
				end if
				AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vcvttpd2dq,VEX_66_0F_W0,EVEX_W1+EVEX_VL,0E6h, vcvttpd2udq,VEX_0F_W1,EVEX_REQUIRED+EVEX_VL,78h

		macro instr? dest*,src_sae*&
			AVX_512.parse_operand_k1z @dest,dest
			match src=,sae, src_sae
				AVX_512.parse_operand @src,src
				AVX_512.parse_sae @src,sae
			else
				AVX_512.parse_operand_bcst @src,src_sae,8
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
				AVX_512.store_instruction @src.size,vex_mpw,evex_f,opcode,@src,@dest.mask,@dest.rm
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
		AVX_512.parse_operand_k1z @dest,dest
		match src=,sae, src_sae
			AVX_512.parse_operand @src,src
			AVX_512.parse_sae @src,sae
		else
			AVX_512.parse_operand @src,src_sae
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
				err 'invalid operand size'
			end if
			if @src.memsize = 0
				@src.memsize = @dest.size shr 1
			end if
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,13h,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vcvtps2ph? dest*,src*,aux*&
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		match sae=,imm, aux
			AVX_512.parse_sae @src,sae
			x86.parse_operand @aux,imm
		else
			x86.parse_operand @aux,aux
		end match
		if (@dest.type = 'mem' | @dest.type = 'mmreg') & @src.type = 'mmreg'
			if (@dest.type = 'mem' & @dest.size and not (@src.size shr 1)) | (@dest.type = 'mmreg' & (@src.size shr 1 - 1) and not 15 + 16 <> @dest.size)
				err 'invalid operand size'
			end if
			if @dest.memsize = 0
				@dest.memsize = @src.size shr 1
			end if
			AVX_512.store_instruction @src.size,VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_VL,1Dh,@dest,@dest.mask,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,vex_mp,evex_f,opcode,msize>, vcvtsd2si,VEX_F2_0F,EVEX_AS_VEX,2Dh,8, vcvtss2si,VEX_F3_0F,EVEX_AS_VEX,2Dh,4, \
						    vcvtsd2usi,VEX_F2_0F,EVEX_REQUIRED,79h,8, vcvtss2usi,VEX_F3_0F,EVEX_REQUIRED,79h,4

		macro instr? dest*,src_er*&
			x86.parse_operand @dest,dest
			match src=,er, src_er
				AVX_512.parse_operand @src,src
				AVX_512.parse_er @src,er,16
			else
				AVX_512.parse_operand @src,src_er
			end match
			if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@dest.size <> 4 & @dest.size <> 8) | (@src.type = 'mem' & @src.size and not msize) | (@src.type = 'mmreg' & @src.size <> 16)
					err 'invalid operand size'
				end if
				if @dest.size = 8
					if x86.mode < 64
						err 'instruction requires long mode'
					end if
					AVX_512.store_instruction 16,vex_mp#_W1,evex_f,opcode,@src,0,@dest.rm
				else
					AVX_512.store_instruction 16,vex_mp#_W0,evex_f,opcode,@src,0,@dest.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mp,evex_f,opcode,msize>, vcvttsd2si,VEX_F2_0F,EVEX_AS_VEX,2Ch,8, vcvttss2si,VEX_F3_0F,EVEX_AS_VEX,2Ch,4, \
						    vcvttsd2usi,VEX_F2_0F,EVEX_REQUIRED,78h,8, vcvttss2usi,VEX_F3_0F,EVEX_REQUIRED,78h,4

		macro instr? dest*,src_sae*&
			x86.parse_operand @dest,dest
			match src=,sae, src_sae
				AVX_512.parse_operand @src,src
				AVX_512.parse_sae @src,sae
			else
				AVX_512.parse_operand @src,src_sae
			end match
			if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@dest.size <> 4 & @dest.size <> 8) | (@src.type = 'mem' & @src.size and not msize) | (@src.type = 'mmreg' & @src.size <> 16)
					err 'invalid operand size'
				end if
				if @dest.size = 8
					if x86.mode < 64
						err 'instruction requires long mode'
					end if
					AVX_512.store_instruction 16,vex_mp#_W1,evex_f,opcode,@src,0,@dest.rm
				else
					AVX_512.store_instruction 16,vex_mp#_W0,evex_f,opcode,@src,0,@dest.rm
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

		macro vcvtsi2sd? dest*,src*,src_er*&
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			match src2=,er, src_er
				AVX_512.parse_operand @src2,src2
				AVX_512.parse_er @src2,er,8
			else
				AVX_512.parse_operand @src2,src_er
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
					AVX_512.store_instruction 16,VEX_F2_0F_W1,evex_f,opcode,@src2,0,@dest.rm,@src.rm
				else
					AVX_512.store_instruction 16,VEX_F2_0F_W0,evex_f,opcode,@src2,0,@dest.rm,@src.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,evex_f,opcode>, vcvtsi2ss,EVEX_AS_VEX,2Ah, vcvtusi2ss,EVEX_REQUIRED,7Bh

		macro vcvtsi2ss? dest*,src*,src_er*&
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			match src2=,er, src_er
				AVX_512.parse_operand @src2,src2
				AVX_512.parse_er @src2,er,@src2.size
			else
				AVX_512.parse_operand @src2,src_er
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
					AVX_512.store_instruction 16,VEX_F3_0F_W1,evex_f,opcode,@src2,0,@dest.rm,@src.rm
				else
					AVX_512.store_instruction 16,VEX_F3_0F_W0,evex_f,opcode,@src2,0,@dest.rm,@src.rm
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
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
				if @src.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode_rm,@src,@dest.mask,@dest.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg'
				if @dest.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,evex_f,opcode_mr,@dest,@dest.mask,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vmovd? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'reg' | @src.type = 'mem')
			if @dest.size <> 16 | @src.size and not 4
				err 'invalid operand size'
			end if
			@src.memsize = 4
			AVX_512.store_instruction 16,VEX_66_0F_W0,EVEX_AS_VEX,6Eh,@src,0,@dest.rm
		else if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg'
			if @dest.size and not 4 | @src.size <> 16
				err 'operand sizes do not match'
			end if
			@dest.memsize = 4
			AVX_512.store_instruction 16,VEX_66_0F_W0,EVEX_AS_VEX,7Eh,@dest,0,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vmovq? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @dest.size <> 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' and @src.size and not 8)
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction 16,VEX_F3_0F_W0,EVEX_W1,7Eh,@src,0,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not 8 | @src.size <> 16
				err 'invalid operand size'
			end if
			@dest.memsize = 8
			AVX_512.store_instruction 16,VEX_66_0F_W0,EVEX_W1,0D6h,@dest,0,@src.rm
		else if @dest.type = 'mmreg' & @src.type = 'reg'
			if @dest.size <> 16 | @src.size <> 8
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX_512.store_instruction 16,VEX_66_0F_W1,EVEX_W1,6Eh,@src,0,@dest.rm
		else if @dest.type = 'reg' & @src.type = 'mmreg'
			if @dest.size <> 8 | @src.size <> 16
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX_512.store_instruction 16,VEX_66_0F_W1,EVEX_W1,7Eh,@dest,0,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vmovddup? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
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
			AVX_512.store_instruction @dest.size,VEX_F2_0F_W0,EVEX_W1+EVEX_VL,12h,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode>, vmovhlps,12h, vmovlhps,16h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mmreg'
				if @dest.size <> 16
					err 'invalid operand size'
				else if @src.size <> @dest.size | @src2.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction 16,VEX_0F_W0,EVEX_AS_VEX,opcode,@src2,0,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vmovhpd,VEX_66_0F_W0,EVEX_W1,16h, vmovhps,VEX_0F_W0,EVEX_AS_VEX,16h, vmovlpd,VEX_66_0F_W0,EVEX_W1,12h, vmovlps,VEX_0F_W0,EVEX_AS_VEX,12h

		macro instr? dest*,src*,src2
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			match , src2
				if @dest.type = 'mem' & @src.type = 'mmreg'
					if @dest.size and not 8 | @src.size <> 16
						err 'invalid operand size'
					end if
					@dest.memsize = 8
					AVX_512.store_instruction 16,vex_mpw,evex_f,opcode+1,@dest,0,@src.rm
				else
					err 'invalid combination of operands'
				end if
			else
				AVX_512.parse_operand @src2,src2
				if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
					if @dest.size <> 16 | @src.size <> 16 | @src2.size and not 8
						err 'invalid operand size'
					end if
					@src2.memsize = 8
					AVX_512.store_instruction 16,vex_mpw,evex_f,opcode,@src2,0,@dest.rm,@src.rm
				else
					err 'invalid combination of operands'
				end if
			end match
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vmovntdq,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,0E7h, vmovntpd,VEX_66_0F_W0,EVEX_W1+EVEX_VL,2Bh, vmovntps,VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,2Bh

		macro instr? dest*,src*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mem' & @src.type = 'mmreg'
				if @dest.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,evex_f,opcode,@dest,0,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vmovntdqa? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,2Ah,@src,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,vex_mpw,evex_f,msize>, vmovsd,VEX_F2_0F_W0,EVEX_W1,8, vmovss,VEX_F3_0F_W0,EVEX_AS_VEX,4

		macro instr? dest*,src*,src2
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			match , src2
				if @dest.type = 'mmreg' & @src.type = 'mem'
					if @dest.size <> 16 | @src.size and not msize
						err 'invalid operand size'
					end if
					@src.memsize = msize
					AVX_512.store_instruction 16,vex_mpw,evex_f,10h,@src,@dest.mask,@dest.rm
				else if @dest.type = 'mem' & @src.type = 'mmreg'
					if @dest.size and not msize | @src.size <> 16
						err 'invalid operand size'
					end if
					@dest.memsize = msize
					AVX_512.store_instruction 16,vex_mpw,evex_f,11h,@dest,@dest.mask,@src.rm
				else
					err 'invalid combination of operands'
				end if
			else
				AVX_512.parse_operand @src2,src2
				if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mmreg'
					if @dest.size <> 16 | @src.size <> 16 | @src2.size <> 16
						err 'invalid operand size'
					end if
					AVX_512.store_instruction 16,vex_mpw,evex_f,10h,@src2,@dest.mask,@dest.rm,@src.rm
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
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand_bcst @src,src,unit
			AVX_512.parse_operand_bcst @src2,src2,unit
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,evex_f,opcode_rrm,@src2,@dest.mask,@dest.rm,@src.rm
			else if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @src2.type = 'imm'
				if @src2.size and not 1
					err 'invalid operand size'
				else if @src.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,VEX_66_0F3A_W0,evex_f,opcode_rri,@src,@dest.mask,@dest.rm,,1,@src2.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpaddd,0FEh, vpsubd,0FAh, vpunpckhdq,6Ah, vpunpckldq,62h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpaddq,0D4h, vpmuludq,0F4h, vpsubq,0FBh, vpunpckhqdq,6Dh, vpunpcklqdq,6Ch

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_W1+EVEX_VL,opcode,8,dest,src,src2
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
			AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpmuldq,28h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_W1+EVEX_VL,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpmuldq,28h, vpmaxsq,3Dh, vpmaxuq,3Fh, vpminsq,39h, vpminuq,3Bh, vpmullq,40h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpabsd,1Eh

		macro instr? dest*,src*
			AVX_512.single_source_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src
		end macro

	end iterate

	iterate <instr,opcode>, vpabsq,1Fh

		macro instr? dest*,src*
			AVX_512.single_source_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src
		end macro

	end iterate

	iterate <instr,vex_mpw>, vpshufd,VEX_66_0F_W0

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
				if @aux.size and not 1
					err 'invalid operand size'
				else if @src.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_AS_VEX+EVEX_VL,70h,@src,@dest.mask,@dest.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpsllvd,47h, vpsrlvd,45h, vpsravd,46h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,evex_f,opcode>, vpsllvq,EVEX_AS_VEX+EVEX_VL,47h, vpsrlvq,EVEX_AS_VEX+EVEX_VL,45h, vpsravq,EVEX_REQUIRED+EVEX_VL,46h

		macro instr? dest*,src*,src2*
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
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand_bcst @src,src,unit
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
				if @src.size and not @dest.size | @aux.size and not 1
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,72h,@src,@dest.mask,postbyte,@dest.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode_rrm,opcode,postbyte>, vpslld,0F2h,72h,6, vpsrad,0E2h,72h,4, vpsrld,0D2h,72h,2

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src2,src2
			if @src2.type = 'imm'
				AVX_512.parse_operand_bcst @src,src,4
			else
				AVX_512.parse_operand @src,src
			end if
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				@src2.memsize = 16
				if @src2.size and not @src2.memsize
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX,opcode_rrm,@src2,@dest.mask,@dest.rm,@src.rm
			else if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @src2.type = 'imm'
				if @src2.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				if @src.type = 'mem'
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode_rrm,opcode,postbyte>, vpsllq,0F3h,73h,6, vpsraq,0E2h,72h,4, vpsrlq,0D3h,73h,2

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src2,src2
			if @src2.type = 'imm'
				AVX_512.parse_operand_bcst @src,src,8
			else
				AVX_512.parse_operand @src,src
			end if
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				@src2.memsize = 16
				if @src2.size and not @src2.memsize
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				if `instr = 'vpsraq'
					AVX_512.store_instruction @dest.size,VEX_66_0F_W1,EVEX_REQUIRED+EVEX_VL,opcode_rrm,@src2,@dest.mask,@dest.rm,@src.rm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_W1+EVEX_VL,opcode_rrm,@src2,@dest.mask,@dest.rm,@src.rm
				end if
			else if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @src2.type = 'imm'
				if @src2.size and not 1
					err 'invalid operand size'
				else if @src.size and not @dest.size
					err 'operand sizes do not match'
				end if
				if @src.type = 'mem' | `instr = 'vpsraq'
					AVX_512.store_instruction @dest.size,VEX_66_0F_W1,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_W1+EVEX_VL,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpcmpeqd,76h, vpcmpgtd,66h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,4
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src2.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,VEX_66_0F_W0,EVEX_FORBIDDEN,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpcmpeqq,29h, vpcmpgtq,37h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,8
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src2.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,VEX_66_0F38_W0,EVEX_FORBIDDEN,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,unit,vex_mpw,opcode>, vptestnmd,4,VEX_F3_0F38_W0,27h, vptestnmq,8,VEX_F3_0F38_W1,27h, vptestmd,4,VEX_66_0F38_W0,27h, vptestmq,8,VEX_66_0F38_W1,27h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,unit
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src2.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,unit,vex_mpw,opcode>, vpcmpd,4,VEX_66_0F3A_W0,1Fh, vpcmpud,4,VEX_66_0F3A_W0,1Eh, vpcmpq,8,VEX_66_0F3A_W1,1Fh, vpcmpuq,8,VEX_66_0F3A_W1,1Eh

		macro instr? dest*,src*,src2*,aux*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,unit
			x86.parse_operand @aux,aux
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
				if @src2.size and not @src.size | @aux.size and not 1
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode,msize>, vpmovsxbd,21h,4, vpmovsxbq,22h,2, vpmovsxwd,23h,8, vpmovsxwq,24h,4, vpmovsxdq,25h,8, \
				      vpmovzxbd,31h,4, vpmovzxbq,32h,2, vpmovzxwd,33h,8, vpmovzxwq,34h,4, vpmovzxdq,35h,8

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				@src.memsize = msize * (@dest.size shr 4)
				if (@src.type = 'mmreg' & @src.size <> (@src.memsize-1) and not 15 + 16) | (@src.type = 'mem' & @src.size and not @src.memsize)
					err 'invalid operand size'
				end if
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpermq,0, vpermpd,1

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand_bcst @src,src,8
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
				if @dest.size < 32 | @aux.size and not 1
					err 'invalid operand size'
				else if @src.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,VEX_66_0F3A_W1,EVEX_AS_VEX+EVEX_VL,opcode,@src,@dest.mask,@dest.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpermd,36h, vpermps,16h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,4
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @dest.size < 32
					err 'invalid operand size'
				else if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

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
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
				if @dest.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm
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
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
				@dest.memsize = @src.size / ratio
				if (@dest.type = 'mmreg' & @dest.size <> (@dest.memsize-1) and not 15 + 16) | (@dest.type = 'mem' & @dest.size and not @dest.memsize)
					err 'invalid operand size'
				end if
				AVX_512.store_instruction @src.size,VEX_F3_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	calminstruction AVX_512.parse_vsib_operand namespace, operand

		local	size, type, segment_prefix
		local	displacement, displacement_size, auto_relative
		local	address, base_registers, index_registers
		local	mode, mod, rm
		local	scale, index, base
		local	visize

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
		check	scale > 2 & scale <> 4 & scale <> 8
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
		jyes	export_address
		compute mode, 32
		jump	export_address
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
		jump	export_address
	      displacement_8bit_wrap:
		compute displacement, displacement - 1 shl mode
	      displacement_8bit:
		compute displacement_size, 1
		compute mod, 1
		jump	export_address
	      index_only:
		compute displacement_size, 4
		compute mod, 0
		jump	export_address
	      displacement_empty:
		compute displacement_size, 0
		compute mod, 0

	  export_address:

		arrange sym, namespace.=address
		publish sym, address

		arrange sym, namespace.=scale
		publish sym, scale

		arrange sym, namespace.=index
		publish sym, index

		arrange sym, namespace.=base
		publish sym, base

		arrange sym, namespace.=auto_relative
		publish sym, auto_relative

		arrange sym, namespace.=displacement
		publish sym, displacement

		arrange sym, namespace.=mode
		publish sym, mode

		arrange sym, namespace.=mod
		publish sym, mod

		arrange sym, namespace.=rm
		publish sym, rm

		arrange sym, namespace.=type
		publish sym, type

		arrange sym, namespace.=size
		publish sym, size

		arrange sym, namespace.=visize
		publish sym, visize

		arrange sym, namespace.=displacement_size
		publish sym, displacement_size

		arrange sym, namespace.=segment_prefix
		publish sym, segment_prefix

		compute i, 0

		arrange sym, namespace.=mask
		publish sym, i

		arrange sym, namespace.=evex_b
		publish sym, i

		arrange sym, namespace.=memsize
		publish sym, i

		exit

	  invalid_operand:
		asmcmd	=err 'invalid operand'
		exit
	  invalid_address:
		asmcmd	=err 'invalid address'
		exit
	  invalid_address_size:
		asmcmd	=err 'invalid address size'
		exit

	end calminstruction

	calminstruction AVX_512.parse_vsib_operand_k1 namespace,operand

		local	k1, mask, sym
		transform operand
		match	operand {k1}, operand
		jyes	k1
		asmcmd	=AVX_512.=parse_vsib_operand namespace, operand
		exit
	    k1:
		asmcmd	=AVX_512.=parse_vsib_operand namespace, operand
		arrange sym, namespace.=type
		check	k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
		jno	invalid_mask
		compute mask, 1 metadataof k1 - AVX_512.maskreg
		arrange sym, namespace.=mask
		publish sym, mask
		exit
	    invalid_mask:
		asmcmd	=err 'invalid mask'

	end calminstruction

	iterate <instr,opcode,asize>, vpgatherdd,90h,4, vpgatherqd,91h,8, vgatherdps,92h,4, vgatherqps,93h,8

		macro instr? dest*,src*,aux
			match , aux
				AVX_512.parse_operand_k1 @dest,dest
				AVX_512.parse_vsib_operand @src,src
				if @dest.type = 'mmreg' & @dest.mask & @src.type = 'mem'
					if @src.size and not 4 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize)
						err 'invalid operand size'
					else if @dest.rm = @src.index
						err 'disallowed combination of registers'
					end if
					@src.memsize = 4
					AVX_512.store_instruction @src.visize,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm,@src.index and 10000b
				else
					err 'invalid combination of operands'
				end if
			else
				AVX.parse_operand @dest,dest
				AVX.parse_vsib_operand @src,src
				AVX.parse_operand @aux,aux
				if @dest.type = 'mmreg' & @src.type = 'mem' & @aux.type = 'mmreg'
					if @src.size and not 4 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize)
						err 'invalid operand size'
					else if @aux.size <> @dest.size
						err 'operand sizes do not match'
					else if @dest.rm = @aux.rm | @dest.rm = @src.index | @aux.rm = @src.index
						err 'disallowed combination of registers'
					end if
					AVX.store_instruction @src.visize,VEX_66_0F38_W0,opcode,@src,@dest.rm,@aux.rm
				else
					err 'invalid combination of operands'
				end if
			end match
		end macro

	end iterate

	iterate <instr,opcode,asize>, vpgatherdq,90h,4, vpgatherqq,91h,8, vgatherdpd,92h,4, vgatherqpd,93h,8

		macro instr? dest*,src*,aux
			match , aux
				AVX_512.parse_operand_k1 @dest,dest
				AVX_512.parse_vsib_operand @src,src
				if @dest.type = 'mmreg' & @dest.mask & @src.type = 'mem'
					if @src.size and not 8 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize * 2) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize * 2)
						err 'invalid operand size'
					else if @dest.rm = @src.index
						err 'disallowed combination of registers'
					end if
					@src.memsize = 8
					AVX_512.store_instruction @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm,@src.index and 10000b
				else
					err 'invalid combination of operands'
				end if
			else
				AVX.parse_operand @dest,dest
				AVX.parse_vsib_operand @src,src
				AVX.parse_operand @aux,aux
				if @dest.type = 'mmreg' & @src.type = 'mem' & @aux.type = 'mmreg'
					if @src.size and not 8 | (@dest.size > 16 & @dest.size * (asize shr 2) > @src.visize * 2) | (@src.visize > 16 & @dest.size * (asize shr 2) < @src.visize * 2)
						err 'invalid operand size'
					else if @aux.size <> @dest.size
						err 'operand sizes do not match'
					else if @dest.rm = @aux.rm | @dest.rm = @src.index | @aux.rm = @src.index
						err 'disallowed combination of registers'
					end if
					AVX.store_instruction @dest.size,VEX_66_0F38_W1,opcode,@src,@dest.rm,@aux.rm
				else
					err 'invalid combination of operands'
				end if
			end match
		end macro

	end iterate

	iterate <instr,opcode,asize>, vpscatterdd,0A0h,4, vpscatterqd,0A1h,8, vscatterdps,0A2h,4, vscatterqps,0A3h,8

		macro instr? dest*,src*
			AVX_512.parse_vsib_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mem' & @dest.mask & @src.type = 'mmreg'
				if @dest.size and not 4 | (@src.size > 16 & @src.size * (asize shr 2) > @dest.visize) | (@dest.visize > 16 & @src.size * (asize shr 2) < @dest.visize)
					err 'invalid operand size'
				else if @src.rm = @dest.index
					err 'disallowed combination of registers'
				end if
				@dest.memsize = 4
				AVX_512.store_instruction @dest.visize,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm,@dest.index and 10000b
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode,asize>, vpscatterdq,0A0h,4, vpscatterqq,0A1h,8, vscatterdpd,0A2h,4, vscatterqpd,0A3h,8

		macro instr? dest*,src*
			AVX_512.parse_vsib_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mem' & @dest.mask & @src.type = 'mmreg'
				if @dest.size and not 8 | (@src.size > 16 & @src.size * (asize shr 2) > @dest.visize * 2) | (@dest.visize > 16 & @src.size * (asize shr 2) < @dest.visize * 2)
					err 'invalid operand size'
				else if @src.rm = @dest.index
					err 'disallowed combination of registers'
				end if
				@dest.memsize = 8
				AVX_512.store_instruction @src.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm,@dest.index and 10000b
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

end if
