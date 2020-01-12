
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,vex_mpw,opcode>, vpbroadcastmb2q,VEX_F3_0F38_W1,2Ah, vpbroadcastmw2d,VEX_F3_0F38_W0,3Ah

	macro instr? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'maskreg'
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpconflictd,4,VEX_66_0F38_W0,0C4h, vpconflictq,8,VEX_66_0F38_W1,0C4h, vplzcntd,4,VEX_66_0F38_W0,44h, vplzcntq,8,VEX_66_0F38_W1,44h

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src
	end macro

end iterate

