
if ~ defined AVX2

	restore AVX2	; this ensures that symbol cannot be forward-referenced
	define AVX2 1

	include 'avx.inc'

	macro AVX.parse_vsib_address ns,addr
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
			else if % metadataof ns.address relativeto SSE.reg | % metadataof ns.address relativeto AVX.reg
				ns.index_registers = ns.index_registers + % elementof ns.address * % scaleof ns.address
			end if
		end repeat
		ns.displacement = ns.address - ns.base_registers - ns.index_registers
		ns.auto_relative = 0
		if elementsof ns.index_registers = 1
			if 1 metadataof ns.index_registers relativeto SSE.reg
				ns.visize = 16
			else
				ns.visize = 32
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

	macro AVX.parse_vsib_operand_value ns,op
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
			AVX.parse_vsib_address ns,offs
		else match [addr], op
			AVX.parse_vsib_address ns,addr
		else
			err 'invalid operand'
		end match
	end macro

	macro AVX.parse_vsib_operand ns,op
		ns.size = 0
		match prefix value, op
			match :sz, x86.prefix
				ns.size = sz
				AVX.parse_vsib_operand_value ns,value
			else
				AVX.parse_vsib_operand_value ns,op
			end match
		else
			AVX.parse_vsib_operand_value ns,op
		end match
	end macro

	iterate <instr,opcode,asize>, vpgatherdd,90h,4, vpgatherqd,91h,8, vgatherdps,92h,4, vgatherqps,93h,8

		macro instr? dest*,src*,mask*
			AVX.parse_operand @dest,dest
			AVX.parse_vsib_operand @src,src
			AVX.parse_operand @aux,mask
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
		end macro

	end iterate

	iterate <instr,opcode,asize>, vpgatherdq,90h,4, vpgatherqq,91h,8, vgatherdpd,92h,4, vgatherqpd,93h,8

		macro instr? dest*,src*,mask*
			AVX.parse_operand @dest,dest
			AVX.parse_vsib_operand @src,src
			AVX.parse_operand @aux,mask
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
		end macro

	end iterate

	iterate <instr,opcode>, packsswb,63h, packuswb,67h, packssdw,6Bh, paddb,0FCh, paddw,0FDh, paddd,0FEh, paddq,0D4h, paddsb,0ECh, paddsw,0EDh, paddusb,0DCh, paddusw,0DDh, \
				pand,0DBh, pandn,0DFh, pavgb,0E0h, pavgw,0E3h, pcmpeqb,74h, pcmpeqw,75h, pcmpeqd,76h, pcmpgtb,64h, pcmpgtw,65h, pcmpgtd,66h, \
				pmaddwd,0F5h, pmaxsw,0EEh, pmaxub,0DEh, pminsw,0EAh, pminub,0DAh, pmulhuw,0E4h, pmulhw,0E5h, pmullw,0D5h, pmuludq,0F4h, \
				por,0EBh, psadbw,0F6h, psubb,0F8h, psubw,0F9h, psubd,0FAh, psubq,0FBh, psubsb,0E8h, psubsw,0E9h, psubusb,0D8h, psubusw,0D9h, \
				punpckhbw,68h, punpckhwd,69h, punpckhdq,6Ah, punpckhqdq,6Dh, punpcklbw,60h, punpcklwd,61h, punpckldq,62h, punpcklqdq,6Ch, pxor,0EFh

		macro v#instr? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F_W0,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, packusdw,2Bh, pcmpeqq,29h, pcmpgtq,37h, phaddw,1, phaddd,2, phaddsw,3, phsubw,5, phsubd,6, phsubsw,7, pmaddubsw,4, \
				pmaxsb,3Ch, pmaxsd,3Dh, pmaxuw,3Eh, pmaxud,3Fh, pminsb,38h, pminsd,39h, pminuw,3Ah, pminud,3Bh, pmulhrsw,0Bh, pmulld,40h, pmuldq,28h, \
				pshufb,0, psignb,8, psignw,9, psignd,0Ah

		macro v#instr? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F38_W0,opcode,0,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, mpsadbw,42h, palignr,0Fh

		macro v#instr? dest*,src*,src2*,imm*
			AVX.basic_instruction_imm8 VEX_66_0F3A_W0,opcode,0,dest,src,src2,imm
		end macro

	end iterate

	iterate <instr,opcode>, pabsb,1Ch, pabsw,1Dh, pabsd,1Eh, pblendw,0Eh

		macro v#instr? dest*,src*
			AVX.single_source_instruction VEX_66_0F38_W0,opcode,0,dest,src
		end macro

	end iterate

	iterate <instr,vex_mpw>, pshufd,VEX_66_0F_W0, pshufhw,VEX_F3_0F_W0, pshuflw,VEX_F2_0F_W0

		macro v#instr? dest*,src*,imm*
			AVX.single_source_instruction_imm8 vex_mpw,70h,0,dest,src,imm
		end macro

	end iterate

	iterate <instr,vex_mpw,opcode>, vpsllvd,VEX_66_0F38_W0,47h, vpsllvq,VEX_66_0F38_W1,47h, vpsrlvd,VEX_66_0F38_W0,45h, vpsrlvq,VEX_66_0F38_W1,45h, vpsravd,VEX_66_0F38_W0,46h

		macro instr? dest*,src*,src2*
			AVX.basic_instruction vex_mpw,opcode,0,dest,src,src2
		end macro

	end iterate

	macro vpblendvb? dest*,src*,src2*,mask*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		AVX.parse_operand @src2,src2
		AVX.parse_operand @aux,mask
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'mmreg'
			if @src.size <> @dest.size | @src2.size and not @dest.size | @aux.size <> @dest.size
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,VEX_66_0F3A_W0,4Ch,@src2,@dest.rm,@src.rm,1,(@aux.rm and 1111b) shl 4
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vpmovmskb? dest*,src*
		x86.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'reg' & @src.type = 'mmreg'
			if (@dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8))
				err 'invalid operand size'
			end if
			AVX.store_instruction @src.size,VEX_66_0F_W0,0D7h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode,msize>, pmovsxbw,20h,8, pmovsxbd,21h,4, pmovsxbq,22h,2, pmovsxwd,23h,8, pmovsxwq,24h,4, pmovsxdq,25h,8, \
				      pmovzxbw,30h,8, pmovzxbd,31h,4, pmovzxbq,32h,2, pmovzxwd,33h,8, pmovzxwq,34h,4, pmovzxdq,35h,8

		macro v#instr? dest*,src*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not (msize * (@dest.size shr 4)))
					err 'invalid operand size'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F38_W0,opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,postbyte>, pslldq,7, psrldq,3

		macro v#instr dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			x86.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'imm'
				if @src2.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F_W0,73h,@src,postbyte,@dest.rm,1,@src2.imm
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
				if @src2.size and not 16
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F_W0,opcode_rrm,@src2,@dest.rm,@src.rm
			else if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'imm'
				if @src2.size and not 1
					err 'invalid operand size'
				else if @src.size <> @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F_W0,opcode,@src,postbyte,@dest.rm,1,@src2.imm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vmovntdqa? dest*,src*
		AVX.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX.store_instruction @dest.size,VEX_66_0F38_W0,2Ah,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,w>, vpmaskmovd,0, vpmaskmovq,1

		macro instr? dest*,src*,src2*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			AVX.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
				if @src.size <> @dest.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F38_W#w,8Ch,@src2,@dest.rm,@src.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg' & @src2.type = 'mmreg'
				if @src.size <> @src2.size | @dest.size and not @src.size
					err 'operand sizes do not match'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F38_W#w,8Eh,@dest,@src2.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vbroadcasti128? dest*,src*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @dest.size <> 32 | @src.size and not 16
				err 'invalid operand size'
			end if
			AVX.store_instruction 32,VEX_66_0F38_W0,5Ah,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vextracti128? dest*,src*,aux*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		x86.parse_operand @aux,aux
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not 16 | @src.size <> 32 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 32,VEX_66_0F3A_W0,39h,@dest,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vinserti128? dest*,src*,src2*,aux*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		AVX.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 32 | @src.size <> 32 | @src2.size and not 16 | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 32,VEX_66_0F3A_W0,38h,@src2,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro vperm2i128? dest*,src*,src2*,imm*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,46h,32,dest,src,src2,imm
	end macro

	iterate <instr,opcode,msize>, vbroadcastss,18h,4, vpbroadcastb,78h,1, vpbroadcastw,79h,2, vpbroadcastd,58h,4, vpbroadcastq,59h,8

		macro instr? dest*,src*
			AVX.parse_operand @dest,dest
			AVX.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
				if (@src.type='mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not msize)
					err 'invalid operand size'
				end if
				AVX.store_instruction @dest.size,VEX_66_0F38_W0,opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro vbroadcastsd? dest*,src*
		AVX.parse_operand @dest,dest
		AVX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @dest.size <> 32 | (@src.type='mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 8)
				err 'invalid operand size'
			end if
			AVX.store_instruction 32,VEX_66_0F38_W0,19h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,opcode>, vpermq,0, vpermpd,1

		macro instr? dest*,src*,imm*
			AVX.single_source_instruction_imm8 VEX_66_0F3A_W1,opcode,32,dest,src,imm
		end macro

	end iterate

	iterate <instr,opcode>, vpermd,36h, vpermps,16h

		macro instr? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F38_W0,opcode,32,dest,src,src2
		end macro

	end iterate

end if
