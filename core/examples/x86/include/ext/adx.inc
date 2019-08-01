
iterate <instr,pfx>, adcx,66h, adox,0F3h
	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
			if @src.size <> 0 & @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @dest.size = 8 & x86.mode = 64
				@src.prefix = 48h
			else if @dest.size <> 4
				err 'invalid operand size'
			end if
			@src.opcode_prefix = pfx
			x86.store_instruction <0Fh,38h,0F6h>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate
