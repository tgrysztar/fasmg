
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,opcode>, vp4dpwssd,52h, vp4dpwssds,53h

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand_k1z @dest,dest
		match rsrc+=3, src
			AVX_512.parse_operand @src,rsrc
		else
			AVX_512.parse_operand @src,src
		end match
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & @src2.type = 'mem'
			if @dest.size <> 64 | @src2.size and not 16
				err 'invalid operand size'
			else if @dest.size <> @src.size
				err 'operand sizes do not match'
			end if
			@src2.memsize = 0
			AVX_512.store_instruction @dest.size,VEX_F2_0F38_W0,EVEX_REQUIRED,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate