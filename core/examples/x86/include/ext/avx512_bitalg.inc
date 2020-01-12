
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,vex_mpw,opcode>, vpopcntb,VEX_66_0F38_W0,54h, vpopcntw,VEX_66_0F38_W1,54h


	macro instr? dest*,src*
		AVX_512.single_source_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src
	end macro

end iterate

macro vpshufbitqmb? dest*,src*,src2*
	AVX_512.parse_operand_k1 @dest,dest
	AVX_512.parse_operand @src,src
	AVX_512.parse_operand @src2,src2
	if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
		if @src2.size and not @src.size
			err 'operand sizes do not match'
		end if
		AVX_512.store_instruction @src.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,8Fh,@src2,@dest.mask,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro
