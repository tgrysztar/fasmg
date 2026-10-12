

iterate <instr,supp>, sha1msg1,0C9h, sha1msg2,0CAh, sha1nexte,0C8h, sha256msg1,0CCh, sha256msg2,0CDh,

	macro instr? dest*,src*
		require SHA
		SSE.basic_instruction 0,<38h,supp>,16,dest,src
	end macro

end iterate

iterate <instr,supp>, sha1rnds4,0CCh

	macro instr? dest*,src*,src2*
		require SHA
		SSE.basic_instruction_imm8 0,<3Ah,supp>,16,dest,src,src2
	end macro

end iterate

macro sha256rnds2? dest*,src*,sel*
	require SHA
	SSE.parse_operand@dest dest
	SSE.parse_operand@src src
	SSE.parse_operand@aux sel
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'mmreg' & @aux.size = 16 & @aux.rm = 0
		if @dest.size or @src.size and not 16
			err 'invalid operand size'
		end if
		x86.store_instruction@src <0Fh,38h,0CBh>,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

