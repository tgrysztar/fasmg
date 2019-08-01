
iterate <instr,postbyte>, rdfsbase,0, rdgsbase,1, wrfsbase,2, wrgsbase,3

	macro instr? dest*
		if x86.mode = 64
			x86.parse_operand @dest,dest
			if @dest.type = 'reg'
				if @dest.size >= 4
					@dest.opcode_prefix = 0F3h
					x86.select_operand_prefix @dest,@dest.size
					x86.store_instruction <0Fh,0AEh>,@dest,postbyte
				else
					err 'invalid operand size'
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'instruction requires long mode'
		end if
	end macro

end iterate

