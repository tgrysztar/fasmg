
if ~ defined AVX

	restore AVX	; this ensures that symbol cannot be forward-referenced
	AVX = 1

	include 'sse4.2.inc'
	include 'xsave.inc'

	element AVX.reg

	repeat 8, i:0
		element ymm#i? : AVX.reg + i
	end repeat

	if defined xmm8
		repeat 8, i:8
			element ymm#i? : AVX.reg + i
		end repeat
	end if

	define x86.qqword? :32
	define x86.yword? :32

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

	calminstruction AVX.parse_operand namespace, operand

		local	size, type, mod, rm, imm
		local	i, sym, cmd

		asmcmd	=x86.=parse_operand namespace, operand

		arrange type, namespace.=type
		arrange size, namespace.=size
		arrange imm, namespace.=imm

		check	type = 'reg' & size = 1 & rm >= 4 & (~ defined x86.REX_FORBIDDEN | rm and x86.REX_FORBIDDEN)
		jyes	invalid_operand
		check	type = 'imm' & size = 0
		jno	done

		check	imm eq 1 elementof imm & 1 metadataof imm relativeto SSE.reg
		jyes	xmm_register
		check	imm eq 1 elementof imm & 1 metadataof imm relativeto AVX.reg
		jyes	ymm_register
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

	    export_mmreg:

		compute type, 'mmreg'
		compute mod, 11b

		arrange sym, namespace.=mod
		publish sym, mod

		arrange sym, namespace.=rm
		publish sym, rm

		arrange sym, namespace.=type
		publish sym, type

		arrange sym, namespace.=size
		publish sym, size

	    done:

	end calminstruction

	calminstruction AVX.store_instruction vsize*,vex_mpw*,opcode*,rm_operand*,reg*,vreg:0,imm_size:0,imm

		local	segment_prefix, vex
		local	mode, mod, rm
		local	scale, index, base
		local	displacement, displacement_size, auto_relative
		local	sym

		arrange segment_prefix, rm_operand.=segment_prefix

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

		compute vex, vex_mpw
	    vex_L:
		check	vsize = 32
		jno	vex_B
		compute vex, vex or 1 shl 10
	    vex_B:
		check	rm and 1000b | (mod <> 11b & mode > 16 & rm = 4 & base and 1000b)
		jno	vex_X
		compute vex, vex or 1 shl 5
	    vex_X:
		check	mod <> 11b & mode > 16 & rm = 4 & index and 1000b
		jno	vex_R
		compute vex, vex or 1 shl 6
	    vex_R:
		check	reg and 1000b
		jno	vex_vvvv
		compute vex, vex or 1 shl 7
	    vex_vvvv:
		compute vex, vex or (vreg and 1111b) shl 11

		check	x86.mode < 64 & vex and 01000000_11100000b
		jno	allowed
		asmcmd	=err 'instruction requires long mode'
	    allowed:

		local	byte2, byte3
		check	vex and 10000000_01111111b <> 1
		jyes	vex_3byte
	    vex_2byte:
		compute byte2, ((vex and 10000000b) or ((vex shr 8) and 1111111b)) xor 11111000b
		asmcmd	=db 0C5h,byte2
		jump	vex_done
	    vex_3byte:
		compute byte2, (vex and 11111111b) xor 11100000b
		compute byte3, (vex shr 8) xor 01111000b
		asmcmd	=db 0C4h,byte2,byte3
	    vex_done:

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

		compute displacement, displacement

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

	macro AVX.basic_instruction vex_mpw,opcode,msize,dest,src,src2
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		AVX.parse_operand @src2,src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if msize & (@dest.size < msize | (@dest.size > msize & @dest.size <> 16) | (@src2.type = 'mem' & @src2.size and not msize))
				err 'invalid operand size'
			else if @src.size <> @dest.size | (@src2.size and not @dest.size & (@src2.type = 'mmreg' | msize = 0))
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,vex_mpw,opcode,@src2,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX.basic_instruction_imm8 vex_mpw,opcode,msize,dest,src,src2,aux
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		AVX.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if (msize & (@dest.size < msize | (@dest.size > msize & @dest.size <> 16) | (@src2.type = 'mem' & @src2.size and not msize))) | @aux.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size | (@src2.size and not @dest.size & (@src2.type = 'mmreg' | msize = 0))
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,vex_mpw,opcode,@src2,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX.single_source_instruction vex_mpw,opcode,msize,dest,src
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if msize & (@dest.size < msize | (@dest.size > msize & @dest.size <> 16) | (@src.type = 'mem' & @src.size and not msize))
				err 'invalid operand size'
			else if @src.size and not @dest.size & (@src.type = 'mmreg' | msize = 0)
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,vex_mpw,opcode,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro AVX.single_source_instruction_imm8 vex_mpw,opcode,msize,dest,src,aux
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if (msize & (@dest.size < msize | (@dest.size > msize & @dest.size <> 16) | (@src.type = 'mem' & @src.size and not msize))) | @aux.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size & (@src.type = 'mmreg' | msize = 0)
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,vex_mpw,opcode,@src,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode>, add,58h, mul,59h, sub,5Ch, min,5Dh, div,5Eh, max,5Fh

		macro v#instr#pd? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F_W0,opcode,0,dest,src,src2
		end macro

		macro v#instr#ps? dest*,src*,src2*
			AVX.basic_instruction VEX_0F_W0,opcode,0,dest,src,src2
		end macro

		macro v#instr#sd? dest*,src*,src2*
			AVX.basic_instruction VEX_F2_0F_W0,opcode,8,dest,src,src2
		end macro

		macro v#instr#ss? dest*,src*,src2*
			AVX.basic_instruction VEX_F3_0F_W0,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, and,54h, andn,55h, or,56h, unpckh,15h, unpckl,14h, xor,57h

		macro v#instr#pd? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F_W0,opcode,0,dest,src,src2
		end macro

		macro v#instr#ps? dest*,src*,src2*
			AVX.basic_instruction VEX_0F_W0,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, addsub,0D0h, hadd,7Ch, hsub,7Dh

		macro v#instr#pd? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F_W0,opcode,0,dest,src,src2
		end macro

		macro v#instr#ps? dest*,src*,src2*
			AVX.basic_instruction VEX_F2_0F_W0,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, rsqrt,52h, rcp,53h

		macro v#instr#ps? dest*,src*
			AVX.single_source_instruction VEX_0F_W0,opcode,0,dest,src
		end macro

		macro v#instr#ss? dest*,src*,src2*
			AVX.basic_instruction VEX_F3_0F_W0,opcode,4,dest,src,src2
		end macro

	end iterate

	macro vsqrtpd? dest*,src*
		AVX.single_source_instruction VEX_66_0F_W0,51h,0,dest,src
	end macro

	macro vsqrtps? dest*,src*
		AVX.single_source_instruction VEX_0F_W0,51h,0,dest,src
	end macro

	macro vsqrtsd? dest*,src*,src2*
		AVX.basic_instruction VEX_F2_0F_W0,51h,8,dest,src,src2
	end macro

	macro vsqrtss? dest*,src*,src2*
		AVX.basic_instruction VEX_F3_0F_W0,51h,4,dest,src,src2
	end macro

	macro vroundpd? dest*,src*,aux*
		AVX.single_source_instruction_imm8 VEX_66_0F3A_W0,9,0,dest,src,aux
	end macro

	macro vroundps? dest*,src*,aux*
		AVX.single_source_instruction_imm8 VEX_66_0F3A_W0,8,0,dest,src,aux
	end macro

	macro vroundsd? dest*,src*,src2*,aux*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,0Bh,8,dest,src,src2,aux
	end macro

	macro vroundss? dest*,src*,src2*,aux*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,0Ah,4,dest,src,src2,aux
	end macro

	macro vshufpd? dest*,src*,src2*,aux*
		AVX.basic_instruction_imm8 VEX_66_0F_W0,0C6h,0,dest,src,src2,aux
	end macro

	macro vshufps? dest*,src*,src2*,aux*
		AVX.basic_instruction_imm8 VEX_0F_W0,0C6h,0,dest,src,src2,aux
	end macro

	iterate <instr,opcode>, blendps,0Ch, blendpd,0Dh

		macro v#instr? dest*,src*,src2*,imm*
			AVX.basic_instruction_imm8 VEX_66_0F3A_W0,opcode,0,dest,src,src2,imm
		end macro

	end iterate

	iterate <instr,opcode>, blendvps,4Ah, blendvpd,4Bh

		macro v#instr? dest*,src*,src2*,mask*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			AVX.parse_operand @src2,src2
			AVX.parse_operand @aux,mask
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'mmreg'
				if @src.size <> @dest.size | @src2.size and not @dest.size | @aux.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F3A_W0,opcode,@src2,@dest.rm,@src.rm,1,(@aux.rm and 1111b) shl 4
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vbroadcastss? dest*,src*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @src.size and not 4
				err 'invalid operand size'
			end if
			AVX.store_instruction @dest.size,VEX_66_0F38_W0,18h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode,msize>, vbroadcastsd,19h,8, vbroadcastf128,1Ah,16

		macro instr? dest*,src*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'mmreg' & @src.type = 'mem'
				if @dest.size <> 32 | @src.size and not msize
					err 'invalid operand size'
				end if
				AVX.store_instruction 32,VEX_66_0F38_W0,opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vextractps? dest*,src*,aux*
		x86.parse_operand @dest,dest
		AVX.parse_operand @src,src
		x86.parse_operand @aux,aux
		if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not 4 | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F3A_W0,17h,@dest,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vinsertps? dest*,src*,src2*,aux*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		AVX.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'mmreg' & @src2.size <> 16) | (@src2.type = 'mem' & @src2.size and not 4) | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F3A_W0,21h,@src2,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vextractf128? dest*,src*,aux*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		x86.parse_operand @aux,aux
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not 16 | @src.size <> 32 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 32,VEX_66_0F3A_W0,19h,@dest,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vinsertf128? dest*,src*,src2*,aux*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		AVX.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 32 | @src.size <> 32 | @src2.size and not 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 32,VEX_66_0F3A_W0,18h,@src2,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode>, cmp,0C2h

		macro v#instr#pd? dest*,src*,src2*,imm*
			AVX.basic_instruction_imm8 VEX_66_0F_W0,opcode,0,dest,src,src2,imm
		end macro

		macro v#instr#ps? dest*,src*,src2*,imm*
			AVX.basic_instruction_imm8 VEX_0F_W0,opcode,0,dest,src,src2,imm
		end macro

		macro v#instr#sd? dest*,src*,src2*,imm*
			AVX.basic_instruction_imm8 VEX_F2_0F_W0,opcode,8,dest,src,src2,imm
		end macro

		macro v#instr#ss? dest*,src*,src2*,imm*
			AVX.basic_instruction_imm8 VEX_F3_0F_W0,opcode,4,dest,src,src2,imm
		end macro

	end iterate

	iterate <cond,code>, eq,0, lt,1, le,2, unord,3, neq,4, nlt,5, nle,6, ord,7, \
			     eq_uq,8, nge,9, ngt,0Ah, false,0Bh, neq_qq,0Ch, ge,0Dh, gt,0Eh, true,0Fh, \
			     eq_os,10h, lt_oq,11h, le_oq,12h, unord_s,13h, neq_us,14h, nlt_uq,15h, nle_uq,16h, ord_s,17h, \
			     eq_us,18h, nge_uq,19h, ngt_uq,1Ah, false_os,1Bh, neq_os,1Ch, ge_oq,1Dh, gt_oq,1Eh, true_us,1Fh

		macro vcmp#cond#pd? dest*,src*,src2*
			vcmppd dest,src,src2,code
		end macro

		macro vcmp#cond#ps? dest*,src*,src2*
			vcmpps dest,src,src2,code
		end macro

		macro vcmp#cond#sd? dest*,src*,src2*
			vcmpsd dest,src,src2,code
		end macro

		macro vcmp#cond#ss? dest*,src*,src2*
			vcmpss dest,src,src2,code
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode,msize>, vcomiss,VEX_0F_W0,2Fh,4, vcomisd,VEX_66_0F_W0,2Fh,8, vucomiss,VEX_0F_W0,2Eh,4, vucomisd,VEX_66_0F_W0,2Eh,8

		macro instr? dest*,src*
			AVX.single_source_instruction vex_mpw,opcode,msize,dest,src
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vcvtdq2pd,VEX_F3_0F_W0,0E6h, vcvtps2pd,VEX_0F_W0,5Ah

		macro instr? dest*,src*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & @src.size <> 16)
					err 'invalid operand size'
				end if
				AVX.store_instruction @dest.size,vex_mpw,opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vcvtpd2dq,VEX_F2_0F_W0,0E6h, vcvttpd2dq,VEX_66_0F_W0,0E6h, vcvtpd2ps,VEX_66_0F_W0,5Ah

		macro instr? dest*,src*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if @src.size = 0
					err ' operand size not specified'
				else if @dest.size <> 16 | (@src.size <> 16 & @src.size <> 32)
					err 'invalid operand size'
				end if
				AVX.store_instruction @dest.size,vex_mpw,opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vcvtdq2ps? dest*,src*
		AVX.single_source_instruction VEX_0F_W0,5Bh,0,dest,src
	end macro

	macro vcvtps2dq? dest*,src*
		AVX.single_source_instruction VEX_66_0F_W0,5Bh,0,dest,src
	end macro

	macro vcvttps2dq? dest*,src*
		AVX.single_source_instruction VEX_F3_0F_W0,5Bh,0,dest,src
	end macro

	iterate <instr,vex_mp,opcode,msize>, vcvtsd2si,VEX_F2_0F,2Dh,8, vcvttsd2si,VEX_F2_0F,2Ch,8, vcvtss2si,VEX_F3_0F,2Dh,4, vcvttss2si,VEX_F3_0F,2Ch,4

		macro instr? dest*,src*
			x86.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@dest.size <> 4 & @dest.size <> 8) | (@src.type = 'mem' & @src.size and not msize) | (@src.type = 'mmreg' & @src.size <> 16)
					err 'invalid operand size'
				end if
				if @dest.size = 8
					if x86.mode < 64
						err 'instruction requires long mode'
					end if
					AVX.store_instruction 16,vex_mp#_W1,opcode,@src,@dest.rm
				else
					AVX.store_instruction 16,vex_mp#_W0,opcode,@src,@dest.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vcvtsd2ss? dest*,src*,src2*
		AVX.basic_instruction VEX_F2_0F_W0,5Ah,8,dest,src,src2
	end macro

	macro vcvtss2sd? dest*,src*,src2*
		AVX.basic_instruction VEX_F3_0F_W0,5Ah,4,dest,src,src2
	end macro

	iterate <instr,vex_mp,opcode>, vcvtsi2sd,VEX_F2_0F,2Ah, vcvtsi2ss,VEX_F3_0F,2Ah

		macro instr? dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			x86.parse_operand @src2,src2
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
					AVX.store_instruction 16,vex_mp#_W1,opcode,@src2,@dest.rm,@src.rm
				else
					AVX.store_instruction 16,vex_mp#_W0,opcode,@src2,@dest.rm,@src.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vdppd? dest*,src*,src2*,imm*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,41h,16,dest,src,src2,imm
	end macro

	macro vdpps? dest*,src*,src2*,imm*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,40h,16,dest,src,src2,imm
	end macro

	macro vlddqu? dest*,src*
		AVX.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,VEX_F2_0F_W0,0F0h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,postbyte>, vldmxcsr,2, vstmxcsr,3

		macro instr? src*
			x86.parse_operand @src,src
			if @src.type = 'mem'
				if @src.size and not 4
					err 'invalid operand size'
				end if
				AVX.store_instruction 16,VEX_0F_W0,0AEh,@src,postbyte
			else
				err 'invalid operand'
			end if
		end macro

	end iterate

	macro vmaskmovdqu? dest*,src*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mmreg'
			if @dest.size <> 16 | @src.size <> 16
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F_W0,0F7h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode>, vmaskmovps,2Ch, vmaskmovpd,2Dh

		macro instr? dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			AVX.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
				if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F38_W0,opcode,@src2,@dest.rm,@src.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg' & @src2.type = 'mmreg'
				if @src.size <> @src2.size | @dest.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F38_W0,opcode+2,@dest,@src2.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode_rm,opcode_mr>, vmovapd,VEX_66_0F_W0,28h,29h, vmovaps,VEX_0F_W0,28h,29h, vmovdqa,VEX_66_0F_W0,6Fh,7Fh, vmovdqu,VEX_F3_0F_W0,6Fh,7Fh, vmovupd,VEX_66_0F_W0,10h,11h, vmovups,VEX_0F_W0,10h,11h

		macro instr? dest*,src*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
				if @src.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,vex_mpw,opcode_rm,@src,@dest.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg'
				if @dest.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @src.size,vex_mpw,opcode_mr,@dest,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vmovd? dest*,src*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'reg' | @src.type = 'mem')
			if @dest.size <> 16 | @src.size and not 4
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F_W0,6Eh,@src,@dest.rm
		else if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg'
			if @dest.size and not 4 | @src.size <> 16
				err 'operand sizes do not match'
			end if
			AVX.store_instruction 16,VEX_66_0F_W0,7Eh,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vmovq? dest*,src*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @dest.size <> 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' and @src.size and not 8)
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_F3_0F_W0,7Eh,@src,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not 8 | @src.size <> 16
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F_W0,0D6h,@dest,@src.rm
		else if @dest.type = 'mmreg' & @src.type = 'reg'
			if @dest.size <> 16 | @src.size <> 8
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction 16,VEX_66_0F_W1,6Eh,@src,@dest.rm
		else if @dest.type = 'reg' & @src.type = 'mmreg'
			if @dest.size <> 8 | @src.size <> 16
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction 16,VEX_66_0F_W1,7Eh,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vmovddup? dest*,src*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if ((@src.type = 'mmreg' | @dest.size = 32) & @src.size and not @dest.size) | (@src.type = 'mem' & @dest.size = 16 & @src.size and not 8)
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,VEX_F2_0F_W0,12h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode>, vmovhlps,12h, vmovlhps,16h

		macro instr? dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			AVX.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mmreg'
				if @dest.size <> 16
					err 'invalid operand size'
				else if @src.size <> @dest.size | @src2.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction 16,VEX_0F_W0,opcode,@src2,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vmovhpd,VEX_66_0F_W0,16h, vmovhps,VEX_0F_W0,16h, vmovlpd,VEX_66_0F_W0,12h, vmovlps,VEX_0F_W0,12h

		macro instr? dest*,src*,src2
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			match , src2
				if @dest.type = 'mem' & @src.type = 'mmreg'
					if @dest.size and not 8 | @src.size <> 16
						err 'invalid operand size'
					end if
					AVX.store_instruction 16,vex_mpw,opcode+1,@dest,@src.rm
				else
					err 'invalid combination of operands'
				end if
			else
				AVX.parse_operand @src2,src2
				if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
					if @dest.size <> 16 | @src.size <> 16 | @src2.size and not 8
						err 'invalid operand size'
					end if
					AVX.store_instruction 16,vex_mpw,opcode,@src2,@dest.rm,@src.rm
				else
					err 'invalid combination of operands'
				end if
			end match
		end macro

	end iterate

	iterate <instr,vex_mpw>, vmovmskpd,VEX_66_0F_W0, vmovmskps,VEX_0F_W0

		macro instr? dest*,src*
			x86.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'reg' & @src.type = 'mmreg'
				if @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)
					err 'invalid operand size'
				end if
				AVX.store_instruction @src.size,vex_mpw,50h,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vmovntdq,VEX_66_0F_W0,0E7h, vmovntpd,VEX_66_0F_W0,2Bh, vmovntps,VEX_0F_W0,2Bh

		macro instr? dest*,src*
			x86.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'mem' & @src.type = 'mmreg'
				if @dest.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @src.size,vex_mpw,opcode,@dest,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vmovntdqa? dest*,src*
		AVX.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @dest.size <> 16
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX.store_instruction 16,VEX_66_0F38_W0,2Ah,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,vex_mpw,msize>, vmovsd,VEX_F2_0F_W0,8, vmovss,VEX_F3_0F_W0,4

		macro instr? dest*,src*,src2
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			match , src2
				if @dest.type = 'mmreg' & @src.type = 'mem'
					if @dest.size <> 16 | @src.size and not msize
						err 'invalid operand size'
					end if
					AVX.store_instruction 16,vex_mpw,10h,@src,@dest.rm
				else if @dest.type = 'mem' & @src.type = 'mmreg'
					if @dest.size and not msize | @src.size <> 16
						err 'invalid operand size'
					end if
					AVX.store_instruction 16,vex_mpw,11h,@dest,@src.rm
				else
					err 'invalid combination of operands'
				end if
			else
				AVX.parse_operand @src2,src2
				if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mmreg'
					if @dest.size <> 16 | @src.size <> 16 | @src2.size <> 16
						err 'invalid operand size'
					end if
					AVX.store_instruction 16,vex_mpw,10h,@src2,@dest.rm,@src.rm
				else
					err 'invalid combination of operands'
				end if
			end match
		end macro

	end iterate

	macro vmovshdup? dest*,src*
		AVX.single_source_instruction VEX_F3_0F_W0,16h,0,dest,src
	end macro

	macro vmovsldup? dest*,src*
		AVX.single_source_instruction VEX_F3_0F_W0,12h,0,dest,src
	end macro

	macro vperm2f128? dest*,src*,src2*,imm*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,6,32,dest,src,src2,imm
	end macro

	iterate <instr,opcode_rrm,opcode_rri>, vpermilps,0Ch,4, vpermilpd,0Dh,5

		macro instr? dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			AVX.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F38_W0,opcode_rrm,@src2,@dest.rm,@src.rm
			else if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @src2.type = 'imm'
				if @src2.size and not 1
					err 'invalid operand size'
				else if @src.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F3A_W0,opcode_rri,@src,@dest.rm,,1,@src2.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, packsswb,63h, packuswb,67h, packssdw,6Bh, paddb,0FCh, paddw,0FDh, paddd,0FEh, paddq,0D4h, paddsb,0ECh, paddsw,0EDh, paddusb,0DCh, paddusw,0DDh, \
				pand,0DBh, pandn,0DFh, pavgb,0E0h, pavgw,0E3h, pcmpeqb,74h, pcmpeqw,75h, pcmpeqd,76h, pcmpgtb,64h, pcmpgtw,65h, pcmpgtd,66h, \
				pmaddwd,0F5h, pmaxsw,0EEh, pmaxub,0DEh, pminsw,0EAh, pminub,0DAh, pmulhuw,0E4h, pmulhw,0E5h, pmullw,0D5h, pmuludq,0F4h, \
				por,0EBh, psadbw,0F6h, psubb,0F8h, psubw,0F9h, psubd,0FAh, psubq,0FBh, psubsb,0E8h, psubsw,0E9h, psubusb,0D8h, psubusw,0D9h, \
				punpckhbw,68h, punpckhwd,69h, punpckhdq,6Ah, punpckhqdq,6Dh, punpcklbw,60h, punpcklwd,61h, punpckldq,62h, punpcklqdq,6Ch, pxor,0EFh

		macro v#instr? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F_W0,opcode,16,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, packusdw,2Bh, pcmpeqq,29h, pcmpgtq,37h, phaddw,1, phaddd,2, phaddsw,3, phsubw,5, phsubd,6, phsubsw,7, pmaddubsw,4, \
				pmaxsb,3Ch, pmaxsd,3Dh, pmaxuw,3Eh, pmaxud,3Fh, pminsb,38h, pminsd,39h, pminuw,3Ah, pminud,3Bh, pmulhrsw,0Bh, pmulld,40h, pmuldq,28h, \
				pshufb,0, psignb,8, psignw,9, psignd,0Ah

		macro v#instr? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F38_W0,opcode,16,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, mpsadbw,42h, palignr,0Fh

		macro v#instr? dest*,src*,src2*,imm*
			AVX.basic_instruction_imm8 VEX_66_0F3A_W0,opcode,16,dest,src,src2,imm
		end macro

	end iterate

	iterate <instr,opcode>, pabsb,1Ch, pabsw,1Dh, pabsd,1Eh, pblendw,0Eh, phminposuw,41h

		macro v#instr? dest*,src*
			AVX.single_source_instruction VEX_66_0F38_W0,opcode,16,dest,src
		end macro

	end iterate

	iterate <instr,opcode>, pcmpestrm,60h, pcmpestri,61h, pcmpistrm,62h, pcmpistri,63h

		macro v#instr? dest*,src*,imm*
			AVX.single_source_instruction_imm8 VEX_66_0F3A_W0,opcode,16,dest,src,imm
		end macro

	end iterate

	iterate <instr,vex_mpw>, pshufd,VEX_66_0F_W0, pshufhw,VEX_F3_0F_W0, pshuflw,VEX_F2_0F_W0

		macro v#instr? dest*,src*,imm*
			AVX.single_source_instruction_imm8 vex_mpw,70h,16,dest,src,imm
		end macro

	end iterate

	macro vpblendvb? dest*,src*,src2*,mask*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		AVX.parse_operand @src2,src2
		AVX.parse_operand @aux,mask
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'mmreg'
			if @dest.size <> 16
				err 'invalid operand size'
			else if @src.size <> @dest.size | @src2.size and not @dest.size | @aux.size <> @dest.size
				err 'operand sizes do not match'
			end if
			AVX.store_instruction 16,VEX_66_0F3A_W0,4Ch,@src2,@dest.rm,@src.rm,1,(@aux.rm and 1111b) shl 4
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode,msize>, vpextrb,14h,1, vpextrd,16h,4

		macro instr? dest*,src*,aux*
			x86.parse_operand @dest,dest
			AVX.parse_operand @src,src
			x86.parse_operand @aux,aux
			if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
				if (@dest.type = 'reg' & @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | (@dest.type = 'mem' & @dest.size and not msize) | @src.size <> 16 | @aux.size and not 1
					err 'invalid operand size'
				end if
				AVX.store_instruction 16,VEX_66_0F3A_W0,opcode,@dest,@src.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vpextrw? dest*,src*,aux*
		x86.parse_operand @dest,dest
		AVX.parse_operand @src,src
		x86.parse_operand @aux,aux
		if @dest.type = 'reg' & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8) | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F_W0,0C5h,@src,@dest.rm,,1,@aux.imm
		else if @dest.type = 'mem' & @src.type = 'mmreg' & @aux.type = 'imm'
			if  @dest.size and not 2 | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F3A_W0,15h,@dest,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vpextrq? dest*,src*,aux*
		x86.parse_operand @dest,dest
		AVX.parse_operand @src,src
		x86.parse_operand @aux,aux
		if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not 8 | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction 16,VEX_66_0F3A_W1,16h,@dest,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,vex_mpw,opcode,msize>, vpinsrb,VEX_66_0F3A_W0,20h,1, vpinsrw,VEX_66_0F_W0,0C4h,2, vpinsrd,VEX_66_0F3A_W0,22h,4

		macro instr? dest*,src*,src2*,aux*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			x86.parse_operand @src2,src2
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
				if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'reg' & @src2.size <> 4) | (@src2.type = 'mem' & @src2.size and not msize) | @aux.size and not 1
					err 'invalid operand size'
				end if
				AVX.store_instruction 16,vex_mpw,opcode,@src2,@dest.rm,@src.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vpinsrq? dest*,src*,src2*,aux*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		x86.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 16 | @src.size <> 16 | @src2.size and not 8 | @aux.size and not 1
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction 16,VEX_66_0F3A_W1,22h,@src2,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vpmovmskb? dest*,src*
		x86.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'reg' & @src.type = 'mmreg'
			if (@dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | @src.size <> 16
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,VEX_66_0F_W0,0D7h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode,msize>, pmovsxbw,20h,8, pmovsxbd,21h,4, pmovsxbq,22h,2, pmovsxwd,23h,8, pmovsxwq,24h,4, pmovsxdq,25h,8, \
				      pmovzxbw,30h,8, pmovzxbd,31h,4, pmovzxbq,32h,2, pmovzxwd,33h,8, pmovzxwq,34h,4, pmovzxdq,35h,8

		macro v#instr? dest*,src*
			AVX.single_source_instruction VEX_66_0F38_W0,opcode,msize,dest,src
		end macro

	end iterate

	iterate <instr,postbyte>, pslldq,7, psrldq,3

		macro v#instr dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			x86.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'imm'
				if @dest.size <> 16 | @src2.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction 16,VEX_66_0F_W0,73h,@src,postbyte,@dest.rm,1,@src2.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode_rrm,opcode,postbyte>, psllw,0F1h,71h,6, pslld,0F2h,72h,6, psllq,0F3h,73h,6, psraw,0E1h,71h,4, psrad,0E2h,72h,4, psrlw,0D1h,71h,2, psrld,0D2h,72h,2, psrlq,0D3h,73h,2

		macro v#instr? dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			AVX.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @dest.size <> 16 | @src2.size and not 16
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction 16,VEX_66_0F_W0,opcode_rrm,@src2,@dest.rm,@src.rm
			else if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'imm'
				if @dest.size <> 16 | @src2.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction 16,VEX_66_0F_W0,opcode,@src,postbyte,@dest.rm,1,@src2.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vtestps,0Eh, vtestpd,0Fh, vptest,17h

		macro instr? dest*,src*
			AVX.single_source_instruction VEX_66_0F38_W0,opcode,0,dest,src
		end macro

	end iterate

	macro vzeroall?
		db 0C5h,11111100b,77h
	end macro

	macro vzeroupper?
		db 0C5h,11111000b,77h
	end macro

	macro xsaveopt? src*
		x86.parse_operand @src,src
		if @src.type = 'mem'
			x86.store_instruction <0Fh,0AEh>,@src,6
		else
			err 'invalid operand'
		end if
	end macro

end if
