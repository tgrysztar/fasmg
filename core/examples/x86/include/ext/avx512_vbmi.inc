
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,vex_mpw,opcode>, vpermb,VEX_66_0F38_W0,8Dh, vpermi2b,VEX_66_0F38_W0,75h, vpermt2b,VEX_66_0F38_W0,7Dh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpmultishiftqb,83h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
	end macro

end iterate
