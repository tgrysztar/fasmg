
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,opcode>, vpmadd52luq,0B4h, vpmadd52huq,0B5h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
	end macro

end iterate

