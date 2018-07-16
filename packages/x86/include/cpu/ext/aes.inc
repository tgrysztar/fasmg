
if ~ defined SSE_AES

	restore SSE_AES ; this ensures that symbol cannot be forward-referenced
	define SSE_AES 1

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

end if