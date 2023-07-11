
macro andn? dest*,src*,src2*
	require BMI1
	x86.parse_operand@dest dest
	x86.parse_operand@src src
	x86.parse_operand@src2 src2
	if @dest.type = 'reg' & @src.type = 'reg' & (@src2.type = 'mem' | @src2.type = 'reg')
		if @dest.size < 4
			err 'invalid operand size'
		else if @src.size <> @dest.size | @src2.size and not @dest.size
			err 'operand sizes do not match'
		end if
		if @dest.size = 8
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction@src2 16,VEX_0F38_W1,0F2h,@dest.rm,@src.rm
		else
			AVX.store_instruction@src2 16,VEX_0F38_W0,0F2h,@dest.rm,@src.rm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro bextr? dest*,src*,src2*
	require BMI1
	x86.parse_operand@dest dest
	x86.parse_operand@src src
	x86.parse_operand@src2 src2
	if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg') & @src2.type = 'reg'
		if @dest.size < 4
			err 'invalid operand size'
		else if @src.size and not @dest.size | @src2.size <> @dest.size
			err 'operand sizes do not match'
		end if
		if @dest.size = 8
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction@src 16,VEX_0F38_W1,0F7h,@dest.rm,@src2.rm
		else
			AVX.store_instruction@src 16,VEX_0F38_W0,0F7h,@dest.rm,@src2.rm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,opcode,postbyte>, blsi,0F3h,3, blsmsk,0F3h,2, blsr,0F3h,1

	macro instr? dest*,src*
		require BMI1
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
			if @dest.size < 4
				err 'invalid operand size'
			else if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			if @dest.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX.store_instruction@src 16,VEX_0F38_W1,opcode,postbyte,@dest.rm
			else
				AVX.store_instruction@src 16,VEX_0F38_W0,opcode,postbyte,@dest.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, lzcnt,0BDh, tzcnt,0BCh

	macro instr? dest*,src*
		require BMI1
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		if @dest.type = 'reg' & ( @src.type = 'reg' | @src.type = 'mem' )
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			@src.opcode_prefix = 0F3h
			if @dest.size > 1
				x86.select_operand_prefix@src @dest.size
				x86.store_instruction@src <0Fh,opcode>,@dest.rm
			else
				err 'invalid operand size'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, bzhi,0F5h

	macro instr? dest*,src*,src2*
		require BMI2
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		x86.parse_operand@src2 src2
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg') & @src2.type = 'reg'
			if @dest.size < 4
				err 'invalid operand size'
			else if @src.size and not @dest.size | @src2.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @dest.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX.store_instruction@src 16,VEX_0F38_W1,opcode,@dest.rm,@src2.rm
			else
				AVX.store_instruction@src 16,VEX_0F38_W0,opcode,@dest.rm,@src2.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mp,opcode>, mulx,VEX_F2_0F38,0F6h, pdep,VEX_F2_0F38,0F5h, pext,VEX_F3_0F38,0F5h

	macro instr? dest*,src*,src2*
		require BMI2
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		x86.parse_operand@src2 src2
		if @dest.type = 'reg' & @src.type = 'reg' & (@src2.type = 'mem' | @src2.type = 'reg')
			if @dest.size < 4
				err 'invalid operand size'
			else if @src.size <> @dest.size | @src2.size and not @dest.size
				err 'operand sizes do not match'
			end if
			if @dest.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX.store_instruction@src2 16,vex_mp#_W1,opcode,@dest.rm,@src.rm
			else
				AVX.store_instruction@src2 16,vex_mp#_W0,opcode,@dest.rm,@src.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro rorx? dest*,src*,src2*
	require BMI2
	x86.parse_operand@dest dest
	x86.parse_operand@src src
	x86.parse_operand@src2 src2
	if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg') & @src2.type = 'imm'
		if @dest.size < 4 | @src2.size and not 1
			err 'invalid operand size'
		else if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		if @dest.size = 8
			if x86.mode < 64
				err 'instruction requires long mode'
			end if
			AVX.store_instruction@src 16,VEX_F2_0F3A_W1,0F0h,@dest.rm,,1,@src2.imm
		else
			AVX.store_instruction@src 16,VEX_F2_0F3A_W0,0F0h,@dest.rm,,1,@src2.imm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mp,opcode>, sarx,VEX_F3_0F38,0F7h, shlx,VEX_66_0F38,0F7h, shrx,VEX_F2_0F38,0F7h

	macro instr? dest*,src*,src2*
		require BMI2
		x86.parse_operand@dest dest
		x86.parse_operand@src src
		x86.parse_operand@src2 src2
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg') & @src2.type = 'reg'
			if @dest.size < 4
				err 'invalid operand size'
			else if @src.size and not @dest.size | @src2.size <> @dest.size
				err 'operand sizes do not match'
			end if
			if @dest.size = 8
				if x86.mode < 64
					err 'instruction requires long mode'
				end if
				AVX.store_instruction@src 16,vex_mp#_W1,opcode,@dest.rm,@src2.rm
			else
				AVX.store_instruction@src 16,vex_mp#_W0,opcode,@dest.rm,@src2.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpermb,VEX_66_0F38_W0,8Dh, vpermi2b,VEX_66_0F38_W0,75h, vpermt2b,VEX_66_0F38_W0,7Dh

	macro instr? dest*,src*,src2*
		require AVX512_VBMI
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,opcode>, vpmultishiftqb,83h

	macro instr? dest*,src*,src2*
		require AVX512_VBMI
		AVX_512.basic_instruction_bcst VEX_66_0F38_W1,EVEX_REQUIRED+EVEX_VL,opcode,8,dest,src,src2
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpshldw,VEX_66_0F3A_W1,70h, vpshrdw,VEX_66_0F3A_W1,72h

	macro instr? dest*,src*,src2*,aux*&
		require AVX512_VBMI2
		AVX_512.basic_instruction_imm8 vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2,aux
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpshldd,4,VEX_66_0F3A_W0,71h, vpshldq,8,VEX_66_0F3A_W1,71h, \
				     vpshrdd,4,VEX_66_0F3A_W0,73h, vpshrdq,8,VEX_66_0F3A_W1,73h

	macro instr? dest*,src*,src2*,aux*&
		require AVX512_VBMI2
		AVX_512.basic_instruction_bcst_imm8 vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src,src2,aux
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpshldvw,VEX_66_0F38_W1,70h, vpshrdvw,VEX_66_0F38_W1,72h

	macro instr? dest*,src*,src2*
		require AVX512_VBMI2
		AVX_512.basic_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src,src2
	end macro

end iterate

iterate <instr,unit,vex_mpw,opcode>, vpshldvd,4,VEX_66_0F38_W0,71h, vpshldvq,8,VEX_66_0F38_W1,71h, \
				     vpshrdvd,4,VEX_66_0F38_W0,73h, vpshrdvq,8,VEX_66_0F38_W1,73h

	macro instr? dest*,src*,src2*
		require AVX512_VBMI2
		AVX_512.basic_instruction_bcst vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,unit,dest,src,src2
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpcompressb,VEX_66_0F38_W0,63h, vpcompressw,VEX_66_0F38_W1,63h

	macro instr? dest*,src*
		require AVX512_VBMI2
		AVX_512.parse_k1z_operand@dest dest
		AVX_512.parse_operand@src src
		if (@dest.type = 'mmreg' | @dest.type = 'mem') & @src.type = 'mmreg'
			if @dest.size and not @src.size
				err 'operand sizes do not match'
			end if
			AVX_512.store_instruction@dest @src.size,vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,@dest.mask,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mpw,opcode>, vpexpandb,VEX_66_0F38_W0,62h, vpexpandw,VEX_66_0F38_W1,62h

	macro instr? dest*,src*
		require AVX512_VBMI2
		AVX_512.single_source_instruction vex_mpw,EVEX_REQUIRED+EVEX_VL,opcode,0,dest,src
	end macro

end iterate
