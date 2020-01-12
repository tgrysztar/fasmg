
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,vex_mpw,opcode>, vpshldw,VEX_66_0F3A_W1,70h, vpshrdw,VEX_66_0F3A_W1,72h

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_imm8 vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2,aux
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpshldd,4,VEX_66_0F3A_W0,71h, vpshldq,8,VEX_66_0F3A_W1,71h, \
				     vpshrdd,4,VEX_66_0F3A_W0,73h, vpshrdq,8,VEX_66_0F3A_W1,73h

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_imm8 vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src,src2,aux
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpshldvw,VEX_66_0F38_W1,70h, vpshrdvw,VEX_66_0F38_W1,72h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpshldvd,4,VEX_66_0F38_W0,71h, vpshldvq,8,VEX_66_0F38_W1,71h, \
				     vpshrdvd,4,VEX_66_0F38_W0,73h, vpshrdvq,8,VEX_66_0F38_W1,73h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src,src2
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpcompressb,VEX_66_0F38_W0,63h, vpcompressw,VEX_66_0F38_W1,63h

	macro instr? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpexpandb,VEX_66_0F38_W0,62h, vpexpandw,VEX_66_0F38_W1,62h

	macro instr? dest*,src*
		AVX_512.single_source_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src
	end macro

end iterate
