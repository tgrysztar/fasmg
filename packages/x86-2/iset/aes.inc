
iterate <instr,supp>, aesdec,0DEh, aesenc,0DCh, aesdeclast,0DFh, aesenclast,0DDh

	macro instr? dest*,src*
		require AESNI
		SSE.basic_instruction 66h,<38h,supp>,16,dest,src
	end macro

	macro v#instr? dest*,src*,src2*
		AVX_512.parse_operand@dest dest
		AVX_512.parse_operand@src src
		AVX_512.parse_operand@src2 src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @dest.size <> @src.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			@src2.memsize = 0
			if x86.ext and x86.VAES.ext
				AVX_512.store_instruction@src2 @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,0,@dest.rm,@src.rm
			else
				require AESNI
				AVX_512.store_instruction@src2 @dest.size,VEX_66_0F38_W0,EVEX_FORBIDDEN,opcode,0,@dest.rm,@src.rm
			end if

		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, aesimc,0DBh

	macro instr? dest*,src*
		require AESNI
		SSE.basic_instruction 66h,<38h,supp>,16,dest,src
	end macro

	macro v#instr? dest*,src*
		require AESNI
		AVX_512.single_source_instruction VEX_66_0F38_W0,EVEX_FORBIDDEN,opcode,16,dest,src
	end macro

end iterate

iterate <instr,supp>, aeskeygenassist,0DFh

	macro instr? dest*,src*,imm*
		require AESNI
		SSE.basic_instruction_imm8 66h,<3Ah,supp>,16,dest,src,imm
	end macro

	macro v#instr? dest*,src*,imm*
		require AESNI
		AVX_512.single_source_instruction_imm8 VEX_66_0F3A_W0,EVEX_FORBIDDEN,opcode,16,dest,src,imm
	end macro

end iterate
