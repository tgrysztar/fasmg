
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,opcode>, kandb,41h, kandnb,42h, knotb,44h, korb,45h, kxnorb,46h, kxorb,47h, kaddb,4Ah

	macro instr? dest*,src*,src2*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
			AVX.store_instruction 32,VEX_66_0F_W0,opcode,@src2,@dest.rm,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, knotb,44h, kortestb,98h, ktestb,99h

	macro instr? dest*,src*
		AVX_512.parse_operand @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'maskreg' & @src.type = 'maskreg'
			AVX.store_instruction 16,VEX_66_0F_W0,opcode,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro kmovb? dest*,src*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	if @dest.type = 'maskreg' & (@src.type = 'maskreg' | @src.type = 'mem')
		if @src.type = 'mem' & @src.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction 16,VEX_66_0F_W0,90h,@src,@dest.rm
	else if @dest.type = 'mem' & @src.type = 'maskreg'
		if @dest.size and not 1
			err 'invalid operand size'
		end if
		AVX.store_instruction 16,VEX_66_0F_W0,91h,@dest,@src.rm
	else if @dest.type = 'maskreg' & @src.type = 'reg'
		if @src.size <> 4
			err 'invalid operand size'
		end if
		AVX.store_instruction 16,VEX_66_0F_W0,92h,@src,@dest.rm
	else if @dest.type = 'reg' & @src.type = 'maskreg'
		if @dest.size <> 4
			err 'invalid operand size'
		end if
		AVX.store_instruction 16,VEX_66_0F_W0,93h,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro kaddw? dest*,src*,src2*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	AVX_512.parse_operand @src2,src2
	if @dest.type = 'maskreg' & @src.type = 'maskreg' & @src2.type = 'maskreg'
		AVX.store_instruction 32,VEX_0F_W0,opcode,@src2,@dest.rm,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro ktestw? dest*,src*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	if @dest.type = 'maskreg' & @src.type = 'maskreg'
		AVX.store_instruction 16,VEX_0F_W0,opcode,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mpw,opcode>, kshiftrb,VEX_66_0F3A_W0,30h, kshiftlb,VEX_66_0F3A_W0,32h

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

iterate <instr,opcode>, and,54h, andn,55h, or,56h, xor,57h

	macro v#instr#pd? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_66_0F_W0,EVEX_W1+EVEX_VL,opcode,8,dest,src,src2
	end macro

	macro v#instr#ps? dest*,src*,src2*&
		AVX_512.basic_instruction_bcst VEX_0F_W0,EVEX_AS_VEX+EVEX_VL,opcode,4,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vbroadcastf32x2,19h, vbroadcasti32x2,59h

	macro instr? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mmreg' | @src.type = 'mem')
			if @dest.size = 16 | (@src.type = 'mmreg' & @src.size <> 16) | (@src.type = 'mem' & @src.size and not 8)
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction @dest.size,VEX_66_0F38_W0,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode,msize>, vbroadcastf32x8,VEX_66_0F38_W0,1Bh,32, vbroadcastf64x2,VEX_66_0F38_W1,1Ah,16, \
				      vbroadcasti32x8,VEX_66_0F38_W0,5Bh,32, vbroadcasti64x2,VEX_66_0F38_W1,5Ah,16

	macro instr? dest*,src*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		if @dest.type = 'mmreg' & @src.type = 'mem'
			if @dest.size <= msize | @src.size and not msize
				err 'invalid operand size'
			end if
			@src.memsize = msize
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvtpd2qq,VEX_66_0F_W1,7Bh, vcvtpd2uqq,VEX_66_0F_W1,79h, \
				vcvtqq2pd,VEX_F3_0F_W1,0E6h, vcvtuqq2pd,VEX_F3_0F_W1,7Ah

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst_er vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvttpd2qq,VEX_66_0F_W1,7Ah, vcvttpd2uqq,VEX_66_0F_W1,78h

	macro instr? dest*,src*&
		AVX_512.single_source_instruction_bcst_sae vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvtps2qq,VEX_66_0F_W0,7Bh, vcvtps2uqq,VEX_66_0F_W0,79h

	macro instr? dest*,src_er*&
		AVX_512.parse_operand_k1z @dest,dest
		match src=,er, src_er
			AVX_512.parse_operand @src,src
			AVX_512.parse_er @src,er,32
		else
			AVX_512.parse_operand_bcst @src,src_er,4
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
				err 'invalid operand size'
			end if
			if @src.memsize = 0
				@src.memsize = @dest.size shr 1
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvtqq2ps,VEX_0F_W1,5Bh, vcvtuqq2ps,VEX_F2_0F_W1,7Ah

	macro instr? dest*,src_er*&
		AVX_512.parse_operand_k1z @dest,dest
		match src=,er, src_er
			AVX_512.parse_operand @src,src
			AVX_512.parse_er @src,er
		else
			AVX_512.parse_operand_bcst @src,src_er,8
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size = 0
				if @dest.size = 16
					err 'operand size not specified'
				else
					@src.size = 64
				end if
			end if
			if (@src.size shr 1 - 1) and not 15 + 16 <> @dest.size | @src.size > 64
				err 'invalid operand size'
			end if
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vcvttps2qq,VEX_66_0F_W0,7Ah, vcvttps2uqq,VEX_66_0F_W0,78h

	macro instr? dest*,src_sae*&
		AVX_512.parse_operand_k1z @dest,dest
		match src=,sae, src_sae
			AVX_512.parse_operand @src,src
			AVX_512.parse_sae @src,sae
		else
			AVX_512.parse_operand_bcst @src,src_sae,4
		end match
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if (@src.type = 'mem' & @src.size and not (@dest.size shr 1)) | (@src.type = 'mmreg' & (@dest.size shr 1 - 1) and not 15 + 16 <> @src.size)
				err 'invalid operand size'
			end if
			if @src.memsize = 0
				@src.memsize = @dest.size shr 1
			end if
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src,@dest.mask,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode,msize>, vextractf32x8,VEX_66_0F3A_W0,1Bh,32, vextractf64x2,VEX_66_0F3A_W1,19h,16, \
				      vextracti32x8,VEX_66_0F3A_W0,3Bh,32, vextracti64x2,VEX_66_0F3A_W1,39h,16

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
			if @dest.size and not msize | @src.size <= msize | @aux.size and not 1
				err 'invalid operand size'
			end if
			@dest.memsize = msize
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest,@dest.mask,@src.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode,msize>, vinsertf32x8,VEX_66_0F3A_W0,1Ah,32, vinsertf64x2,VEX_66_0F3A_W1,18h,16, \
				      vinserti32x8,VEX_66_0F3A_W0,3Ah,32, vinserti64x2,VEX_66_0F3A_W1,38h,16

	macro instr? dest*,src*,src2*,aux*
		AVX_512.parse_operand_k1z @dest,dest
		AVX_512.parse_operand @src,src
		AVX_512.parse_operand @src2,src2
		x86.parse_operand @aux,aux
		if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'mmreg' | @src2.type = 'mem') & @aux.type = 'imm'
			if @dest.size <= msize | @src.size <= msize | @src2.size and not msize | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src2.memsize = msize
			AVX_512.store_instruction @dest.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@src2,@dest.mask,@dest.rm,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw>, vfpclasspd,8,VEX_66_0F3A_W1, vfpclassps,4,VEX_66_0F3A_W0

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand_k1 @dest,dest
		AVX_512.parse_operand_bcst @src,src,unit
		x86.parse_operand @aux,aux
		if @dest.type = 'maskreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if @src.size = 0
				err 'operand size not specified'
			else if (@src.size <> 16 & @src.size <> 32 & @src.size <> 64) | @aux.size and not 1
				err 'invalid operand size'
			end if
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,66h,@src,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,unit,vex_mpw>, vfpclasssd,8,VEX_66_0F3A_W1, vfpclassss,4,VEX_66_0F3A_W0

	macro instr? dest*,src*,aux*
		AVX_512.parse_operand_k1 @dest,dest
		AVX_512.parse_operand @src,src
		x86.parse_operand @aux,aux
		if @dest.type = 'maskreg' & (@src.type = 'mem' | @src.type = 'mmreg') & @aux.type = 'imm'
			if (@src.type = 'mem' & @src.size and not unit) | (@src.type = 'mmreg' & @src.size <> 16) | @aux.size and not 1
				err 'invalid operand size'
			end if
			@src.memsize = 16
			AVX_512.store_instruction @src.size,vex_mpw,EVEX_REQUIRED,67h,@src,@dest.mask,@dest.rm,,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro vpextrd? dest*,src*,aux*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	x86.parse_operand @aux,aux
	if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if (@dest.type = 'reg' & @dest.size <> 4 & (x86.mode < 64 | @dest.size <> 8)) | (@dest.type = 'mem' & @dest.size and not 4) | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		@dest.memsize = 4
		AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,16h,@dest,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpinsrd? dest*,src*,src2*,aux*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	AVX_512.parse_operand @src2,src2
	x86.parse_operand @aux,aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
		if @dest.size <> 16 | @src.size <> 16 | (@src2.type = 'reg' & @src2.size <> 4) | (@src2.type = 'mem' & @src2.size and not 4) | @aux.size and not 1
			err 'invalid operand size'
		end if
		@src2.memsize = 4
		AVX_512.store_instruction 16,VEX_66_0F3A_W0,EVEX_AS_VEX,22h,@src2,0,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpextrq? dest*,src*,aux*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	x86.parse_operand @aux,aux
	if (@dest.type = 'reg' | @dest.type = 'mem') & @src.type = 'mmreg' & @aux.type = 'imm'
		if @dest.size and not 8 | @src.size <> 16 | @aux.size and not 1
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		@dest.memsize = 8
		AVX_512.store_instruction 16,VEX_66_0F3A_W1,EVEX_AS_VEX,16h,@dest,0,@src.rm,,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpinsrq? dest*,src*,src2*,aux*
	AVX_512.parse_operand @dest,dest
	AVX_512.parse_operand @src,src
	x86.parse_operand @src2,src2
	x86.parse_operand @aux,aux
	if @dest.type = 'mmreg' & @src.type = 'mmreg' & (@src2.type = 'reg' | @src2.type = 'mem') & @aux.type = 'imm'
		if @dest.size <> 16 | @src.size <> 16 | @src2.size and not 8 | @aux.size and not 1
			err 'invalid operand size'
		end if
		if x86.mode < 64
			err 'instruction requires long mode'
		end if
		@src2.memsize = 8
		AVX_512.store_instruction 16,VEX_66_0F3A_W1,EVEX_AS_VEX,22h,@src2,0,@dest.rm,@src.rm,1,@aux.imm
	else
		err 'invalid combination of operands'
	end if
end macro

macro vpmullq? dest*,src*,src2*
	AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,40h,8,dest,src,src2
end macro

iterate <instr,vex_mpw,opcode>, vpmovm2d,VEX_F3_0F38_W0,38h, vpmovm2q,VEX_F3_0F38_W1,38h

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

iterate <instr,vex_mpw,opcode>, vpmovd2m,VEX_F3_0F38_W0,39h, vpmovq2m,VEX_F3_0F38_W1,39h

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

iterate <instr,opcode>, range,50h

	macro v#instr#pd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2,aux
	end macro

	macro v#instr#ps? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_VL,opcode,4,dest,src,src2,aux
	end macro

	macro v#instr#sd? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED,opcode+1,8,dest,src,src2,aux
	end macro

	macro v#instr#ss? dest*,src*,src2*,aux*&
		AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED,opcode+1,4,dest,src,src2,aux
	end macro

end iterate

macro vreducepd? dest*,src*,aux*&
	AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED+EVEX_VL,56h,8,dest,src,aux
end macro

macro vreduceps? dest*,src*,aux*&
	AVX_512.single_source_instruction_bcst_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED+EVEX_VL,56h,4,dest,src,aux
end macro

macro vreducesd? dest*,src*,src2*,aux*&
	AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W1,EVEX_REQUIRED,57h,8,dest,src,src2,aux
end macro

macro vreducess? dest*,src*,src2*,aux*&
	AVX_512.basic_instruction_sae_imm8 VEX_66_0F3A_W0,EVEX_REQUIRED,57h,4,dest,src,src2,aux
end macro
