
include 'avx.inc'

macro vcvtph2ps? dest*,src*
	AVX.parse_operand @dest,dest
	AVX.parse_operand @src,src
	if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
		if (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size*2 <> @dest.size)
			err 'invalid operand size'
		end if
		AVX.store_instruction @dest.size,VEX_66_0F38_W0,13h,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vcvtps2ph? dest*,src*,round*
	AVX.parse_operand @dest,dest
	AVX.parse_operand @src,src
	x86.parse_operand @aux,round
	if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if (@dest.type = 'mmreg' & @dest.size <> 16) | (@dest.type = 'mem' & @dest.size*2 <> @src.size) | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction @src.size,VEX_66_0F3A_W0,1Dh,@dest,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro
