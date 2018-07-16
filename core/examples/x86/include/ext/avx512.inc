
if ~ defined AVX_512

	restore AVX_512 ; this ensures that symbol cannot be forward-referenced
	define AVX_512 1

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

	macro AVX_512.parse_operand ns,op
		ns.mask = 0
		ns.evex_b = 0
		ns.memsize = 0
		x86.parse_operand ns,op
		if ns.type = 'imm' & ns.size = 0 & ns.imm eq 1 elementof ns.imm
			if 1 metadataof ns.imm relativeto SSE.reg
				ns.type = 'mmreg'
				ns.mod = 11b
				ns.rm = 1 metadataof ns.imm - SSE.reg
				ns.size = 16
			else if 1 metadataof ns.imm relativeto AVX.reg
				ns.type = 'mmreg'
				ns.mod = 11b
				ns.rm = 1 metadataof ns.imm - AVX.reg
				ns.size = 32
			else if 1 metadataof (1 metadataof ns.imm) relativeto AVX_512.reg
				ns.type = 'mmreg'
				ns.mod = 11b
				ns.rm = 1 metadataof ns.imm - 1 elementof (1 metadataof ns.imm)
				ns.size = 1 metadataof (1 metadataof ns.imm) - AVX_512.reg
			else if 1 metadataof ns.imm relativeto AVX_512.maskreg
				ns.type = 'maskreg'
				ns.mod = 11b
				ns.rm = 1 metadataof ns.imm - 1 elementof (1 metadataof ns.imm)
				ns.size = 8
			end if
		end if
		if ns.type = 'reg' & ns.size = 1 & ns.rm >= 4 & (~ defined x86.REX_FORBIDDEN | ns.rm and x86.REX_FORBIDDEN)
			err 'invalid operand'
		end if
	end macro

	macro AVX_512.parse_operand_k1z ns,opmask
		match op {k1} { =z? }, opmask
			AVX_512.parse_operand ns,op
			if ns.type <> 'mem' & k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
				ns.mask = (1 metadataof k1 - AVX_512.maskreg) or 80h
			else
				err 'invalid mask'
			end if
		else match op {k1}, opmask
			AVX_512.parse_operand ns,op
			if k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
				ns.mask = 1 metadataof k1 - AVX_512.maskreg
			else
				err 'invalid mask'
			end if
		else
			AVX_512.parse_operand ns,opmask
		end match
	end macro

	macro AVX_512.parse_operand_k1 ns,opmask
		match op {k1}, opmask
			AVX_512.parse_operand ns,op
			if k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
				ns.mask = 1 metadataof k1 - AVX_512.maskreg
			else
				err 'invalid mask'
			end if
		else
			AVX_512.parse_operand ns,opmask
		end match
	end macro

	macro AVX_512.parse_operand_bcst ns,opbcst,unit
		match op {bcst}, opbcst
			AVX_512.parse_operand ns,op
			if ns.type = 'mem'
				ns.memsize = unit
				if ns.memsize = 0
					if ns.size
						ns.memsize = ns.size
					else
						err 'operand size not specified'
					end if
				else if ns.size and not ns.memsize
					err 'invalid operand size'
				end if
				match =1to2, bcst
					ns.broadcast = 2
				else match =1to4, bcst
					ns.broadcast = 4
				else match =1to8, bcst
					ns.broadcast = 8
				else match =1to16, bcst
					ns.broadcast = 16
				else
					err 'invalid broadcast'
				end match
				ns.evex_b = 1
				ns.size = ns.memsize * ns.broadcast
			else
				err = 'invalid operand'
			end if
		else
			AVX_512.parse_operand ns,opbcst
		end match
	end macro

	macro AVX_512.parse_er ns,op,vsize:64
		if ns.type = 'mem' | ns.size <> vsize
			err 'invalid operand'
		else match { =rn?-=sae? }, op
			ns.evex_b = 1
			ns.rounding = 0
		else match { =rd?-=sae? }, op
			ns.evex_b = 1
			ns.rounding = 1
		else match { =ru?-=sae? }, op
			ns.evex_b = 1
			ns.rounding = 2
		else match { =rz?-=sae? }, op
			ns.evex_b = 1
			ns.rounding = 3
		else
			err 'invalid operand'
		end match
	end macro

	macro AVX_512.parse_sae ns,op,vsize:64
		if ns.type = 'mem' | ns.size <> vsize
			err 'invalid operand'
		else match { =sae? }, op
			ns.evex_b = 1
			ns.rounding = 0
		else
			err 'invalid operand'
		end match
	end macro

	macro AVX_512.store_instruction vsize*,vex_mpw*,evex_f*,opcode*,rm_operand*,mask*,reg*,vreg:0,imm_size:0,imm

		if rm_operand.segment_prefix
			if x86.mode = 64
				if rm_operand.segment_prefix >= 64h
					db rm_operand.segment_prefix
				end if
			else if rm_operand.mode = 16 & ( rm_operand.rm = 2 | rm_operand.rm = 3 | ( rm_operand.mod > 0 & rm_operand.rm = 6 ) )
				if rm_operand.segment_prefix <> 36h
					db rm_operand.segment_prefix
				end if
			else if rm_operand.mode = 32 & ( ( rm_operand.mod > 0 & rm_operand.rm = 5 ) | ( rm_operand.rm = 4 & rm_operand.base = 4 ) | ( rm_operand.mod > 0 & rm_operand.rm = 4 & rm_operand.base = 5 ) )
				if rm_operand.segment_prefix <> 36h
					db rm_operand.segment_prefix
				end if
			else if rm_operand.segment_prefix <> 3Eh
				db rm_operand.segment_prefix
			end if
		end if
		if rm_operand.mod <> 11b & rm_operand.mode <> x86.mode
			if rm_operand.mode = 64 | (rm_operand.mode = 16 & x86.mode = 64)
				err 'illegal addressing mode'
			end if
			db 67h
		end if

		rm_operand.evex = vex_mpw

		if rm_operand.evex_b
			rm_operand.evex = rm_operand.evex or rm_operand.evex_b shl 20
		end if

		if rm_operand.mod = 11b & rm_operand.evex_b
			rm_operand.evex = rm_operand.evex or (rm_operand.rounding shl 21)
		else if vsize = 64
			rm_operand.evex = rm_operand.evex or (1 shl 22)
		else if vsize = 32
			rm_operand.evex = rm_operand.evex or (1 shl 21)
		end if

		if rm_operand.rm and 10000b | (rm_operand.mod <> 11b & rm_operand.mode > 16 & rm_operand.rm = 4 & rm_operand.index and 1000b)
			rm_operand.evex = rm_operand.evex or (1 shl 6)
		end if
		if rm_operand.rm and 1000b | (rm_operand.mod <> 11b & rm_operand.mode > 16 & rm_operand.rm = 4 & rm_operand.base and 1000b)
			rm_operand.evex = rm_operand.evex or (1 shl 5)
		end if
		if reg and 10000b
			rm_operand.evex = rm_operand.evex or (1 shl 4)
		end if
		if reg and 1000b
			rm_operand.evex = rm_operand.evex or (1 shl 7)
		end if
		if vreg and 10000b
			rm_operand.evex = rm_operand.evex or (1 shl 19)
		end if
		rm_operand.evex = rm_operand.evex or (vreg and 1111b) shl 11

		if x86.mode < 64 & rm_operand.evex and 00001000_01000000_11110000b
			err 'instruction requires long mode'
		end if

		if mask
			rm_operand.evex = rm_operand.evex or mask shl 16
		end if

		if rm_operand.memsize = 0
			rm_operand.memsize = vsize
		end if

		if rm_operand.mod <> 11b & rm_operand.mod <> 0
			if rm_operand.mode > 16
				rm_operand.evex_displacement_size = 4
			else
				rm_operand.evex_displacement_size = 2
			end if
			if rm_operand.displacement relativeto 0 & rm_operand.displacement mod rm_operand.memsize = 0
				rm_operand.compressed_displacement = rm_operand.displacement / rm_operand.memsize
				if rm_operand.compressed_displacement < 80h & rm_operand.compressed_displacement >= -80h
					rm_operand.evex_displacement_size = 1
				else if rm_operand.compressed_displacement - 1 shl rm_operand.mode >= -80h & rm_operand.compressed_displacement < 1 shl rm_operand.mode
					rm_operand.compressed_displacement = rm_operand.compressed_displacement - 1 shl rm_operand.mode
					rm_operand.evex_displacement_size = 1
				end if
			end if
		else
			rm_operand.evex_displacement_size = rm_operand.displacement_size
		end if

		if evex_f and EVEX_REQUIRED | rm_operand.evex and 11011111_00000000_00010000b | rm_operand.rm and 10000b
			if evex_f and EVEX_FORBIDDEN
				err 'invalid operand'
			else
				rm_operand.evex = rm_operand.evex or 1 shl 10
			end if
		else if ~ evex_f and EVEX_FORBIDDEN & rm_operand.evex_displacement_size + 1 < rm_operand.displacement_size
			rm_operand.evex = rm_operand.evex or 1 shl 10
		end if

		if rm_operand.evex and 1 shl 10
			if evex_f and EVEX_W1
				rm_operand.evex = rm_operand.evex or 1 shl 15
			end if
			dd 62h + (rm_operand.evex xor 00001000_01111000_11110000b) shl 8
			if rm_operand.mod <> 11b & rm_operand.mod <> 0 & rm_operand.evex_displacement_size > 0
				rm_operand.displacement_size = rm_operand.evex_displacement_size
				if rm_operand.evex_displacement_size = 1
					rm_operand.displacement = rm_operand.compressed_displacement
					rm_operand.mod = 1
				else
					rm_operand.mod = 2
				end if
			end if
		else
			rm_operand.vex = rm_operand.evex and 11111011_11111111b or (rm_operand.evex and 1 shl 21) shr (21-10)
			if rm_operand.vex and 10000000_01111111b <> 1
				db 0C4h,(rm_operand.vex and 11111111b) xor 11100000b,(rm_operand.vex shr 8) xor 01111000b
			else
				db 0C5h,((rm_operand.vex and 10000000b) or ((rm_operand.vex shr 8) and 1111111b)) xor 11111000b
			end if
		end if

		db opcode, rm_operand.mod shl 6 + (reg and 111b) shl 3 + rm_operand.rm and 111b
		if rm_operand.mod <> 11b & rm_operand.rm = 4 & rm_operand.mode <> 16
			db (bsf rm_operand.scale) shl 6 + (rm_operand.index and 111b) shl 3 + (rm_operand.base and 111b)
		end if
		if rm_operand.displacement_size = 1
			db rm_operand.displacement
		else if rm_operand.displacement_size = 2
			dw rm_operand.displacement
		else if rm_operand.displacement_size = 4
			if defined rm_operand.auto_relative & rm_operand.auto_relative
				if imm_size < 8
					rm_operand.displacement = rm_operand.displacement - ($ + 4 + imm_size)
				else
					rm_operand.displacement = rm_operand.displacement - ($ + 4 + 4)
				end if
			end if
			if rm_operand.mode = 64 & rm_operand.displacement relativeto 0
				if rm_operand.displacement - 1 shl 64 >= -80000000h & rm_operand.displacement < 1 shl 64
					rm_operand.displacement = rm_operand.displacement - 1 shl 64
				else if rm_operand.displacement < -80000000h | rm_operand.displacement >= 80000000h
					err 'address value out of signed range'
				end if
			end if
			dd rm_operand.displacement
		else if rm_operand.displacement_size = 8
			err 'long address not encodable'
		end if
		if imm_size = 1
			db imm
		else if imm_size = 2
			dw imm
		else if imm_size = 4
			dd imm
		else if imm_size = 8
			x86.simm32 imm
		end if
	end macro

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
			AVX_512.parse_sae @src2,sae,(unit-1) and not 15 + 16
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
			AVX_512.parse_sae @src2,sae,(unit-1) and not 15 + 16
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
			AVX_512.basic_instruction_bcst_er VEX_66_0F_W0,EVEX_W1,opcode,8,dest,src,src2
		end macro

		macro v#instr#ps? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst_er VEX_0F_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
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
			AVX_512.basic_instruction_bcst_sae VEX_66_0F_W0,EVEX_W1,opcode,8,dest,src,src2
		end macro

		macro v#instr#ps? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst_sae VEX_0F_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
		end macro

		macro v#instr#sd? dest*,src*,src2*&
			AVX_512.basic_instruction_sae VEX_F2_0F_W0,EVEX_W1,opcode,8,dest,src,src2
		end macro

		macro v#instr#ss? dest*,src*,src2*&
			AVX_512.basic_instruction_sae VEX_F3_0F_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, and,54h, andn,55h, or,56h, unpckh,15h, unpckl,14h, xor,57h

		macro v#instr#pd? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_W1,opcode,8,dest,src,src2
		end macro

		macro v#instr#ps? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst VEX_0F_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
		end macro

	end iterate

	macro vsqrtpd? dest*,src*&
		AVX_512.single_source_instruction_bcst_er VEX_66_0F_W0,EVEX_W1,51h,8,dest,src
	end macro

	macro vsqrtps? dest*,src*&
		AVX_512.single_source_instruction_bcst_er VEX_0F_W0,EVEX_AS_VEX,51h,4,dest,src
	end macro

	macro vsqrtsd? dest*,src*,src2*&
		AVX_512.basic_instruction_er VEX_F2_0F_W0,EVEX_W1,51h,8,dest,src,src2
	end macro

	macro vsqrtss? dest*,src*,src2*&
		AVX_512.basic_instruction_er VEX_F3_0F_W0,EVEX_AS_VEX,51h,4,dest,src,src2
	end macro

	macro vshufpd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_imm8 VEX_66_0F_W0,EVEX_W1,0C6h,8,dest,src,src2,aux
	end macro

	macro vshufps? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_imm8 VEX_0F_W0,EVEX_AS_VEX,0C6h,4,dest,src,src2,aux
	end macro

	macro vbroadcastss? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 4)
				err 'invalid operand size'
			end if
			@src2.memsize = 4
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX,18h,@src,@dest.mask,@dest.rm
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
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_W1,19h,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode,opcode_g,msize>, vpbroadcastb,78h,7Ah,1, vpbroadcastw,79h,7Bh,2, vpbroadcastd,58h,7Ch,4, vpbroadcastq,59h,7Ch,8

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
					AVX_512.store_instruction @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED,opcode_g,@src,@dest.mask,@dest.rm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED,opcode_g,@src,@dest.mask,@dest.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vbroadcastf32x2,19h, vbroadcasti32x2,59h

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
				if @dest.size = 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 8)
					err 'invalid operand size'
				end if
				@src.memsize = 8
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED,opcode,@src,@dest.mask,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode,msize>, vbroadcastf32x4,VEX_66_0F38_W0,1Ah,16, vbroadcastf32x8,VEX_66_0F38_W0,1Bh,32, vbroadcastf64x2,VEX_66_0F38_W1,1Ah,16, vbroadcastf64x4,VEX_66_0F38_W1,1Bh,32, \
					      vbroadcasti32x4,VEX_66_0F38_W0,5Ah,16, vbroadcasti32x8,VEX_66_0F38_W0,5Bh,32, vbroadcasti64x2,VEX_66_0F38_W1,5Ah,16, vbroadcasti64x4,VEX_66_0F38_W1,5Bh,32

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & @src.type = 'mem'
				if @dest.size <= msize | @src.size and not msize
					err 'invalid operand size'
				end if
				@src.memsize = msize
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED,opcode,@src,@dest.mask,@dest.rm
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
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
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

	iterate <instr,vex_mpw,opcode,msize>, vextractf32x4,VEX_66_0F3A_W0,19h,16, vextractf32x8,VEX_66_0F3A_W0,1Bh,32, vextractf64x2,VEX_66_0F3A_W1,19h,16, vextractf64x4,VEX_66_0F3A_W1,1Bh,32, \
					      vextracti32x4,VEX_66_0F3A_W0,39h,16, vextracti32x8,VEX_66_0F3A_W0,3Bh,32, vextracti64x2,VEX_66_0F3A_W1,39h,16, vextracti64x4,VEX_66_0F3A_W1,3Bh,32

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			x86.parse_operand @aux,aux
			if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
				if @dest.size and not msize | @src.size <= msize | @aux.size and not 1
					err 'invalid operand size'
				end if
				@dest.memsize = msize
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,opcode,@dest,@dest.mask,@src.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode,msize>, vinsertf32x4,VEX_66_0F3A_W0,18h,16, vinsertf32x8,VEX_66_0F3A_W0,1Ah,32, vinsertf64x2,VEX_66_0F3A_W1,18h,16, vinsertf64x4,VEX_66_0F3A_W1,1Ah,32, \
					      vinserti32x4,VEX_66_0F3A_W0,38h,16, vinserti32x8,VEX_66_0F3A_W0,3Ah,32, vinserti64x2,VEX_66_0F3A_W1,38h,16, vinserti64x4,VEX_66_0F3A_W1,3Ah,32

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
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
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
				AVX_512.store_instruction @src.size,evex_mpw,EVEX_REQUIRED,0C2h,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
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
				AVX_512.parse_sae @src2,sae,16
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
				AVX_512.parse_sae @src,sae,(unit-1) and not 15 + 16
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

	iterate <t,vex_mpw,msize>, b,VEX_66_0F_W0,1, w,VEX_0F_W0,2, d,VEX_66_0F_W1,4, q,VEX_0F_W1,8

		iterate <instr,opcode>, kand,41h, kandn,42h, knot,44h, kor,45h, kxnor,46h, kxor,47h, kadd,4Ah

			macro instr#t? dest*,src*,src2*
				AVX_512.parse_operand @dest,dest
				AVX_512.parse_operand @src,src
				AVX_512.parse_operand @src2,src2
				if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
					AVX.store_instruction 32,vex_mpw,opcode,@src2,@dest.rm,@src.rm
				else
					err 'invalid combination of operands'
				end if
			end macro

		end iterate

		iterate <instr,opcode>, knot,44h, kortest,98h, ktest,99h

			macro instr#t? dest*,src*
				AVX_512.parse_operand @dest,dest
				AVX_512.parse_operand @src,src
				if @dest.type = 'maskreg' & @src.type = 'maskreg'
					AVX.store_instruction 16,vex_mpw,opcode,@src,@dest.rm
				else
					err 'invalid combination of operands'
				end if
			end macro

		end iterate

		macro kmov#t? dest*,src*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'maskreg' & (@src.type = 'maskreg' | @src.type = 'mem')
				if @src.type = 'mem' & @src.size and not msize
					err 'invalid operand size'
				end if
				AVX.store_instruction 16,vex_mpw,90h,@src,@dest.rm
			else if @dest.type = 'mem' & @src.type = 'maskreg'
				if @dest.size and not msize
					err 'invalid operand size'
				end if
				AVX.store_instruction 16,vex_mpw,91h,@dest,@src.rm
			else if @dest.type = 'maskreg' & @src.type = 'reg'
				if (msize < 8 & @src.size <> 4) | (msize = 8 & @src.size <> 8)
					err 'invalid operand size'
				else if msize = 8 & x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX.store_instruction 16,vex_mpw,92h,@src,@dest.rm
			else if @dest.type = 'reg' & @src.type = 'maskreg'
				if (msize < 8 & @dest.size <> 4) | (msize = 8 & @dest.size <> 8)
					err 'invalid operand size'
				else if msize = 8 & x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX.store_instruction 16,vex_mpw,93h,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, kshiftrb,VEX_66_0F3A_W0,30h, kshiftrw,VEX_66_0F3A_W1,30h, kshiftrd,VEX_66_0F3A_W0,31h, kshiftrq,VEX_66_0F3A_W1,31h, \
					kshiftlb,VEX_66_0F3A_W0,32h, kshiftlw,VEX_66_0F3A_W1,32h, kshiftld,VEX_66_0F3A_W0,33h, kshiftlq,VEX_66_0F3A_W1,33h

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

	iterate <instr,vex_mpw>, kunpckbw,VEX_66_0F_W0, kunpckwd,VEX_0F_W0, kunpckdq,VEX_0F_W1

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
				AVX.store_instruction 32,vex_mpw,4Bh,@src2,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,evex_f,opcode>, vcvtdq2pd,EVEX_AS_VEX,0E6h, vcvtudq2pd,EVEX_REQUIRED,7Ah

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

	iterate <instr,vex_mpw,evex_f,opcode>, vcvtps2qq,VEX_66_0F_W0,EVEX_REQUIRED,7Bh, vcvtps2uqq,VEX_66_0F_W0,EVEX_REQUIRED,79h

		macro instr? dest*,src_er*&
			AVX_512.parse_operand_k1z @dest,dest
			match src=,er, src_er
				AVX_512.parse_operand @src,src
				AVX_512.parse_er @src,er,32
			else
				AVX_512.parse_operand_bcst @src,src_er,4
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

	iterate <instr,vex_mpw,evex_f,opcode>, vcvtpd2dq,VEX_F2_0F_W0,EVEX_W1,0E6h, vcvtpd2ps,VEX_66_0F_W0,EVEX_W1,5Ah, vcvtpd2udq,VEX_0F_W1,EVEX_REQUIRED,79h, \
					       vcvtqq2ps,VEX_0F_W1,EVEX_REQUIRED,5Bh, vcvtuqq2ps,VEX_F2_0F_W1,EVEX_REQUIRED,7Ah

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

	iterate <instr,vex_mpw,evex_f,opcode>, vcvtps2pd,VEX_0F_W0,EVEX_AS_VEX,5Ah, vcvttps2qq,VEX_66_0F_W0,EVEX_REQUIRED,7Ah, vcvttps2uqq,VEX_66_0F_W0,EVEX_REQUIRED,78h

		macro instr? dest*,src_sae*&
			AVX_512.parse_operand_k1z @dest,dest
			match src=,sae, src_sae
				AVX_512.parse_operand @src,src
				AVX_512.parse_sae @src,sae,32
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

	iterate <instr,vex_mpw,evex_f,opcode>, vcvttpd2dq,VEX_66_0F_W0,EVEX_W1,0E6h, vcvttpd2udq,VEX_0F_W1,EVEX_REQUIRED,78h

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

	iterate <instr,vex_mpw,evex_f,opcode>, vcvtdq2ps,VEX_0F_W0,EVEX_AS_VEX,5Bh, vcvtudq2ps,VEX_F2_0F_W0,EVEX_REQUIRED,7Ah, \
					       vcvtps2dq,VEX_66_0F_W0,EVEX_AS_VEX,5Bh, vcvtps2udq,VEX_0F_W0,EVEX_REQUIRED,79h

		macro instr? dest*,src*&
			AVX_512.single_source_instruction_bcst_er vex_mpw,evex_f,opcode,4,dest,src
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vcvtpd2qq,VEX_66_0F_W1,EVEX_REQUIRED,7Bh, vcvtpd2uqq,VEX_66_0F_W1,EVEX_REQUIRED,79h, \
					       vcvtqq2pd,VEX_F3_0F_W1,EVEX_REQUIRED,0E6h, vcvtuqq2pd,VEX_F3_0F_W1,EVEX_REQUIRED,7Ah

		macro instr? dest*,src*&
			AVX_512.single_source_instruction_bcst_er vex_mpw,evex_f,opcode,8,dest,src
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vcvttps2dq,VEX_F3_0F_W0,EVEX_AS_VEX,5Bh, vcvttps2udq,VEX_0F_W0,EVEX_REQUIRED,78h

		macro instr? dest*,src*&
			AVX_512.single_source_instruction_bcst_sae vex_mpw,evex_f,opcode,4,dest,src
		end macro

	end iterate

	iterate <instr,vex_mpw,evex_f,opcode>, vcvttpd2qq,VEX_66_0F_W1,EVEX_REQUIRED,7Ah, vcvttpd2uqq,VEX_66_0F_W1,EVEX_REQUIRED,78h

		macro instr? dest*,src*&
			AVX_512.single_source_instruction_bcst_sae vex_mpw,evex_f,opcode,8,dest,src
		end macro

	end iterate

	macro vcvtph2ps? dest*,src_sae*&
		AVX_512.parse_operand_k1z @dest,dest
		match src=,sae, src_sae
			AVX_512.parse_operand @src,src
			AVX_512.parse_sae @src,sae,32
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
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX,13h,@src,@dest.mask,@dest.rm
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
			AVX_512.store_instruction @src.size,VEX_66_0F3A_W0,EVEX_AS_VEX,1Dh,@dest,@dest.mask,@src.rm,,1,@aux.imm
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
				AVX_512.parse_sae @src,sae,16
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

	iterate <instr,evex_f,opcode>, vcvtsi2sd,EVEX_AS_VEX,2Ah, vcvtusi2ss,EVEX_REQUIRED,7Bh

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

	iterate <instr,vex_mpw,evex_f,opcode_rm,opcode_mr>, vmovapd,VEX_66_0F_W0,EVEX_W1,28h,29h, vmovaps,VEX_0F_W0,EVEX_AS_VEX,28h,29h, vmovupd,VEX_66_0F_W0,EVEX_W1,10h,11h, vmovups,VEX_0F_W0,EVEX_AS_VEX,10h,11h, \
							    vmovdqa32,VEX_66_0F_W0,EVEX_REQUIRED,6Fh,7Fh, vmovdqa64,VEX_66_0F_W1,EVEX_REQUIRED,6Fh,7Fh, \
							    vmovdqu8,VEX_F2_0F_W0,EVEX_REQUIRED,6Fh,7Fh, vmovdqu16,VEX_F2_0F_W1,EVEX_REQUIRED,6Fh,7Fh, \
							    vmovdqu32,VEX_F3_0F_W0,EVEX_REQUIRED,6Fh,7Fh, vmovdqu64,VEX_F3_0F_W1,EVEX_REQUIRED,6Fh,7Fh

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
			AVX_512.store_instruction @dest.size,VEX_F2_0F_W0,EVEX_W1,12h,@src,@dest.mask,@dest.rm
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

	iterate <instr,vex_mpw,evex_f,opcode>, vmovntdq,VEX_66_0F_W0,EVEX_AS_VEX,0E7h, vmovntpd,VEX_66_0F_W0,EVEX_W1,2Bh, vmovntps,VEX_0F_W0,EVEX_AS_VEX,2Bh

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
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX,2Ah,@src,0,@dest.rm
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
		AVX_512.single_source_instruction VEX_F3_0F_W0,EVEX_AS_VEX,16h,0,dest,src
	end macro

	macro vmovsldup? dest*,src*
		AVX_512.single_source_instruction VEX_F3_0F_W0,EVEX_AS_VEX,12h,0,dest,src
	end macro

	iterate <instr,unit,evex_f,opcode_rrm,opcode_rri>, vpermilps,4,EVEX_AS_VEX,0Ch,4, vpermilpd,8,EVEX_W1,0Dh,5

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

	iterate <instr,opcode>, vpacksswb,63h, vpackuswb,67h, vpaddb,0FCh, vpaddw,0FDh,  paddsb,0ECh, vpaddsw,0EDh, vpaddusb,0DCh, vpaddusw,0DDh, vpavgb,0E0h, vpavgw,0E3h, \
				vpmaddwd,0F5h, vpmaxsw,0EEh, vpmaxub,0DEh, vpminsw,0EAh, vpminub,0DAh, vpmulhuw,0E4h, vpmulhw,0E5h, vpmullw,0D5h, \
				vpsadbw,0F6h, vpsubb,0F8h, vpsubw,0F9h,  psubsb,0E8h, vpsubsw,0E9h, vpsubusb,0D8h, vpsubusw,0D9h, \
				vpunpckhbw,68h, vpunpckhwd,69h, vpunpcklbw,60h, vpunpcklwd,61h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction VEX_66_0F_W0,EVEX_AS_VEX,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpackssdw,6Bh, vpaddd,0FEh, vpsubd,0FAh, vpunpckhdq,6Ah, vpunpckldq,62h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpaddq,0D4h, vpmuludq,0F4h, vpsubq,0FBh, vpunpckhqdq,6Dh, vpunpcklqdq,6Ch

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_W1,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpandd,0DBh, vpandnd,0DFh, vpord,0EBh, vpxord,0EFh

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_REQUIRED,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpandq,0DBh, vpandnq,0DFh, vporq,0EBh, vpxorq,0EFh

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F_W1,EVEX_REQUIRED,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpmaddubsw,4, vpmaxsb,3Ch, vpmaxuw,3Eh, vpminsb,38h, vpminuw,3Ah, vpmulhrsw,0Bh, vpshufb,0

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_AS_VEX,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpmadd52luq,0B4h, vpmadd52huq,0B5h, vpmultishiftqb,83h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpackusdw,2Bh, vpmaxsd,3Dh, vpmaxud,3Fh, vpminsd,39h, vpminud,3Bh, vpmulld,40h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpmuldq,28h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_W1,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpmuldq,28h, vpmaxsq,3Dh, vpmaxuq,3Fh, vpminsq,39h, vpminuq,3Bh, vpmullq,40h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpalignr,0Fh

		macro instr? dest*,src*,src2*,imm*
			AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_AS_VEX,opcode,0,dest,src,src2,imm
		end macro

	end iterate

	iterate <instr,opcode>, vpabsb,1Ch, vpabsw,1Dh

		macro instr? dest*,src*
			AVX_512.single_source_instruction VEX_66_0F38_W0,EVEX_AS_VEX,opcode,0,dest,src
		end macro

	end iterate

	iterate <instr,opcode>, vpabsd,1Eh

		macro instr? dest*,src*
			AVX_512.single_source_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX,opcode,4,dest,src
		end macro

	end iterate

	iterate <instr,opcode>, vpabsq,1Fh

		macro instr? dest*,src*
			AVX_512.single_source_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED,opcode,8,dest,src
		end macro

	end iterate

	iterate <instr,vex_mpw>, vpshufd,VEX_66_0F_W0, vpshufhw,VEX_F3_0F_W0, vpshuflw,VEX_F2_0F_W0

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
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_AS_VEX,70h,@src,@dest.mask,@dest.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpsllvw,12h, vpsrlvw,10h, vpsravw,11h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction VEX_66_0F38_W1,EVEX_REQUIRED,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vpsllvd,47h, vpsrlvd,45h, vpsravd,46h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,evex_f,opcode>, vpsllvq,EVEX_AS_VEX,47h, vpsrlvq,EVEX_AS_VEX,45h, vpsravq,EVEX_REQUIRED,46h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst VEX_66_0F38_W1,evex_f,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vpermb,VEX_66_0F38_W0,8Dh, vpermw,VEX_66_0F38_W1,8Dh, \
					vpermi2b,VEX_66_0F38_W0,75h, vpermi2w,VEX_66_0F38_W1,75h, \
					vpermt2b,VEX_66_0F38_W0,7Dh, vpermt2w,VEX_66_0F38_W1,7Dh

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,unit,vex_mpw,opcode>, vpermi2d,4,VEX_66_0F38_W0,76h, vpermi2q,8,VEX_66_0F38_W1,76h, \
					     vpermt2d,4,VEX_66_0F38_W0,7Eh, vpermt2q,8,VEX_66_0F38_W1,7Eh, \
					     vprorvd,4,VEX_66_0F38_W0,14h, vprorvq,8,VEX_66_0F38_W1,14h, \
					     vprolvd,4,VEX_66_0F38_W0,15h, vprolvq,8,VEX_66_0F38_W1,15h

		macro instr? dest*,src*,src2*
			AVX_512.basic_instruction_bcst vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src,src2
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
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED,72h,@src,@dest.mask,postbyte,@dest.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,postbyte>, vpslldq,7, vpsrldq,3

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @aux.type = 'imm'
				if @aux.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				if @src.type = 'mem'
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_REQUIRED,73h,@src,0,postbyte,@dest.rm,1,@aux.imm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX,73h,@src,0,postbyte,@dest.rm,1,@aux.imm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode_rrm,opcode,postbyte>, vpsllw,0F1h,71h,6, vpsraw,0E1h,71h,4, vpsrlw,0D1h,71h,2

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
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
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_REQUIRED,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				end if
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
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_REQUIRED,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
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
					AVX_512.store_instruction @dest.size,VEX_66_0F_W1,EVEX_REQUIRED,opcode_rrm,@src2,@dest.mask,@dest.rm,@src.rm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_W1,opcode_rrm,@src2,@dest.mask,@dest.rm,@src.rm
				end if
			else if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @src2.type = 'imm'
				if @src2.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				if @src.type = 'mem' | `instr = 'vpsraq'
					AVX_512.store_instruction @dest.size,VEX_66_0F_W1,EVEX_REQUIRED,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				else
					AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_W1,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
				end if
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, vpcmpeqb,74h, pcmpeqw,75h, vpcmpgtb,64h, vpcmpgtw,65h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src.size and not @src2.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,VEX_66_0F_W0,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm
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

	iterate <instr,opcode>, vpcmpeqd,76h, vpcmpgtd,66h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand_bcst @src2,src2,4
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src.size and not @src2.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,VEX_66_0F_W0,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm
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
				if @src.size and not @src2.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,VEX_66_0F38_W1,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm
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

	iterate <instr,vex_mpw,opcode>, vptestnmb,VEX_F3_0F38_W0,26h, vptestnmw,VEX_F3_0F38_W1,26h, vptestmb,VEX_66_0F38_W0,26h, vptestmw,VEX_66_0F38_W1,26h

		macro instr? dest*,src*,src2*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @src.size and not @src2.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm
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
				if @src.size and not @src2.size
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vpcmpb,VEX_66_0F3A_W0,3Fh, vpcmpub,VEX_66_0F3A_W0,3Eh, vpcmpw,VEX_66_0F3A_W1,3Fh, vpcmpuw,VEX_66_0F3A_W1,3Eh

		macro instr? dest*,src*,src2*,aux*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			x86.parse_operand @aux,aux
			if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
				if @src.size and not @src2.size | @aux.size and not 1
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
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
				if @src.size and not @src2.size | @aux.size and not 1
					err 'operand sizes do not match'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode,msize>, vpextrb,14h,1, vpextrd,16h,4

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			x86.parse_operand @aux,aux
			if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
				if (@dest.type = 'reg' & @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | (@dest.type = 'mem' & @dest.size and not msize) | @src.size <> 16 | @aux.size and not 1
					err 'invalid operand size'
				end if
				@dest.memsize = msize
				AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,opcode,@dest,0,@src.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vpextrw? dest*,src*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if @dest.type = 'reg' & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8) | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX_512.store_instruction 16,VEX_66_0F_W0,EVEX_AS_VEX,0C5h,@src,0,@dest.rm,,1,@aux.imm
		else if @dest.type = 'mem' & @src.type = 'mmreg' & @aux.type = 'imm'
			if  @dest.size and not 2 | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			@dest.memsize = 2
			AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,15h,@dest,0,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vpextrq? dest*,src*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not 8 | @src.size <> 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			@dest.memsize = 8
			AVX_512.store_instruction 16,VEX_66_0F3A_W1,EVEX_AS_VEX,16h,@dest,0,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,vex_mpw,opcode,msize>, vpinsrb,VEX_66_0F3A_W0,20h,1, vpinsrw,VEX_66_0F_W0,0C4h,2, vpinsrd,VEX_66_0F3A_W0,22h,4

		macro instr? dest*,src*,src2*,aux*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			x86.parse_operand @aux,aux
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
				if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'reg' & @src2.size <> 4) | (@src2.type = 'mem' & @src2.size and not msize) | @aux.size and not 1
					err 'invalid operand size'
				end if
				@src2.memsize = msize
				AVX_512.store_instruction 16,vex_mpw,EVEX_AS_VEX,opcode,@src2,0,@dest.rm,@src.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vpinsrq? dest*,src*,src2*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 16 | @src.size <> 16 | @src2.size and not 8 | @aux.size and not 1
				err 'invalid operand size'
			end if
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			@src2.memsize = 8
			AVX_512.store_instruction 16,VEX_66_0F3A_W1,EVEX_AS_VEX,22h,@src2,0,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode,msize>, vpmovsxbw,20h,8, vpmovsxbd,21h,4, vpmovsxbq,22h,2, vpmovsxwd,23h,8, vpmovsxwq,24h,4, vpmovsxdq,25h,8, \
				      vpmovzxbw,30h,8, vpmovzxbd,31h,4, vpmovzxbq,32h,2, vpmovzxwd,33h,8, vpmovzxwq,34h,4, vpmovzxdq,35h,8

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				@src.memsize = msize * (@dest.size shr 4)
				if (@src.type = 'mmreg' & @src.size <> (@src.memsize-1) and not 15 + 16) | (@src.type = 'mem' & @src.size and not @src.memsize)
					err 'invalid operand size'
				end if
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX,opcode,@src,@dest.mask,@dest.rm
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
				AVX_512.store_instruction @dest.size,VEX_66_0F3A_W1,EVEX_AS_VEX,opcode,@src,@dest.mask,@dest.rm,,1,@aux.imm
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
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX,opcode,@src2,@dest.mask,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,lcode>, vfmaddsub,6, vfmsubadd,7, vfmadd,8, vfmsub,0Ah, vfnmadd,0Ch, vfnmsub,0Eh

		iterate <order,hcode>, 132,90h, 213,0A0h, 231,0B0h

			macro instr#order#pd? dest*,src*,src2*&
				AVX_512.basic_instruction_bcst_er VEX_66_0F38_W1,EVEX_AS_VEX,hcode+lcode,8,dest,src,src2
			end macro

			macro instr#order#ps? dest*,src*,src2*&
				AVX_512.basic_instruction_bcst_er VEX_66_0F38_W0,EVEX_AS_VEX,hcode+lcode,4,dest,src,src2
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
			AVX_512.basic_instruction_bcst_imm8 vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src,src2,aux
		end macro

	end iterate

	iterate <instr,opcode>, valignq,3

		macro instr? dest*,src*,src2*,aux*&
			AVX_512.basic_instruction_bcst_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED,opcode,8,dest,src,src2,aux
		end macro

	end iterate

	iterate <instr,opcode>, vblendmps,65h, vpblendmd,64h

		macro instr? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_REQUIRED,opcode,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, vblendmpd,65h, vpblendmq,64h

		macro instr? dest*,src*,src2*&
			AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED,opcode,8,dest,src,src2
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vpblendmb,VEX_66_0F38_W0,66h, vpblendmw,VEX_66_0F38_W1,66h

		macro instr? dest*,src*,src2*&
			AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_REQUIRED,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vdbpsadbw,VEX_66_0F3A_W0,42h

		macro instr? dest*,src*,src2*,aux*&
			AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED,opcode,0,dest,src,src2,aux
		end macro

	end iterate

	iterate <instr,unit,vex_mpw,opcode>, vpconflictd,4,VEX_66_0F38_W0,0C4h, vpconflictq,8,VEX_66_0F38_W1,0C4h, vplzcntd,4,VEX_66_0F38_W0,44h, vplzcntq,8,VEX_66_0F38_W1,44h, \
					     vrcp14ps,4,VEX_66_0F38_W0,4Ch, vrcp14pd,8,VEX_66_0F38_W1,4Ch, vrsqrt14ps,4,VEX_66_0F38_W0,4Eh, vrsqrt14pd,8,VEX_66_0F38_W1,4Eh

		macro instr? dest*,src*&
			AVX_512.single_source_instruction_bcst vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src
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
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,opcode,@dest,@dest.mask,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vexpandps,VEX_66_0F38_W0,88h, vexpandpd,VEX_66_0F38_W1,88h, vpexpandd,VEX_66_0F38_W0,89h, vpexpandq,VEX_66_0F38_W1,89h

		macro instr? dest*,src*
			AVX_512.single_source_instruction vex_mpw,EVEX_REQUIRED,opcode,0,dest,src
		end macro

	end iterate

	iterate <instr,opcode>, fixupimm,54h, range,50h

		macro v#instr#pd? dest*,src*,src2*,aux*&
			AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED,opcode,8,dest,src,src2,aux
		end macro

		macro v#instr#ps? dest*,src*,src2*,aux*&
			AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED,opcode,4,dest,src,src2,aux
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
			AVX_512.single_source_instruction_bcst_sae VEX_66_0F38_W1,EVEX_REQUIRED,opcode,8,dest,src
		end macro

		macro v#instr#ps? dest*,src*&
			AVX_512.single_source_instruction_bcst_sae VEX_66_0F38_W0,EVEX_REQUIRED,opcode,4,dest,src
		end macro

		macro v#instr#sd? dest*,src*,src2*&
			AVX_512.basic_instruction_sae VEX_66_0F38_W1,EVEX_REQUIRED,opcode+1,8,dest,src,src2
		end macro

		macro v#instr#ss? dest*,src*,src2*&
			AVX_512.basic_instruction_sae VEX_66_0F38_W0,EVEX_REQUIRED,opcode+1,4,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode_ps,opcode_pd,opcode_ss,opcode_sd>, getmant,26h,26h,27h,27h, reduce,56h,56h,57h,57h, rndscale,8,9,0Ah,0Bh

		macro v#instr#pd? dest*,src*,aux*&
			AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED,opcode_pd,8,dest,src,aux
		end macro

		macro v#instr#ps? dest*,src*,aux*&
			AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED,opcode_ps,4,dest,src,aux
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
			AVX_512.basic_instruction_bcst_er vex_mpw,EVEX_REQUIRED,2Ch,unit,dest,src,src2
		end macro

	end iterate

	iterate <instr,unit,vex_mpw>, vscalefsd,8,VEX_66_0F38_W1, vscalefss,4,VEX_66_0F38_W0

		macro instr? dest*,src*,src2*&
			AVX_512.basic_instruction_er vex_mpw,EVEX_REQUIRED,2Dh,unit,dest,src,src2
		end macro

	end iterate

	iterate <instr,unit,vex_mpw>, vfpclasspd,8,VEX_66_0F3A_W1, vfpclassps,4,VEX_66_0F3A_W0

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand_bcst @src,src,unit
			x86.parse_operand @aux,aux
			if @dest.type = 'maskreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
				if @src.size = 0
					err 'operand size not specified'
				else if (@src.size <> 16 & @src.size <> 32 & @src.size <> 64) | @aux.size and not 1
					err 'invalid operand size'
				end if
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,66h,@src,@dest.mask,@dest.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,unit,vex_mpw>, vfpclasssd,8,VEX_66_0F3A_W1, vfpclassss,4,VEX_66_0F3A_W0

		macro instr? dest*,src*,aux*
			AVX_512.parse_operand_k1 @dest,dest
			AVX_512.parse_operand @src,src
			x86.parse_operand @aux,aux
			if @dest.type = 'maskreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
				if (@src.type = 'mem' & @src.size and not unit) | (@src.type = 'mmreg' & @src.size <> 16) | @aux.size and not 1
					err 'invalid operand size'
				end if
				@src.memsize = 16
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,67h,@src,@dest.mask,@dest.rm,,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vpmovm2b,VEX_F3_0F38_W0,28h, vpmovm2w,VEX_F3_0F38_W1,28h, vpmovm2d,VEX_F3_0F38_W0,38h, vpmovm2q,VEX_F3_0F38_W1,38h, \
					vpbroadcastmb2q,VEX_F3_0F38_W1,2Ah, vpbroadcastmw2d,VEX_F3_0F38_W0,3Ah

		macro instr? dest*,src*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'mmreg' & @src.type = 'maskreg'
				AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED,opcode,@src,0,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vpmovb2m,VEX_F3_0F38_W0,29h, vpmovw2m,VEX_F3_0F38_W1,29h, vpmovd2m,VEX_F3_0F38_W0,39h, vpmovq2m,VEX_F3_0F38_W1,39h

		macro instr? dest*,src*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'maskreg' & @src.type = 'mmreg'
				AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,opcode,@src,0,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,ratio,opcode>, vpmovuswb,2,10h, vpmovswb,2,20h, vpmovwb,2,30h, vpmovusdb,4,11h, vpmovsdb,4,21h, vpmovdb,4,31h, \
				      vpmovusqb,8,12h, vpmovsqb,8,22h, vpmovqb,8,32h, vpmovusdw,2,13h, vpmovsdw,2,23h, vpmovdw,2,33h, \
				      vpmovusqw,4,14h, vpmovsqw,4,24h, vpmovqw,4,34h, vpmovusqd,2,15h, vpmovsqd,2,25h, vpmovqd,2,35h

		macro instr? dest*,src*
			AVX_512.parse_operand_k1z @dest,dest
			AVX_512.parse_operand @src,src
			if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
				@dest.memsize = @src.size / ratio
				if (@dest.type = 'mmreg' & @dest.size <> (@dest.memsize-1) and not 15 + 16) | (@dest.type = 'mem' & @dest.size and not @dest.memsize)
					err 'invalid operand size'
				end if
				AVX_512.store_instruction @src.size,VEX_F3_0F38_W0,EVEX_REQUIRED,opcode,@dest,@dest.mask,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro AVX_512.parse_vsib_address ns,addr
		ns.mode = 0
		match :sz value, x86.addr
			if sz = 4 | sz = 8
				ns.mode = sz shl 3
			else
				err 'invalid address size'
			end if
			ns.address = +value
			if ns.size = 0
				ns.size = sizeof (value)
			end if
		else
			ns.address = +addr
			if ns.size = 0
				ns.size = sizeof (addr)
			end if
		end match
		ns.base_registers = 0
		ns.index_registers = 0
		repeat elementsof ns.address
			if % metadataof ns.address relativeto x86.r32 | % metadataof ns.address relativeto x86.r64
				ns.base_registers = ns.base_registers + % elementof ns.address * % scaleof ns.address
			else if % metadataof ns.address relativeto SSE.reg | % metadataof ns.address relativeto AVX.reg | 1 metadataof (% metadataof ns.address) relativeto AVX_512.reg
				ns.index_registers = ns.index_registers + % elementof ns.address * % scaleof ns.address
			end if
		end repeat
		ns.displacement = ns.address - ns.base_registers - ns.index_registers
		ns.auto_relative = 0
		if elementsof ns.index_registers = 1
			if 1 metadataof ns.index_registers relativeto SSE.reg
				ns.visize = 16
			else if 1 metadataof ns.index_registers relativeto AVX.reg
				ns.visize = 32
			else
				ns.visize = 1 metadataof (1 metadataof ns.index_registers) - AVX_512.reg
			end if
			ns.rm = 4
			if elementsof ns.base_registers = 1 & 1 scaleof ns.base_registers = 1
				if ns.mode = 0
					ns.mode = 0 scaleof (1 metadataof (1 metadataof ns.base_registers)) shl 3
				else if ns.mode <> 0 scaleof (1 metadataof (1 metadataof ns.base_registers)) shl 3
					err 'invalid address'
				end if
				ns.index_only = 0
				ns.base = 0 scaleof (1 metadataof ns.base_registers)
			else if elementsof ns.base_registers = 0
				if x86.mode = 64
					ns.mode = 64
				else
					ns.mode = 32
				end if
				ns.index_only = 1
				ns.base = 5
			else
				err 'invalid address'
			end if
			ns.scale = 1 scaleof ns.index_registers
			ns.index = 0 scaleof (1 metadataof ns.index_registers)
			if ns.scale > 2 & ns.scale <> 4 & ns.scale <> 8
				err 'invalid address'
			end if
			ns.displacement_size = 4
			ns.mod = 2
			if ns.index_only
				ns.mod = 0
			else if ns.displacement relativeto 0
				if ns.displacement = 0 & ns.rm and 111b <> 5 & (ns.rm <> 4 | ns.base and 111b <> 5)
					ns.displacement_size = 0
					ns.mod = 0
				else if ns.displacement < 80h & ns.displacement >= -80h
					ns.displacement_size = 1
					ns.mod = 1
				else if ns.displacement - 1 shl ns.mode >= -80h & ns.displacement < 1 shl ns.mode
					ns.displacement = ns.displacement - 1 shl ns.mode
					ns.displacement_size = 1
					ns.mod = 1
				end if
			end if
		else
			err 'invalid address'
		end if
	end macro

	macro AVX_512.parse_vsib_operand_value ns,op
		ns.type = 'mem'
		ns.segment_prefix = 0
		match [seg:offs], op
			ns.segment = +seg
			if ns.segment eq 1 elementof ns.segment & 1 metadataof ns.segment relativeto x86.sreg
				ns.segment = 1 metadataof ns.segment - x86.sreg
				if ns.segment < 4
					ns.segment_prefix = 26h + ns.segment shl 3
				else
					ns.segment_prefix = 64h + ns.segment-4
				end if
			else
				err 'invalid operand'
			end if
			AVX_512.parse_vsib_address ns,offs
		else match [addr], op
			AVX_512.parse_vsib_address ns,addr
		else
			err 'invalid operand'
		end match
	end macro

	macro AVX_512.parse_vsib_operand ns,op
		ns.mask = 0
		ns.evex_b = 0
		ns.memsize = 0
		ns.size = 0
		match prefix value, op
			match :sz, x86.prefix
				ns.size = sz
				AVX_512.parse_vsib_operand_value ns,value
			else
				AVX_512.parse_vsib_operand_value ns,op
			end match
		else
			AVX_512.parse_vsib_operand_value ns,op
		end match
	end macro

	macro AVX_512.parse_vsib_operand_k1 ns,opmask
		match op {k1}, opmask
			AVX_512.parse_vsib_operand ns,op
			if k1 eq 1 elementof k1 & 1 metadataof k1 relativeto AVX_512.maskreg & 1 metadataof k1 - AVX_512.maskreg > 0
				ns.mask = 1 metadataof k1 - AVX_512.maskreg
			else
				err 'invalid mask'
			end if
		else
			AVX_512.parse_vsib_operand ns,opmask
		end match
	end macro

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
					AVX_512.store_instruction @src.visize,VEX_66_0F38_W0,EVEX_REQUIRED,opcode,@src,@dest.mask,@dest.rm,@src.index and 10000b
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
					AVX_512.store_instruction @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED,opcode,@src,@dest.mask,@dest.rm,@src.index and 10000b
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
				AVX_512.store_instruction @dest.visize,VEX_66_0F38_W0,EVEX_REQUIRED,opcode,@dest,@dest.mask,@src.rm,@dest.index and 10000b
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
				AVX_512.store_instruction @src.size,VEX_66_0F38_W1,EVEX_REQUIRED,opcode,@dest,@dest.mask,@src.rm,@dest.index and 10000b
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

end if
