
macro movdiri? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @src.type = 'reg' & @dest.type = 'mem'
		if @dest.size <> 0 & @src.size <> @dest.size
			err 'operand sizes do not match'
		end if
		if @src.size = 8 & x86.mode = 64
			@dest.prefix = 48h
		else if @src.size <> 4
			err 'invalid operand size'
		end if
		x86.store_instruction <0Fh,38h,0F9h>,@dest,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

