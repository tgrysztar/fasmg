
include 'ssse3.inc'

iterate <instr,supp>, ptest,17h, pmuldq,28h, pcmpeqq,29h, packusdw,2Bh, pminsb,38h, pminsd,39h, pminuw,3Ah, pminud,3Bh, pmaxsb,3Ch, pmaxsd,3Dh, pmaxuw,3Eh, pmaxud,3Fh, pmulld,40h, phminposuw,41h
	macro instr? dest*,src*
		SSE.basic_instruction 66h,<38h,supp>,16,dest,src
	end macro
end iterate

iterate <instr,supp>, roundps,08h, roundpd,09h, roundss,0Ah, roundsd,0Bh, blendps,0Ch, blendpd,0Dh, pblendw,0Eh, dpps,40h, dppd,41h, mpsadbw,42h
	macro instr? dest*,src*,imm*
		SSE.basic_instruction_imm8 66h,<3Ah,supp>,16,dest,src,imm
	end macro
end iterate

iterate <instr,supp>, pblendvb,10h, blendvps,14h, blendvpd,15h
	macro instr? dest*,src*,sel*
		SSE.parse_operand @dest,dest
		SSE.parse_operand @src,src
		SSE.parse_operand @aux,sel
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'mmreg' & @aux.size = 16 & @aux.rm = 0
			if @dest.size or @src.size and not 16
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 66h
			x86.store_instruction <0Fh,38h,supp>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <conv,code,msize>, bw,0,8, bd,1,4, bq,2,2, wd,3,8, wq,4,4, dq,5,8
	macro pmovsx#conv? dest*,src*
		SSE.basic_instruction 66h,<38h,20h+code>,msize,dest,src
	end macro
	macro pmovzx#conv? dest*,src*
		SSE.basic_instruction 66h,<38h,30h+code>,msize,dest,src
	end macro
end iterate

macro insertps? dest*,src*,sel*
	SSE.basic_instruction_imm8 66h,<3Ah,21h>,4,dest,src,sel
end macro

macro extractps? dest*,src*,sel*
	x86.parse_operand @dest,dest
	SSE.parse_operand @src,src
	x86.parse_operand @aux,sel
	if @dest.type = 'reg' & @src.type = 'mmreg' & @aux.type = 'imm'
		if x86.mode = 64 & @dest.size = 8
			@dest.size = 4
		end if
		if @dest.size <> 4 | @src.size and not 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.opcode_prefix = 66h
		x86.store_instruction <0Fh,3Ah,17h>,@dest,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro pinsrb? dest*,src*,sel*
	SSE.parse_operand @dest,dest
	x86.parse_operand @src,src
	x86.parse_operand @aux,sel
	if (@dest.type = 'mmreg' & @dest.size = 16) & (@src.type = 'reg' | @src.type = 'mem') & @aux.type = 'imm'
		if (@src.type = 'reg' & @src.size <> 4) | (@src.type = 'mem' & @src.size and not 1) | @aux.size and not 1
			err 'invalid operand size'
		end if
		@src.opcode_prefix = 66h
		x86.store_instruction <0Fh,3Ah,20h>,@src,@dest.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro pinsrd? dest*,src*,sel*
	SSE.parse_operand @dest,dest
	x86.parse_operand @src,src
	x86.parse_operand @aux,sel
	if (@dest.type = 'mmreg' & @dest.size = 16) & (@src.type = 'reg' | @src.type = 'mem') & @aux.type = 'imm'
		if @src.size and not 4 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@src.opcode_prefix = 66h
		x86.store_instruction <0Fh,3Ah,22h>,@src,@dest.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro pinsrq? dest*,src*,sel*
	SSE.parse_operand @dest,dest
	x86.parse_operand @src,src
	x86.parse_operand @aux,sel
	if (@dest.type = 'mmreg' & @dest.size = 16) & (@src.type = 'reg' | @src.type = 'mem') & @aux.type = 'imm'
		if @src.size and not 8 | @aux.size and not 1
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		@src.opcode_prefix = 66h
		@src.rex_prefix = 48h
		x86.store_instruction <0Fh,3Ah,22h>,@src,@dest.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro pextrb? dest*,src*,sel*
	x86.parse_operand @dest,dest
	SSE.parse_operand @src,src
	x86.parse_operand @aux,sel
	if (@dest.type = 'reg' | @dest.type = 'mem') & (@src.type = 'mmreg' & @src.size = 16) & @aux.type = 'imm'
		if x86.mode = 64 & @dest.type = 'reg' & @dest.size = 8
			@dest.size = 4
		end if
		if (@dest.type = 'reg' & @dest.size <> 4) | (@dest.size = 'mem' & @dest.size and not 1) | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.opcode_prefix = 66h
		x86.store_instruction <0Fh,3Ah,14h>,@dest,@src.rm,1,@aux.imm
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
	else if @dest.type = 'mem' & (@src.type = 'mmreg' & @src.size = 16) & @aux.type = 'imm'
		if @dest.size and not 2 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.opcode_prefix = 66h
		x86.store_instruction <0Fh,3Ah,15h>,@dest,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro pextrd? dest*,src*,sel*
	x86.parse_operand @dest,dest
	SSE.parse_operand @src,src
	x86.parse_operand @aux,sel
	if (@dest.type = 'reg' | @dest.type = 'mem') & (@src.type = 'mmreg' & @src.size = 16) & @aux.type = 'imm'
		if x86.mode = 64 & @dest.type = 'reg' & @dest.size = 8
			@dest.size = 4
		end if
		if @dest.size and not 4 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.opcode_prefix = 66h
		x86.store_instruction <0Fh,3Ah,16h>,@dest,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro pextrq? dest*,src*,sel*
	x86.parse_operand @dest,dest
	SSE.parse_operand @src,src
	x86.parse_operand @aux,sel
	if (@dest.type = 'reg' | @dest.type = 'mem') & (@src.type = 'mmreg' & @src.size = 16) & @aux.type = 'imm'
		if @dest.size and not 8 | @aux.size and not 1
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		@dest.opcode_prefix = 66h
		@dest.rex_prefix = 48h
		x86.store_instruction <0Fh,3Ah,16h>,@dest,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro movntdqa? dest*,src*
	SSE.parse_operand @dest,dest
	SSE.parse_operand @src,src
	if @dest.type = 'mmreg' & @src.type = 'mem'
		if (@dest.size or @src.size) and not 16
			err 'invalid operand size'
		end if
		@src.opcode_prefix = 66h
		x86.store_instruction <0Fh,38h,2Ah>,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro
