
macro fisttp? src*
	require SSE3+
	x86.parse_operand@src src
	if @src.type = 'mem'
		if @src.size = 2
			x86.store_instruction@src 0DFh,1
		else if @src.size = 4
			x86.store_instruction@src 0DBh,1
		else if @src.size = 8
			x86.store_instruction@src 0DDh,1
		else if @src.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else
		err 'invalid operand'
	end if
end macro

iterate <instr,ext>, addsub,0D0h, hadd,7Ch, hsub,7Dh
	macro instr#pd? dest*,src*
		require SSE3+
		SSE.basic_instruction 66h,ext,16,dest,src
	end macro
	macro instr#ps? dest*,src*
		require SSE3+
		SSE.basic_instruction 0F2h,ext,16,dest,src
	end macro
end iterate

iterate <instr,ext>, movsldup,12h, movshdup,16h
	macro instr? dest*,src*
		require SSE3+
		SSE.basic_instruction 0F3h,ext,16,dest,src
	end macro
end iterate

macro movddup? dest*,src*
	require SSE3+
	SSE.basic_instruction 0F2h,12h,8,dest,src
end macro

macro lddqu? dest*,src*
	require SSE3+
	SSE.parse_operand@src dest
	SSE.parse_operand@src src
	if (@dest.size or @src.size) and not 16
		err 'invalid operand size'
	end if
	if @dest.type = 'mmreg' & @src.type = 'mem'
		@src.opcode_prefix = 0F2h
		x86.store_instruction@src <0Fh,0F0h>,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,supp>, pshufb,0, phaddw,1, phaddd,2, phaddsw,3, pmaddubsw,4, phsubw,5, phsubd,6, phsubsw,7, psignb,8, psignw,9, psignd,0Ah, pmulhrsw,0Bh, pabsb,1Ch, pabsw,1Dh, pabsd,1Eh
	macro instr? dest*,src*
		require SSSE3+
		MMX.basic_instruction <38h,supp>,dest,src
	end macro
end iterate

macro palignr? dest*,src*,aux*
	require SSSE3+
	SSE.parse_operand@dest dest
	SSE.parse_operand@src src
	x86.parse_operand@aux aux
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		if @aux.size and not 1
			err 'invalid operand size'
		end if
		MMX.select_operand_prefix@src @dest.size
		x86.store_instruction@src <0Fh,3Ah,0Fh>,@dest.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro
