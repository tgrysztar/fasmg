
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,opcode>, vpdpbusd,50h, vpdpbusds,51h, vpdpwssd,52h, vpdpwssds,53h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate
