
if ~ defined RDSEED

	restore RDSEED	 ; this ensures that symbol cannot be forward-referenced
	define RDSEED 1

	macro rdseed? dest*
		x86.parse_operand @dest,dest
		if @dest.type = 'reg'
			x86.select_operand_prefix @dest,@dest.size
			x86.store_instruction <0Fh,0C7h>,@dest,7
		else
			err 'invalid operand'
		end if
	end macro

end if
