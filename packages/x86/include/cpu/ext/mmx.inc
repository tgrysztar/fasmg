
if ~ defined MMX

	restore MMX	; this ensures that symbol cannot be forward-referenced
	MMX = 1

	element MMX.reg

	repeat 8, i:0
		element mm#i? : MMX.reg + i
	end repeat

	calminstruction MMX.parse_operand namespace, operand

		local	size, type, mod, rm, imm
		local	i, sym, cmd

		asmcmd	=x86.=parse_operand namespace, operand

		arrange type, namespace.=type
		arrange size, namespace.=size
		arrange imm, namespace.=imm

		check	type = 'imm' & size = 0
		jno	done
		check	imm eq 1 elementof imm & 1 metadataof imm relativeto MMX.reg
		jno	done

		compute type, 'mmreg'
		compute mod, 11b
		compute rm, 1 metadataof imm - MMX.reg
		compute size, 8

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

	calminstruction MMX.basic_instruction ext,dest,src
		asmcmd	=MMX.=parse_operand =@dest,dest
		asmcmd	=MMX.=parse_operand =@src,src
		check	(@src.size or @dest.size) and not 8
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		jno	invalid_combination_of_operands
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,=@dest.=rm
		exit
	    invalid_combination_of_operands:
		asmcmd	=err 'invalid combination of operands'
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
		asmcmd	=MMX.=parse_operand =@dest,dest
		asmcmd	=MMX.=parse_operand =@src,src
		check	(@src.size or @dest.size) and not 8
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		jyes	mmreg_mem
		check	@dest.type = 'mem' & @src.type = 'mmreg'
		jyes	mem_mmreg
		asmcmd	=err 'invalid combination of operands'
		exit
	    mmreg_mem:
		asmcmd	=x86.=store_instruction <0Fh,6Fh>,=@src,=@dest.=rm
		exit
	    mem_mmreg:
		asmcmd	=x86.=store_instruction <0Fh,7Fh>,=@dest,=@src.=rm
		exit
	end calminstruction

	calminstruction movd? dest*,src*
		asmcmd	=MMX.=parse_operand =@dest,dest
		asmcmd	=MMX.=parse_operand =@src,src
		check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	mmreg_rm
		check	(@dest.type = 'mem' | @dest.type = 'reg') & @src.type = 'mmreg'
		jyes	rm_mmreg
		asmcmd	=err 'invalid combination of operands'
		exit
	    mmreg_rm:
		check	@src.size and not 4
		jno	mmreg_rm_ok
		asmcmd	=err 'invalid operand size'
	      mmreg_rm_ok:
		asmcmd	=x86.=store_instruction <0Fh,6Eh>,=@src,=@dest.=rm
		exit
	    rm_mmreg:
		check	@dest.size and not 4
		jno	rm_mmreg_ok
		asmcmd	=err 'invalid operand size'
	      rm_mmreg_ok:
		asmcmd	=x86.=store_instruction <0Fh,7Eh>,=@dest,=@src.=rm
	end calminstruction

	calminstruction MMX.bit_shift_instruction ext,dest,src
		asmcmd	=MMX.=parse_operand =@dest,dest
		asmcmd	=MMX.=parse_operand =@src,src
		check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		jyes	mmreg_rm
		check	@dest.type = 'mmreg' & @src.type = 'imm'
		jyes	mmreg_imm
		asmcmd	=err 'invalid combination of operands'
		exit
	    mmreg_rm:
		check	@src.size and not 8
		jno	mmreg_rm_ok
		asmcmd	=err 'invalid operand size'
	      mmreg_rm_ok:
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,=@dest.=rm
		exit
	    mmreg_imm:
		check	@src.size and not 1
		jno	rm_mmreg_ok
		asmcmd	=err 'invalid operand size'
	      rm_mmreg_ok:
		local	iext, irm
		compute iext, 70h+(ext and 0Fh)
		compute irm, ((ext shr 4)-0Ch) shl 1
		asmcmd	=x86.=store_instruction <0Fh,iext>,=@dest,irm,1,=@src.=imm
	end calminstruction

	iterate <instr,opcode>, psrlw,0D1h, psrld,0D2h, psrlq,0D3h, psrad,0E2h, psraw,0E1h, psllw,0F1h, pslld,0F2h, psllq,0F3h

		macro instr? dest*,src*
			MMX.bit_shift_instruction opcode,dest,src
		end macro

	end iterate

	macro emms?
		db 0Fh,77h
	end macro

end if
