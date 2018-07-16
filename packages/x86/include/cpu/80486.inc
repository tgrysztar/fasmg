
include '80386.inc'

purge loadall?
purge xbts?,ibts?

macro x86.basic_486_instruction basecode,dest,src
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size = 0 & @src.size = 0
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
	end if
	if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if defined size & size > 1
			x86.select_operand_prefix @dest,size
			x86.store_instruction <0Fh,basecode+1>,@dest,@src.rm
		else
			x86.store_instruction <0Fh,basecode>,@dest,@src.rm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro cmpxchg? dest*,src*
	x86.basic_486_instruction 0B0h,dest,src
end macro

macro xadd? dest*,src*
	x86.basic_486_instruction 0C0h,dest,src
end macro

macro bswap? dest*
	x86.parse_operand @dest,dest
	if @dest.type = 'reg' & @dest.size = 4
		x86.store_operand_prefix @dest.size
		db 0Fh,0C8h + @dest.rm
	else
		err 'invalid operand'
	end if
end macro

macro invlpg? dest*
	x86.parse_operand @dest,dest
	if @dest.type = 'mem'
		x86.store_instruction <0Fh,1>,@dest,7
	else
		err 'invalid operand'
	end if
end macro

macro invd?
	db 0Fh,8
end macro

macro wbinvd?
	db 0Fh,9
end macro
