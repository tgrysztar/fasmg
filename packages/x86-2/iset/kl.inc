
iterate <instr,supp,membytes>, aesdec128kl,0DDh,48, aesdec256kl,0DFh,64, aesenc128kl,0DCh,48, aesenc256kl,0DEh,64

	macro instr? dest*,src*
		require KL
		SSE.parse_operand@dest dest
		x86.parse_operand@src src
		if @dest.type = 'mmreg' & @dest.size = 16 & @src.type = 'mem'
			if @src.size and not membytes
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 0F3h
			x86.store_instruction@src <0Fh,38h,supp>,@dest.rm
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,postbyte,membytes>, aesdecwide128kl,001b,48, aesdecwide256kl,011b,64, aesencwide128kl,000b,48, aesencwide256kl,010b,64

	macro instr? src*
		require KL
		x86.parse_operand@src src
		if @src.type = 'mem'
			if @src.size and not membytes
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 0F3h
			x86.store_instruction@src <0Fh,38h,0D8h>,postbyte
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,supp>, encodekey128,0FAh, encodekey256,0FBh

	macro instr? dest*,src*
		require KL
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		if @dest.type = 'reg' & @dest.size = 4 & @src.type = 'reg' & @src.size = 4
			@src.opcode_prefix = 0F3h
			x86.store_instruction@src <0Fh,38h,supp>,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro loadiwkey? src*,src2*
	require KL
	SSE.parse_operand@src src
	SSE.parse_operand@src2 src2
	if @src.type = 'mmreg' & @src.size = 16 & @src2.type = 'mmreg' & @src2.size = 16
		@src2.opcode_prefix = 0F3h
		x86.store_instruction@src2 <0Fh,38h,0DCh>,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro
