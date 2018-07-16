
include '8086.inc'

define @aux

macro push? src*
	x86.parse_operand @src,src
	if @src.size <> 0 & @src.size <> 2
		err 'invalid operand size'
	end if
	if @src.type = 'mem'
		x86.store_instruction 0FFh,@src,110b
	else if @src.type = 'reg'
		db 50h + @src.rm
	else if @src.type = 'sreg'
		db 6 + @src.rm shl 3
	else if @src.type = 'imm'
		if @src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
			db 6Ah
			db @src.imm
		else if @src.imm relativeto 0 & @src.imm-10000h >= -80h & @src.imm < 10000h
			@src.imm = @src.imm - 10000h
			db 6Ah
			db @src.imm
		else
			db 68h
			dw @src.imm
		end if
	else
		err 'invalid operand'
	end if
end macro

macro pop? dest*
	x86.parse_operand @dest,dest
	if @dest.size <> 0 & @dest.size <> 2
		err 'invalid operand size'
	end if
	if @dest.type = 'mem'
		x86.store_instruction 8Fh,@dest,0
	else if @dest.type = 'reg'
		db 58h + @dest.rm
	else if @dest.type = 'sreg'
		if @dest.rm <> 1
			db 7 + @dest.rm shl 3
		else
			err 'invalid operand'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro pusha?
	db 60h
end macro

macro popa?
	db 61h
end macro

macro x86.three_operand_imul_instruction dest,src1,src2
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src1
	x86.parse_operand @aux,src2
	local size
	if @dest.size = 0 & @src.size = 0 & @aux.size = 0
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
		if @aux.size <> 0 & @aux.size <> size
			err 'operand sizes do not match'
		else if size <> 2
			err 'invalid operand size'
		end if
	end if
	if @aux.type = 'imm' & ( @src.type = 'mem' | @src.type = 'reg' ) & @dest.type = 'reg'
		if @aux.imm relativeto 0 & @aux.imm<80h & @aux.imm>=-80h
			x86.store_instruction 6Bh,@src,@dest.rm,1,@aux.imm
		else if @aux.imm relativeto 0 & @aux.imm-10000h>=-80h & @aux.imm<10000h
			@aux.imm = @aux.imm-10000h
			x86.store_instruction 6Bh,@src,@dest.rm,1,@aux.imm
		else
			x86.store_instruction 69h,@src,@dest.rm,2,@aux.imm
		end if
	else
		err 'invalid operand'
	end if
end macro

macro imul? dest*,src1,src2
	match , src1 src2
		single_operand_instruction 5,dest
	else match , src2
		x86.three_operand_imul_instruction dest,dest,src1
	else
		x86.three_operand_imul_instruction dest,src1,src2
	end match
end macro

macro x86.shift_instruction opcode,dest,cnt
	x86.parse_operand @dest,dest
	x86.parse_operand @src,cnt
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size > 2
		err 'invalid operand size'
	end if
	if @src.type = 'reg' & @src.size = 1 & @src.rm = 1 & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if @dest.size = 2
			x86.store_instruction 0D3h,@dest,opcode
		else
			x86.store_instruction 0D2h,@dest,opcode
		end if
	else if @src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if @dest.size = 2
			if @src.imm = 1
				x86.store_instruction 0D1h,@dest,opcode
			else
				x86.store_instruction 0C1h,@dest,opcode,1,@src.imm
			end if
		else
			if @src.imm = 1
				x86.store_instruction 0D0h,@dest,opcode
			else
				x86.store_instruction 0C0h,@dest,opcode,1,@src.imm
			end if
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro insb?
	db 06Ch
end macro

macro insw?
	db 06Dh
end macro

macro outsb?
	db 06Eh
end macro

macro outsw?
	db 06Fh
end macro

macro ins? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size > 2
		err 'invalid operand size'
	end if
	if @src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
		if @dest.size = 2
			db 06Dh
		else
			db 06Ch
		end if
	else
		err 'invalid operand'
	end if
end macro

macro outs? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @src.size = 0
		err 'operand size not specified'
	else if @src.size > 2
		err 'invalid operand size'
	end if
	if @dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'mem' & @src.mod = 0 & @src.rm = 4
		if @src.segment_prefix & @src.segment_prefix <> 3Eh
			db @src.segment_prefix
		end if
		if @src.size = 2
			db 06Fh
		else
			db 06Eh
		end if
	else
		err 'invalid operand'
	end if
end macro

macro bound? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
		if size <> 2
			err 'invalid operand size'
		end if
	end if
	if @src.type = 'mem' & @dest.type = 'reg'
		x86.store_instruction 62h,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro enter? alloc*,nesting*
	x86.parse_operand @src,alloc
	x86.parse_operand @aux,nesting
	if (@src.size <> 0 & @src.size <> 2) | (@aux.size <> 0 & @aux.size <> 1)
		err 'invalid operand size'
	end if
	if @src.type = 'imm' & @aux.type = 'imm'
		db 0C8h
		dw @src.imm
		db @aux.imm
	else
		err 'invalid operand'
	end if
end macro

macro leave?
	db 0C9h
end macro
