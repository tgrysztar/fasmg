
iterate <instr,supp>, gf2p8mulb,0CFh
	macro instr? dest*,src*
		require GFNI
		SSE.basic_instruction 66h,<38h,supp>,16,dest,src
	end macro
end iterate

iterate <instr,supp>, gf2p8affineinvqb,0CFh, gf2p8affineqb,0CEh
	macro instr? dest*,src*,imm*
		require GFNI
		SSE.basic_instruction_imm8 66h,<3Ah,supp>,16,dest,src,imm
	end macro
end iterate

iterate <instr,opcode>, gf2p8mulb,0CFh

	macro v#instr? dest*,src*,src2*&
		require GFNI
		AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, gf2p8affineinvqb,0CFh, gf2p8affineqb,0CEh

	macro v#instr? dest*,src*,src2*,imm*&
		require GFNI
		AVX_512.basic_instruction_bcst_imm8 VEX_66_0F3A_W1,EVEX_W1+EVEX_VL,opcode,8,dest,src,src2,imm
	end macro

end iterate

