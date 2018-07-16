
if ~ defined CLMUL

	restore CLMUL	; this ensures that symbol cannot be forward-referenced
	define CLMUL 1

	include 'sse.inc'

	macro pclmulqdq? dest*,src*,imm*
		SSE.basic_instruction_imm8 66h,<3Ah,44h>,16,dest,src,imm
	end macro

	macro vpclmulqdq? dest*,src*,src2*,imm*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,44h,16,dest,src,src2,imm
	end macro

end if
