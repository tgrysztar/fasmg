
if ~ defined AVX2

	restore AVX2	; this ensures that symbol cannot be forward-referenced
	AVX2 = 1

	include 'avx.inc'

	calminstruction AVX.parse_vsib_operand namespace, operand

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
		check	i metadataof address relativeto SSE.reg | i metadataof address relativeto AVX.reg
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
