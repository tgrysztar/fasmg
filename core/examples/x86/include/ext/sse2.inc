
if ~ defined SSE2

	restore SSE2	; this ensures that symbol cannot be forward-referenced
	SSE2 = 1

	include 'sse.inc'

	iterate <instr,ext>, sqrt,51h, rsqrt,52h, rcp,53h, add,58h, mul,59h, sub,5Ch, min,5Dh, div,5Eh, max,5Fh
		macro instr#pd? dest*,src*
			SSE.basic_instruction 66h,ext,16,dest,src
		end macro
		macro instr#sd? dest*,src*
			SSE.basic_instruction 0F2h,ext,8,dest,src
		end macro
	end iterate

	iterate <instr,ext>, and,54h, andn,55h, or,56h, xor,57h, unpckl,14h, unpckh,15h
		macro instr#pd? dest*,src*
			SSE.basic_instruction 66h,ext,16,dest,src
		end macro
	end iterate

	macro cmppd? dest*,src*,code*
		SSE.basic_instruction_imm8 66h,0C2h,16,dest,src,code
	end macro

	macro SSE.cmpsd? dest*,src*,code*
		SSE.basic_instruction_imm8 0F2h,0C2h,8,dest,src,code
	end macro

	calminstruction cmpsd? args&
		match	, args
		jno	sse
		assemble x86.o32
		assemble x86.cmpsw
		exit
	    sse:
		arrange args, =SSE.=cmpsd args
		assemble args
	end calminstruction

	iterate <cond,code>, eq,0, lt,1, le,2, unord,3, neq,4, nlt,5, nle,6, ord,7
		macro cmp#cond#pd? dest*,src*
			cmppd dest,src,code
		end macro
		macro cmp#cond#sd? dest*,src*
			cmpsd dest,src,code
		end macro
	end iterate

	macro shufpd? dest*,src*,imm*
		SSE.basic_instruction_imm8 66h,0C6h,16,dest,src,imm
	end macro

	iterate <instr,ext>, movapd,28h, movupd,10h
		macro instr? dest*,src*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if (@dest.size or @src.size) and not 16
				err 'invalid operand size'
			end if
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				@src.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg'
				@dest.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext+1>,@dest,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	iterate <instr,ext>, movlpd,12h, movhpd,16h
		macro instr? dest*,src*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if @dest.type = 'mmreg' & @src.type = 'mem'
				if @dest.size <> 16 | @src.size and not 8
					err 'invalid operand size'
				end if
				@src.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg'
				if @dest.size and not 8 | @src.size <> 16
					err 'invalid operand size'
				end if
				@dest.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext+1>,@dest,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	macro SSE.movsd? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @dest.size <> 16 | (@src.type = 'mem' & @src.size and not 8) | (@src.type = 'mmreg' & @src.size <> 16)
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 0F2h
			x86.store_instruction <0Fh,10h>,@src,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not 8 | @src.size <> 16
				err 'invalid operand size'
			end if
			@dest.opcode_prefix = 0F2h
			x86.store_instruction <0Fh,11h>,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	calminstruction movsd? args&
		match	, args
		jno	sse
		assemble x86.o32
		assemble x86.movsw
		exit
	    sse:
		arrange args, =SSE.=movsd args
		assemble args
	end calminstruction

	iterate <instr,pre>, movdqa,66h, movdqu,0F3h
		macro instr? dest*,src*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if (@dest.size or @src.size) and not 16
				err 'invalid operand size'
			end if
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				@src.opcode_prefix = pre
				x86.store_instruction <0Fh,6Fh>,@src,@dest.rm
			else if @dest.type = 'mem' & @src.type = 'mmreg'
				@dest.opcode_prefix = pre
				x86.store_instruction <0Fh,7Fh>,@dest,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	iterate <instr,ext>, movntpd,2Bh, movntdq,0E7h
		macro instr? dest*,src*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if @dest.type = 'mem' & @src.type = 'mmreg'
				if (@dest.size or @src.size) and not 16
					err 'invalid operand size'
				end if
				@dest.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext>,@dest,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	macro movmskpd? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'reg' & @src.type = 'mmreg'
			if (@dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | @src.size <> 16
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 66h
			x86.store_instruction <0Fh,50h>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro maskmovdqu? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mmreg'
			if (@dest.size or @src.size) <> 16
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 66h
			x86.store_instruction <0Fh,0F7h>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,ext>, ucomisd,2Eh, comisd,2Fh
		macro instr? dest*,src*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if @dest.size <> 16 | (@src.type = 'mem' & @src.size and not 8) | (@src.type = 'mmreg' & @src.size <> 16)
					err 'invalid operand size'
				end if
				@src.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	macro cvtps2pd? dest*,src*
		SSE.basic_instruction 0,5Ah,8,dest,src
	end macro

	macro cvtpd2ps? dest*,src*
		SSE.basic_instruction 66h,5Ah,16,dest,src
	end macro

	macro cvtsd2ss? dest*,src*
		SSE.basic_instruction 0F2h,5Ah,8,dest,src
	end macro

	macro cvtss2sd? dest*,src*
		SSE.basic_instruction 0F3h,5Ah,4,dest,src
	end macro

	macro cvtdq2ps? dest*,src*
		SSE.basic_instruction 0,5Bh,16,dest,src
	end macro

	macro cvtps2dq? dest*,src*
		SSE.basic_instruction 66h,5Bh,16,dest,src
	end macro

	macro cvttps2dq? dest*,src*
		SSE.basic_instruction 0F3h,5Bh,16,dest,src
	end macro

	macro cvttpd2dq? dest*,src*
		SSE.basic_instruction 66h,0E6h,16,dest,src
	end macro

	macro cvtpd2dq? dest*,src*
		SSE.basic_instruction 0F2h,0E6h,16,dest,src
	end macro

	macro cvtdq2pd? dest*,src*
		SSE.basic_instruction 0F3h,0E6h,8,dest,src
	end macro

	macro movdq2q? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mmreg'
			if @dest.size <> 8 | @src.size <> 16
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 0F2h
			x86.store_instruction <0Fh,0D6h>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro movq2dq? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mmreg'
			if @dest.size <> 16 | @src.size <> 8
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 0F3h
			x86.store_instruction <0Fh,0D6h>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro cvtpi2pd? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @dest.size <> 16 | @src.size and not 8
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 66h
			x86.store_instruction <0Fh,2Ah>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro cvtsi2sd? dest*,src*
		SSE.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'reg')
			if @src.size = 0
				err 'operand size not specified'
			else if @dest.size <> 16 | @src.size < 4
				err 'invalid operand size'
			end if
			x86.select_operand_prefix @src,@src.size
			@src.opcode_prefix = 0F2h
			x86.store_instruction <0Fh,2Ah>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,ext>, cvttpd2pi,2Ch, cvtpd2pi,2Dh
		macro instr? dest*,src*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if @dest.size <> 8 | @src.size and not 16
					err 'invalid operand size'
				end if
				@src.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	iterate <instr,ext>, cvttsd2si,2Ch, cvtsd2si,2Dh
		macro instr? dest*,src*
			x86.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if @dest.size < 4 | (@src.type = 'mem' & @src.size and not 8) | (@src.type = 'mmreg' & @src.size <>16)
					err 'invalid operand size'
				end if
				x86.select_operand_prefix @src,@dest.size
				@src.opcode_prefix = 0F2h
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	calminstruction MMX.select_operand_prefix rm_operand*,size*
		local	sym, prefix
		check	size = 16
		jno	no_prefix
		compute prefix, 66h
		arrange sym, rm_operand.=prefix
		publish sym, prefix
		exit
	    no_prefix:
		check	size <> 8
		jno	done
		asmcmd	=err 'invalid operand size'
	    done:
	end calminstruction

	calminstruction MMX.basic_instruction ext,dest,src
		asmcmd	=SSE.=parse_operand =@dest,dest
		asmcmd	=SSE.=parse_operand =@src,src
		check	@src.size and not @dest.size
		jno	size_ok
		asmcmd	=err 'operand sizes do not match'
	    size_ok:
		check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		jno	invalid_combination_of_operands
		asmcmd	=MMX.=select_operand_prefix =@src,=@dest.=size
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,=@dest.=rm
		exit
	    invalid_combination_of_operands:
		asmcmd	=err 'invalid combination of operands'
	end calminstruction

	calminstruction MMX.bit_shift_instruction ext,dest,src
		asmcmd	=SSE.=parse_operand =@dest,dest
		asmcmd	=SSE.=parse_operand =@src,src
		check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		jyes	mmreg_rm
		check	@dest.type = 'mmreg' & @src.type = 'imm'
		jyes	mmreg_imm
		asmcmd	=err 'invalid combination of operands'
		exit
	    mmreg_rm:
		check	@src.size and not @dest.size
		jno	mmreg_rm_ok
		asmcmd	=err 'operand sizes do not match'
	      mmreg_rm_ok:
		asmcmd	=MMX.=select_operand_prefix =@src,=@dest.=size
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
		asmcmd	=MMX.=select_operand_prefix =@dest,=@dest.=size
		asmcmd	=x86.=store_instruction <0Fh,iext>,=@dest,irm,1,=@src.=imm
	end calminstruction

	iterate <instr,ext>, paddq,0D4h, pmuludq,0F4h, psubq,0FBh
		macro instr? dest*,src*
			MMX.basic_instruction ext,dest,src
		end macro
	end iterate

	macro movq? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not 8) | (@src.type = 'mmreg' & @src.size <> @dest.size)
				err 'invalid operand size'
			end if
			if @dest.size = 8
				x86.store_instruction <0Fh,6Fh>,@src,@dest.rm
			else
				@src.opcode_prefix = 0F3h
				x86.store_instruction <0Fh,7Eh>,@src,@dest.rm
			end if
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not 8
				err 'invalid operand size'
			end if
			if @src.size = 8
				x86.store_instruction <0Fh,7Fh>,@dest,@src.rm
			else
				@dest.opcode_prefix = 66h
				x86.store_instruction <0Fh,0D6h>,@dest,@src.rm
			end if
		else if @dest.type = 'reg' & @src.type = 'mmreg'
			if @dest.size <> 8
				err 'invalid operand size'
			end if
			if @src.size = 16
				@dest.opcode_prefix = 66h
			end if
			@dest.prefix = 48h
			x86.store_instruction <0Fh,7Eh>,@dest,@src.rm
		else if @dest.type = 'mmreg' & @src.type = 'reg'
			if @src.size <> 8
				err 'invalid operand size'
			end if
			if @dest.size = 16
				@src.opcode_prefix = 66h
			end if
			@src.prefix = 48h
			x86.store_instruction <0Fh,6Eh>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro movd? dest*,src*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'reg')
			if @src.size and not 4
				err 'invalid operand size'
			end if
			MMX.select_operand_prefix @src,@dest.size
			x86.store_instruction <0Fh,6Eh>,@src,@dest.rm
		else if (@dest.type = 'mem' | @dest.type = 'reg') & @src.type = 'mmreg'
			if @dest.size and not 4
				err 'invalid operand size'
			end if
			MMX.select_operand_prefix @dest,@src.size
			x86.store_instruction <0Fh,7Eh>,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro pinsrw? dest*,src*,sel*
		SSE.parse_operand @dest,dest
		x86.parse_operand @src,src
		x86.parse_operand @aux,sel
		if @dest.type = 'mmreg' & (@src.type = 'reg' | @src.type = 'mem') & @aux.type = 'imm'
			if (@src.type = 'reg' & @src.size <> 4) | (@src.type = 'mem' & @src.size and not 2) | @aux.size and not 1
				err 'invalid operand size'
			end if
			MMX.select_operand_prefix @src,@dest.size
			x86.store_instruction <0Fh,0C4h>,@src,@dest.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro pextrw? dest*,src*,sel*
		x86.parse_operand @dest,dest
		SSE.parse_operand @src,src
		x86.parse_operand @aux,sel
		if @dest.type = 'reg' & @src.type = 'mmreg' & @aux.type = 'imm'
			if x86.mode = 64 & @dest.size = 8
				@dest.size = 4
			end if
			if @dest.size <> 4 | @aux.size and not 1
				err 'invalid operand size'
			end if
			MMX.select_operand_prefix @src,@src.size
			x86.store_instruction <0Fh,0C5h>,@src,@dest.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,pre>, pshufd,66h, pshuflw,0F2h, pshufhw,0F3h
		macro instr? dest*,src*,sel*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			x86.parse_operand @aux,sel
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
				if @dest.size <> 16 | @src.size and not 16 | @aux.size and not 1
					err 'invalid operand size'
				end if
				@src.opcode_prefix = pre
				x86.store_instruction <0Fh,70h>,@src,@dest.rm,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	macro pmovmskb? dest*,src*
		x86.parse_operand @dest,dest
		SSE.parse_operand @src,src
		if @dest.type = 'reg' & @src.type = 'mmreg'
			if @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)
				err 'invalid operand size'
			end if
			MMX.select_operand_prefix @src,@src.size
			x86.store_instruction <0Fh,0D7h>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,postbyte>, psrldq,3, pslldq,7
		macro instr? dest*,cnt*
			SSE.parse_operand @dest,dest
			x86.parse_operand @aux,cnt
			if @dest.type = 'mmreg' & @aux.type = 'imm'
				if @dest.size <> 16 | @aux.size and not 1
					err 'invalid operand size'
				end if
				@dest.opcode_prefix = 66h
				x86.store_instruction <0Fh,73h>,@dest,postbyte,1,@aux.imm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	iterate <instr,ext>, punpcklqdq,6Ch, punpckhqdq,6Dh
		macro instr? dest*,src*
			SSE.parse_operand @dest,dest
			SSE.parse_operand @src,src
			if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
				if (@dest.size or @src.size) and not 16
					err 'invalid operand size'
				end if
				@src.opcode_prefix = 66h
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end iterate

	macro movnti? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'mem' & @src.type = 'reg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			else if @src.size <> 4 & @src.size <> 8
				err 'invalid operand size'
			end if
			x86.select_operand_prefix @dest,@src.size
			x86.store_instruction <0Fh,0C3h>,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro clflush? src*
		x86.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size and not 1
				err 'invalid operand size'
			end if
			x86.store_instruction <0Fh,0AEh>,@src,7
		else
			err 'invalid operand'
		end if
	end macro

	macro lfence?
		db 0Fh,0AEh,0E8h
	end macro

	macro mfence?
		db 0Fh,0AEh,0F0h
	end macro

end if