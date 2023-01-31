
element MMX.reg

repeat 8, i:0
	element mm#i? : MMX.reg + i
end repeat

iterate context, @dest,@src,@src2,@aux

	namespace context

		calminstruction MMX.parse_operand#context operand

			call	x86.parse_operand#context, operand

			check	type = 'imm' & size = 0
			jno	done
			check	imm eq 1 elementof imm & 1 metadataof imm relativeto MMX.reg
			jno	done

			compute type, 'mmreg'
			compute mod, 11b
			compute rm, 1 metadataof imm - MMX.reg
			compute size, 8

		    done:

		end calminstruction

		calminstruction MMX.select_operand_prefix#context size*
			check	size = 16
			jno	no_prefix
			call	x86.require.SSE2
			compute prefix, 66h
			exit
		    no_prefix:
			check	size <> 8
			jno	done
			err	'invalid operand size'
		    done:
		end calminstruction

	end namespace

end iterate

calminstruction MMX.basic_instruction ext,dest,src
	call	x86.require.MMX
	call	SSE.parse_operand@dest, dest
	call	SSE.parse_operand@src, src
	check	@src.size and not @dest.size
	jno	size_ok
	err	'operand sizes do not match'
    size_ok:
	check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
	jno	invalid_combination_of_operands
	call	MMX.select_operand_prefix@src, @dest.size
	xcall	x86.store_instruction@src, <0Fh,ext>,@dest.rm
	exit
    invalid_combination_of_operands:
	err	'invalid combination of operands'
end calminstruction

calminstruction MMX.bit_shift_instruction ext,dest,src
	call	x86.require.MMX
	call	SSE.parse_operand@dest, dest
	call	SSE.parse_operand@src, src
	check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
	jyes	mmreg_rm
	check	@dest.type = 'mmreg' & @src.type = 'imm'
	jyes	mmreg_imm
	err	'invalid combination of operands'
	exit
    mmreg_rm:
	check	@src.size and not @dest.size
	jno	mmreg_rm_ok
	err	'operand sizes do not match'
      mmreg_rm_ok:
	call	MMX.select_operand_prefix@src, @dest.size
	xcall	x86.store_instruction@src, <0Fh,ext>,@dest.rm
	exit
    mmreg_imm:
	check	@src.size and not 1
	jno	rm_mmreg_ok
	err	'invalid operand size'
      rm_mmreg_ok:
	local	iext, irm
	compute iext, 70h+(ext and 0Fh)
	compute irm, ((ext shr 4)-0Ch) shl 1
	call	MMX.select_operand_prefix@dest, @dest.size
	xcall	x86.store_instruction@dest, <0Fh,iext>,irm,byte,@src.imm
end calminstruction

iterate <instr,opcode>, punpcklbw,60h, punpcklwd,61h, punpckldq,62h, packsswb,63h, pcmpgtb,64h, pcmpgtw,65h, pcmpgtd,66h, packuswb,67h, punpckhbw,68h, \
			punpckhwd,69h, punpckhdq,6Ah, packssdw,6Bh, pcmpeqb,74h, pcmpeqw,75h, pcmpeqd,76h, pmullw,0D5h, psubusb,0D8h, psubusw,0D9h, \
			pand,0DBh, paddusb,0DCh, paddusw,0DDh, pandn,0DFh, pmulhw,0E5h, psubsb,0E8h, psubsw,0E9h, por,0EBh, paddsb,0ECh, paddsw,0EDh, \
			pxor,0EFh, pmaddwd,0F5h, psubb,0F8h, psubw,0F9h, psubd,0FAh, paddb,0FCh, paddw,0FDh, paddd,0FEh

	macro instr? dest*,src*
		MMX.basic_instruction opcode,dest,src
	end macro

end iterate

calminstruction movq? dest*,src*
	call	x86.require.MMX
	call	MMX.parse_operand@dest, dest
	call	MMX.parse_operand@src, src
	check	(@src.size or @dest.size) and not 8
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
	jyes	mmreg_mem
	check	@dest.type = 'mem' & @src.type = 'mmreg'
	jyes	mem_mmreg
	err	'invalid combination of operands'
	exit
    mmreg_mem:
	xcall	x86.store_instruction@src, <0Fh,6Fh>,@dest.rm
	exit
    mem_mmreg:
	xcall	x86.store_instruction@dest, <0Fh,7Fh>,@src.rm
	exit
end calminstruction

calminstruction movd? dest*,src*
	call	x86.require.MMX
	call	MMX.parse_operand@dest, dest
	call	MMX.parse_operand@src, src
	check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'reg')
	jyes	mmreg_rm
	check	(@dest.type = 'mem' | @dest.type = 'reg') & @src.type = 'mmreg'
	jyes	rm_mmreg
	err	'invalid combination of operands'
	exit
    mmreg_rm:
	check	@src.size and not 4
	jno	mmreg_rm_ok
	err	'invalid operand size'
      mmreg_rm_ok:
	xcall	x86.store_instruction@src, <0Fh,6Eh>,@dest.rm
	exit
    rm_mmreg:
	check	@dest.size and not 4
	jno	rm_mmreg_ok
	err	'invalid operand size'
      rm_mmreg_ok:
	xcall	x86.store_instruction@dest, <0Fh,7Eh>,@src.rm
end calminstruction

iterate <instr,opcode>, psrlw,0D1h, psrld,0D2h, psrlq,0D3h, psrad,0E2h, psraw,0E1h, psllw,0F1h, pslld,0F2h, psllq,0F3h

	macro instr? dest*,src*
		MMX.bit_shift_instruction opcode,dest,src
	end macro

end iterate

calminstruction emms?
	call	x86.require.MMX
	emit	1, 0Fh
	emit	1, 77h
end calminstruction
