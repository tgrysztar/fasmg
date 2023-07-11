

iterate <instr,modrm>, endbr32,0FBh, endbr64,0FAh

	macro instr?
		require CET_IBT
		db 0F3h,0Fh,1Eh,modrm
	end macro

end iterate

iterate <instr,ext,postbyte>, clrssbsy,0AEh,6, rstorssp,01h,5

	macro instr? src*
		require CET_SS
		x86.parse_operand@src src
		if @src.type = 'mem'
			if @src.size and not 8
				err 'invalid operand size'
			else
				@src.opcode_prefix = 0F3h
				x86.store_instruction@src <0Fh,ext>,postbyte
			end if
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,ext,postbyte>, incsspd,0AEh,5, rdsspd,1Eh,1

	macro instr? src*
		require CET_SS
		x86.parse_operand@src src
		if @src.type = 'reg'
			if @src.size <> 4
				err 'invalid operand size'
			else
				@src.opcode_prefix = 0F3h
				x86.store_instruction@src <0Fh,ext>,postbyte
			end if
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,ext,postbyte>, incsspq,0AEh,5, rdsspq,1Eh,1

	macro instr? src*
		require CET_SS
		x86.parse_operand@src src
		if @src.type = 'reg'
			if @src.size <> 8
				err 'invalid operand size'
			else
				x86.select_operand_prefix@src 8
				@src.opcode_prefix = 0F3h
				x86.store_instruction@src <0Fh,ext>,postbyte
			end if
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,modrm>, saveprevssp,0EAh, setssbsy,0E8h

	macro instr?
		require CET_SS
		db 0F3h,0Fh,01h,modrm
	end macro

end iterate

iterate <instr,prefix,ext>, wrssd,0,0F6h, wrussd,66h,0F5h

	macro instr? dest*,src*
		require CET_SS
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		local size
		if @src.size <> 4
			err 'invalid operand size'
		else if @dest.size and not @src.size
			err 'operand sizes do not match'
		end if
		if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			@dest.opcode_prefix = prefix
			x86.store_instruction@dest <0Fh,38h,ext>,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,prefix,ext>, wrssq,0,0F6h, wrussq,66h,0F5h

	macro instr? dest*,src*
		require CET_SS
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		local size
		if @src.size <> 8
			err 'invalid operand size'
		else if @dest.size and not @src.size
			err 'operand sizes do not match'
		end if
		if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			x86.select_operand_prefix@dest 8
			@dest.opcode_prefix = prefix
			x86.store_instruction@dest <0Fh,38h,ext>,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate
