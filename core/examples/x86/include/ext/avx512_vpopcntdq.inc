
if ~ definite AVX_512

	include 'avx512f.inc'

end if

iterate <instr,vex_mpw,opcode>, vpopcntd,VEX_66_0F38_W0,55h, vpopcntq,VEX_66_0F38_W1,55h


	macro instr? dest*,src*
		AVX_512.single_source_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src
	end macro

end iterate

