
macro invpcid? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.type = 'reg' & @src.type = 'mem'
		if (x86.mode < 64 & @dest.size <> 4) | (x86.mode = 64 & @dest.size <> 8) | @src.size and not 16
			err 'invalid operand size'
		end if
		@src.opcode_prefix = 66h
		x86.store_instruction <0Fh,38h,82h>,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

