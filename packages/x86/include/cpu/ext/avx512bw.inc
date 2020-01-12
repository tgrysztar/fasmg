

if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <t,vex_mpw,msize>, d,VEX_66_0F_W1,4, q,VEX_0F_W1,8

	iterate <instr,opcode>, kand,41h, kandn,42h, knot,44h, kor,45h, kxnor,46h, kxor,47h, kadd,4Ah

		macro instr#t? dest*,src*,src2*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			AVX_512.parse_operand @src2,src2
			if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
				AVX.store_instruction 32,vex_mpw,opcode,@src2,@dest.rm,@src.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	iterate <instr,opcode>, knot,44h, kortest,98h, ktest,99h

		macro instr#t? dest*,src*
			AVX_512.parse_operand @dest,dest
			AVX_512.parse_operand @src,src
			if @dest.type = 'maskreg' & @src.type = 'maskreg'
				AVX.store_instruction 16,vex_mpw,opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro

	end iterate

	macro kmov#t? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'maskreg' & (@src.type = 'maskreg' | @src.type = 'mem')
			if @src.type = 'mem' & @src.size and not msize
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,vex_mpw,90h,@src,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'maskreg'
			if @dest.size and not msize
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,vex_mpw,91h,@dest,@src.rm
		else if @dest.type = 'maskreg' & @src.type = 'reg'
			if (msize < 8 & @src.size <> 4) | (msize = 8 & @src.size <> 8)
				err 'invalid operand size'
			else if msize = 8 & x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction 16,vex_mpw,92h,@src,@dest.rm
		else if @dest.type = 'reg' & @src.type = 'maskreg'
			if (msize < 8 & @dest.size <> 4) | (msize = 8 & @dest.size <> 8)
				err 'invalid operand size'
			else if msize = 8 & x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction 16,vex_mpw,93h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, kshiftrd,VEX_66_0F3A_W0,31h, kshiftrq,VEX_66_0F3A_W1,31h, \
				kshiftld,VEX_66_0F3A_W0,33h, kshiftlq,VEX_66_0F3A_W1,33h

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX.store_instruction 16,vex_mpw,opcode,@src,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw>, kunpckwd,VEX_0F_W0, kunpckdq,VEX_0F_W1

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
			AVX.store_instruction 32,vex_mpw,4Bh,@src2,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,evex_f,opcode_rm,opcode_mr>, vmovdqu8,VEX_F2_0F_W0,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh, vmovdqu16,VEX_F2_0F_W1,EVEX_REQUIRED+EVEX_VL,6Fh,7Fh

	macro instr? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,evex_f,opcode_rm,@src,@dest.mask,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @src.size,vex_mpw,evex_f,opcode_mr,@dest,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpabsb,1Ch, vpabsw,1Dh

	macro instr? dest*,src*
		AVX_512.single_source_instruction VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,0,dest,src
	end macro

end iterate

iterate <instr,opcode>, vpacksswb,63h, vpackuswb,67h, vpaddb,0FCh, vpaddw,0FDh, vpaddsb,0ECh, vpaddsw,0EDh, vpaddusb,0DCh, vpaddusw,0DDh, vpavgb,0E0h, vpavgw,0E3h, \
			vpmaddwd,0F5h, vpmaxsw,0EEh, vpmaxub,0DEh, vpminsw,0EAh, vpminub,0DAh, vpmulhuw,0E4h, vpmulhw,0E5h, vpmullw,0D5h, \
			vpsadbw,0F6h, vpsubb,0F8h, vpsubw,0F9h, vpsubsb,0E8h, vpsubsw,0E9h, vpsubusb,0D8h, vpsubusw,0D9h, \
			vpunpckhbw,68h, vpunpckhwd,69h, vpunpcklbw,60h, vpunpcklwd,61h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpackssdw,6Bh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpackusdw,2Bh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction_bcst VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpalignr,0Fh

	macro instr? dest*,src*,src2*,imm*
		AVX_512.basic_instruction_imm8 VEX_66_0F3A_W0,EVEX_AS_VEX+EVEX_VL,opcode,0,dest,src,src2,imm
	end macro

end iterate

iterate <instr,opcode>, vpmaddubsw,4, vpmaxsb,3Ch, vpmaxuw,3Eh, vpminsb,38h, vpminuw,3Ah, vpmulhrsw,0Bh, vpshufb,0

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpcmpeqb,74h, pcmpeqw,75h, vpcmpgtb,64h, vpcmpgtw,65h

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand_k1 @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @src.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @src.size,VEX_66_0F_W0,EVEX_FORBIDDEN,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vpextrb? dest*,src*,aux*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	x86.parse_operand @aux,aux
	if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if (@dest.type = 'reg' & @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | (@dest.type = 'mem' & @dest.size and not 1) | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.memsize = 1
		AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,14h,@dest,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpextrw? dest*,src*,aux*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	x86.parse_operand @aux,aux
	if @dest.type = 'reg' & @src.type = 'mmreg' & @aux.type = 'imm'
		if @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8) | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		AVX_512.store_instruction 16,VEX_66_0F_W0,EVEX_AS_VEX,0C5h,@src,0,@dest.rm,,1,@aux.imm
	else if @dest.type = 'mem' & @src.type = 'mmreg' & @aux.type = 'imm'
		if  @dest.size and not 2 | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.memsize = 2
		AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,15h,@dest,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw,opcode,msize>, vpinsrb,VEX_66_0F3A_W0,20h,1, vpinsrw,VEX_66_0F_W0,0C4h,2

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'reg' & @src2.size <> 4) | (@src2.type = 'mem' & @src2.size and not msize) | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src2.memsize = msize
			AVX_512.store_instruction 16,vex_mpw,EVEX_AS_VEX,opcode,@src2,0,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode,msize>, vpmovsxbw,20h,8, vpmovzxbw,30h,8

	macro instr? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			@src.memsize = msize * (@dest.size shr 4)
			if (@src.type = 'mmreg' & @src.size <> (@src.memsize-1) and not 15 + 16) | (@src.type = 'mem' & @src.size and not @src.memsize)
				err 'invalid operand size'
			end if
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw>, vpshufhw,VEX_F3_0F_W0, vpshuflw,VEX_F2_0F_W0

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_AS_VEX+EVEX_VL,70h,@src,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,postbyte>, vpslldq,7, vpsrldq,3

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @aux.type = 'imm'
			if @aux.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @src.type = 'mem'
				AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,73h,@src,0,postbyte,@dest.rm,1,@aux.imm
			else
				AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,73h,@src,0,postbyte,@dest.rm,1,@aux.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode_rrm,opcode,postbyte>, vpsllw,0F1h,71h,6, vpsraw,0E1h,71h,4, vpsrlw,0D1h,71h,2

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			@src2.memsize = 16
			if @src2.size and not @src2.memsize
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode_rrm,@src2,@dest.mask,@dest.rm,@src.rm
		else if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem') & @src2.type = 'imm'
			if @src2.size and not 1
				err 'invalid operand size'
			else if @src.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @src.type = 'mem'
				AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			else
				AVX_512.store_instruction @dest.size,VEX_66_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,@src,@dest.mask,postbyte,@dest.rm,1,@src2.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vdbpsadbw,VEX_66_0F3A_W0,42h

	macro instr? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_imm8 vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2,aux
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpblendmb,VEX_66_0F38_W0,66h, vpblendmw,VEX_66_0F38_W1,66h

	macro instr? dest*,src*,src2*&
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode,opcode_g,msize>, vpbroadcastb,78h,7Ah,1, vpbroadcastw,79h,7Bh,2

	macro instr? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if (@src.type='mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not msize)
				err 'invalid operand size'
			end if
			@src.memsize = msize
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_AS_VEX+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
		else if @dest.type = 'mmreg' & @src.type = 'reg'
			if @src.size <> msize & (@src.size <> 4 | msize = 8)
				err 'invalid operand size'
			end if
			@src.memsize = msize
			if msize = 8
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode_g,@src,@dest.mask,@dest.rm
			else
				AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode_g,@src,@dest.mask,@dest.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpcmpb,VEX_66_0F3A_W0,3Fh, vpcmpub,VEX_66_0F3A_W0,3Eh, vpcmpw,VEX_66_0F3A_W1,3Fh, vpcmpuw,VEX_66_0F3A_W1,3Eh

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_operand_k1 @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg') & @aux.type = 'imm'
			if @src2.size and not @src.size | @aux.size and not 1
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpermw,VEX_66_0F38_W1,8Dh, vpermi2w,VEX_66_0F38_W1,75h, vpermt2w,VEX_66_0F38_W1,7Dh

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpmovb2m,VEX_F3_0F38_W0,29h, vpmovw2m,VEX_F3_0F38_W1,29h

	macro instr? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'maskreg' & @src.type = 'mmreg'
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpmovm2b,VEX_F3_0F38_W0,28h, vpmovm2w,VEX_F3_0F38_W1,28h

	macro instr? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'maskreg'
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,0,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,ratio,opcode>, vpmovuswb,2,10h, vpmovswb,2,20h, vpmovwb,2,30h

	macro instr? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
			@dest.memsize = @src.size / ratio
			if (@dest.type = 'mmreg' & @dest.size <> (@dest.memsize-1) and not 15 + 16) | (@dest.type = 'mem' & @dest.size and not @dest.memsize)
				err 'invalid operand size'
			end if
			AVX_512.store_instruction @src.size,VEX_F3_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, vpsllvw,12h, vpsrlvw,10h, vpsravw,11h

	macro instr? dest*,src*,src2*
		AVX_512.basic_instruction VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vptestnmb,VEX_F3_0F38_W0,26h, vptestnmw,VEX_F3_0F38_W1,26h, vptestmb,VEX_66_0F38_W0,26h, vptestmw,VEX_66_0F38_W1,26h

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand_k1 @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'maskreg' & @src.type = 'mmreg' & (@src2.type = 'mem' | @src2.type = 'mmreg')
			if @src2.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate
