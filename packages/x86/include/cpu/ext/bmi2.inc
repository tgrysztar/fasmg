
include 'bmi1.inc'

iterate <instr,opcode>, bzhi,0F5h

	macro instr? dest*,src*,src2*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		x86.parse_operand @src2,src2
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
				AVX.store_instruction 16,VEX_0F38_W1,opcode,@src,@dest.rm,@src2.rm
			else
				AVX.store_instruction 16,VEX_0F38_W0,opcode,@src,@dest.rm,@src2.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,vex_mp,opcode>, mulx,VEX_F2_0F38,0F6h, pdep,VEX_F2_0F38,0F5h, pext,VEX_F3_0F38,0F5h

	macro instr? dest*,src*,src2*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		x86.parse_operand @src2,src2
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
				AVX.store_instruction 16,vex_mp#_W1,opcode,@src2,@dest.rm,@src.rm
			else
				AVX.store_instruction 16,vex_mp#_W0,opcode,@src2,@dest.rm,@src.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

macro rorx? dest*,src*,src2*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	x86.parse_operand @src2,src2
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
			AVX.store_instruction 16,VEX_F2_0F3A_W1,0F0h,@src,@dest.rm,,1,@src2.imm
		else
			AVX.store_instruction 16,VEX_F2_0F3A_W0,0F0h,@src,@dest.rm,,1,@src2.imm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,vex_mp,opcode>, sarx,VEX_F3_0F38,0F7h, shlx,VEX_66_0F38,0F7h, shrx,VEX_F2_0F38,0F7h

	macro instr? dest*,src*,src2*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		x86.parse_operand @src2,src2
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
				AVX.store_instruction 16,vex_mp#_W1,opcode,@src,@dest.rm,@src2.rm
			else
				AVX.store_instruction 16,vex_mp#_W0,opcode,@src,@dest.rm,@src2.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate
