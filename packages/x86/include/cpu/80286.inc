
include '80186.inc'

macro arpl? dest*,src*
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
	if @src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
		x86.store_instruction <63h>,@dest,@src.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,ext,postbyte>, sldt,0,0, str,0,1, lldt,0,2, ltr,0,3, verr,0,4, verw,0,5, smsw,1,4, lmsw,1,6
	macro instr? dest*
		x86.parse_operand @dest,dest
		if @dest.size <> 0 & @dest.size <> 2
			err 'invalid operand size'
		end if
		if @dest.type = 'reg' | @dest.type = 'mem'
			x86.store_instruction <0Fh,ext>,@dest,postbyte
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,postbyte>, lgdt,2, lidt,3, sgdt,0, sidt,1
	macro instr? dest*
		x86.parse_operand @dest,dest
		if @dest.size <> 0 & @dest.size <> 5
			err 'invalid operand size'
		end if
		if @dest.type = 'mem'
			x86.store_instruction <0Fh,1>,@dest,postbyte
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,ext>, lar,2, lsl,3
	macro instr? dest*,src*
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
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
			x86.store_instruction <0Fh,ext>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

macro loadall?
	db 0Fh,05h
end macro

macro clts?
	db 0Fh,06h
end macro
