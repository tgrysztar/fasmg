
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,unit,vex_mpw,opcode>, vpopcntd,4,VEX_66_0F38_W0,55h, vpopcntq,8,VEX_66_0F38_W1,55h


	macro instr? dest*,src*
		AVX_512.single_source_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src
	end macro

end iterate

