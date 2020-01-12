
include 'sse4.1.inc'

iterate <instr,supp>, pcmpgtq,37h
	macro instr? dest*,src*
		SSE.basic_instruction 66h,<38h,supp>,16,dest,src
	end macro
end iterate

iterate <instr,supp>, pcmpestrm,60h, pcmpestri,61h, pcmpistrm,62h, pcmpistri,63h
	macro instr? dest*,src*,imm*
		SSE.basic_instruction_imm8 66h,<3Ah,supp>,16,dest,src,imm
	end macro
end iterate

macro crc32? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.type = 'reg' & ( @src.type = 'reg' | @src.type = 'mem' )
		if @dest.size <> 4 & ( @dest.size <> 8 | x86.mode <> 64 )
			err 'invalid operand size'
		end if
		@src.opcode_prefix = 0F2h
		if @src.size > 1
			x86.select_operand_prefix @src,@src.size
			x86.store_instruction <0Fh,38h,0F1h>,@src,@dest.rm
		else if @src.size > 0
			x86.store_instruction <0Fh,38h,0F0h>,@src,@dest.rm
		else
			err 'operand size not specified'
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro popcnt? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.type = 'reg' & ( @src.type = 'reg' | @src.type = 'mem' )
		if @src.size and not @dest.size
			err 'operand sizes do not match'
		end if
		@src.opcode_prefix = 0F3h
		if @dest.size > 1
			x86.select_operand_prefix @src,@dest.size
			x86.store_instruction <0Fh,0B8h>,@src,@dest.rm
		else
			err 'invalid operand size'
		end if
	else
		err 'invalid combination of operands'
	end if
end macro
