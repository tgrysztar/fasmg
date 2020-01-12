
include 'sse2.inc'

macro fisttp? src*
	x86.parse_operand @src,src
	if @src.type = 'mem'
		if @src.size = 2
			x86.store_instruction 0DFh,@src,1
		else if @src.size = 4
			x86.store_instruction 0DBh,@src,1
		else if @src.size = 8
			x86.store_instruction 0DDh,@src,1
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
		SSE.basic_instruction 66h,ext,16,dest,src
	end macro
	macro instr#ps? dest*,src*
		SSE.basic_instruction 0F2h,ext,16,dest,src
	end macro
end iterate

iterate <instr,ext>, movsldup,12h, movshdup,16h
	macro instr? dest*,src*
		SSE.basic_instruction 0F3h,ext,16,dest,src
	end macro
end iterate

macro movddup? dest*,src*
	SSE.basic_instruction 0F2h,12h,8,dest,src
end macro

macro lddqu? dest*,src*
	SSE.parse_operand @dest,dest
	SSE.parse_operand @src,src
	if (@dest.size or @src.size) and not 16
		err 'invalid operand size'
	end if
	if @dest.type = 'mmreg' & @src.type = 'mem'
		@src.opcode_prefix = 0F2h
		x86.store_instruction <0Fh,0F0h>,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro monitor? arg1,arg2,arg3
	match any, arg1 arg2 arg3
		if ~ arg1 eq eax | ~ arg2 eq ecx | ~ arg3 eq edx
			err 'invalid combination of operands'
		end if
	end match
	db 0Fh,01h,0C8h
end macro

macro mwait? arg1,arg2
	match any, arg1 arg2
		if ~ arg1 eq eax | ~ arg2 eq ecx
			err 'invalid combination of operands'
		end if
	end match
	db 0Fh,01h,0C9h
end macro
