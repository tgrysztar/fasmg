
iterate <instr,postbyte>, xsave,4, xrstor,5

	macro instr? src*
		x86.parse_operand @src,src
		if @src.type = 'mem'
			x86.store_instruction <0Fh,0AEh>,@src,postbyte
		else
			err 'invalid operand'
		end if
	end macro

	macro instr#64? src*
		if x86.mode = 64
			x86.parse_operand @src,src
			if @src.type = 'mem'
				x86.select_operand_prefix @src,8
				x86.store_instruction <0Fh,0AEh>,@src,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'instruction requires long mode'
		end if
	end macro

end iterate

macro xgetbv?
	db 0Fh,1,0D0h
end macro

macro xsetbv?
	db 0Fh,1,0D1h
end macro
