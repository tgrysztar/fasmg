
include 'aes.inc'

if defined AVX_512

	iterate <instr,opcode>, aesenc,0DCh, aesenclast,0DDh, aesdec,0DEh, aesdeclast,0DFh

		macro v#instr? dest*,src*,src2*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
				if @dest.size <> @src.size | @src2.size and not @dest.size
					err 'operand sizes do not match'
				end if
				@src2.memsize = 0
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,@src2,0,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

else

	include 'avx.inc'

	iterate <instr,opcode>, aesenc,0DCh, aesenclast,0DDh, aesdec,0DEh, aesdeclast,0DFh

		macro v#instr? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F38_W0,opcode,0,dest,src,src2
		end macro

	end iterate

end if
