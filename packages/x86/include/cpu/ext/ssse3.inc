
include 'sse3.inc'

iterate <instr,supp>, pshufb,0, phaddw,1, phaddd,2, phaddsw,3, pmaddubsw,4, phsubw,5, phsubd,6, phsubsw,7, psignb,8, psignw,9, psignd,0Ah, pmulhrsw,0Bh, pabsb,1Ch, pabsw,1Dh, pabsd,1Eh
	macro instr? dest*,src*
		MMX.basic_instruction <38h,supp>,dest,src
	end macro
end iterate

macro palignr? dest*,src*,aux*
	SSE.parse_operand @dest,dest
	SSE.parse_operand @src,src
	x86.parse_operand @aux,aux
	if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		if @aux.size and not 1
			err 'invalid operand size'
		end if
		MMX.select_operand_prefix @src,@dest.size
		x86.store_instruction <0Fh,3Ah,0Fh>,@src,@dest.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro
