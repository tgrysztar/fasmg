
include 'sse.inc'

macro pclmulqdq? dest*,src*,imm*
	SSE.basic_instruction_imm8 66h,<3Ah,44h>,16,dest,src,imm
end macro

if defined AVX

	macro vpclmulqdq? dest*,src*,src2*,imm*
		AVX.basic_instruction_imm8 VEX_66_0F3A_W0,44h,16,dest,src,src2,imm
	end macro

end if
