
include 'avx.inc'

macro andn? dest*,src*,src2*
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
			AVX.store_instruction 16,VEX_0F38_W1,0F2h,@src2,@dest.rm,@src.rm
		else
			AVX.store_instruction 16,VEX_0F38_W0,0F2h,@src2,@dest.rm,@src.rm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro bextr? dest*,src*,src2*
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
			AVX.store_instruction 16,VEX_0F38_W1,0F7h,@src,@dest.rm,@src2.rm
		else
			AVX.store_instruction 16,VEX_0F38_W0,0F7h,@src,@dest.rm,@src2.rm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,opcode,postbyte>, blsi,0F3h,3, blmsk,0F3h,2, blsr,0F3h,1

	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
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
				AVX.store_instruction 16,VEX_0F38_W1,opcode,@src,postbyte,@dest.rm
			else
				AVX.store_instruction 16,VEX_0F38_W0,opcode,@src,postbyte,@dest.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, lzcnt,0BDh, tzcnt,0BCh

	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'reg' & ( @src.type = 'reg' | @src.type = 'mem' )
			if @src.size and not @dest.size
				err 'operand sizes do not match'
			end if
			@src.opcode_prefix = 0F3h
			if @dest.size > 1
				x86.select_operand_prefix @src,@dest.size
				x86.store_instruction <0Fh,opcode>,@src,@dest.rm
			else
				err 'invalid operand size'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate
