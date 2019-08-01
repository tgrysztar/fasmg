
include 'sse.inc'

iterate <instr,supp>, aesdec,0DEh, aesenc,0DCh, aesimc,0DBh, aesdeclast,0DFh, aesenclast,0DDh
	macro instr? dest*,src*
		SSE.basic_instruction 66h,<38h,supp>,16,dest,src
	end macro
end iterate

iterate <instr,supp>, aeskeygenassist,0DFh
	macro instr? dest*,src*,imm*
		SSE.basic_instruction_imm8 66h,<3Ah,supp>,16,dest,src,imm
	end macro
end iterate

if defined AVX

	iterate <instr,opcode>, aesenc,0DCh, aesenclast,0DDh, aesdec,0DEh, aesdeclast,0DFh

		macro v#instr? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F38_W0,opcode,16,dest,src,src2
		end macro

	end iterate

	iterate <instr,opcode>, aesimc,0DBh

		macro v#instr? dest*,src*
			AVX.single_source_instruction VEX_66_0F38_W0,opcode,16,dest,src
		end macro

	end iterate

	iterate <instr,opcode>, aeskeygenassist,0DFh

		macro v#instr? dest*,src*,imm*
			AVX.single_source_instruction_imm8 VEX_66_0F3A_W0,opcode,16,dest,src,imm
		end macro

	end iterate

end if

