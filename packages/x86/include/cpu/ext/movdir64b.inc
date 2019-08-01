
if ~ defined AVX512
	define x86.dqqword? :64
	define x86.zword? :64
end if

macro movdir64b? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.type = 'reg' & @src.type = 'mem'
		if @src.size and not 64
			err 'invalid operand size'
		end if
		if (@src.mode = 16 & @dest.size <> 2) | (@src.mode = 32 & @dest.size <> 4) | (@src.mode = 64 & @dest.size <> 8)
			err 'invalid operand size'
		end if
		@src.opcode_prefix = 66h
		x86.store_instruction <0Fh,38h,0F8h>,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

