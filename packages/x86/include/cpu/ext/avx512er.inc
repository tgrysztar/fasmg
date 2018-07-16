

if ~ defined AVX_512_ER

	restore AVX_512_ER ; this ensures that symbol cannot be forward-referenced
	define AVX_512_ER 1

	include 'avx512.inc'

	iterate <instr,unit,vex_mpw,opcode>, vexp2ps,4,VEX_66_0F38_W0,0C8h, vexp2pd,4,VEX_66_0F38_W1,0C8h, \
					     vrcp28ps,4,VEX_66_0F38_W0,0CAh, vrcp28pd,8,VEX_66_0F38_W1,0CAh, vrsqrt28ps,4,VEX_66_0F38_W0,0CCh, vrsqrt28pd,8,VEX_66_0F38_W1,0CCh

		macro instr? dest*,src*&
			AVX_512.single_source_instruction_bcst_sae vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src
		end macro

	end iterate

	iterate <instr,unit,vex_mpw,opcode>, vrcp28ss,4,VEX_66_0F38_W0,0CBh, vrcp28sd,8,VEX_66_0F38_W1,0CBh, vrsqrt28ss,4,VEX_66_0F38_W0,0CDh, vrsqrt28sd,8,VEX_66_0F38_W1,0CDh

		macro instr? dest*,src*&
			AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src
		end macro

	end iterate

end if
