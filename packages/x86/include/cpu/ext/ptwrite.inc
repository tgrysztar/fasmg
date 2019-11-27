
macro ptwrite? src*
	x86.parse_operand @src,src
	if @src.size = 0
		err 'operand size not specified'
	else if @src.size <> 4 & (@src.size <> 8 | x86.mode <> 64)
		err 'invalid operand size'
	end if
	if @src.type = 'reg' | @src.type = 'mem'
		if @src.size = 8
			x86.select_operand_prefix @src,8
		end if
		@src.opcode_prefix = 0F3h
		x86.store_instruction <0Fh,0AEh>,@src,4
	else
		err 'invalid operand'
	end if
end macro
