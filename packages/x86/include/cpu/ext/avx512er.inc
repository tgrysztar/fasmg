
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,unit,vex_mpw,opcode>, vexp2ps,4,VEX_66_0F38_W0,0C8h, vexp2pd,4,VEX_66_0F38_W1,0C8h, \
				     vrcp28ps,4,VEX_66_0F38_W0,0CAh, vrcp28pd,8,VEX_66_0F38_W1,0CAh, vrsqrt28ps,4,VEX_66_0F38_W0,0CCh, vrsqrt28pd,8,VEX_66_0F38_W1,0CCh

	macro instr? dest*,src_sae*&
		AVX_512.parse_operand_k1z @dest,dest
		match src=,sae, src_sae
			AVX_512.parse_operand @src,src
			AVX_512.parse_sae @src,sae
		else
			AVX_512.parse_operand_bcst @src,src_sae,unit
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @dest.size <> 64
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vrcp28ss,4,VEX_66_0F38_W0,0CBh, vrcp28sd,8,VEX_66_0F38_W1,0CBh, vrsqrt28ss,4,VEX_66_0F38_W0,0CDh, vrsqrt28sd,8,VEX_66_0F38_W1,0CDh

	macro instr? dest*,src*,src2*&
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED,opcode,unit,dest,src,src2
	end macro

end iterate
