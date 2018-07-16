
include 'p5.inc'

macro sysenter?
	db 0Fh,34h
end macro

macro sysexit?
	db 0Fh,35h
end macro

macro x86.conditional_move ext,dest,src
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
	end if
	if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		x86.select_operand_prefix @src,@dest.size
		x86.store_instruction <0Fh,ext>,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro cmovo? dest*,src*
	x86.conditional_move 40h,dest,src
end macro

macro cmovno? dest*,src*
	x86.conditional_move 41h,dest,src
end macro

macro cmovb? dest*,src*
	x86.conditional_move 42h,dest,src
end macro

macro cmovc? dest*,src*
	x86.conditional_move 42h,dest,src
end macro

macro cmovnae? dest*,src*
	x86.conditional_move 42h,dest,src
end macro

macro cmovae? dest*,src*
	x86.conditional_move 43h,dest,src
end macro

macro cmovnb? dest*,src*
	x86.conditional_move 43h,dest,src
end macro

macro cmovnc? dest*,src*
	x86.conditional_move 43h,dest,src
end macro

macro cmove? dest*,src*
	x86.conditional_move 44h,dest,src
end macro

macro cmovz? dest*,src*
	x86.conditional_move 44h,dest,src
end macro

macro cmovne? dest*,src*
	x86.conditional_move 45h,dest,src
end macro

macro cmovnz? dest*,src*
	x86.conditional_move 45h,dest,src
end macro

macro cmovbe? dest*,src*
	x86.conditional_move 46h,dest,src
end macro

macro cmovna? dest*,src*
	x86.conditional_move 46h,dest,src
end macro

macro cmova? dest*,src*
	x86.conditional_move 47h,dest,src
end macro

macro cmovnbe? dest*,src*
	x86.conditional_move 47h,dest,src
end macro

macro cmovs? dest*,src*
	x86.conditional_move 48h,dest,src
end macro

macro cmovns? dest*,src*
	x86.conditional_move 49h,dest,src
end macro

macro cmovp? dest*,src*
	x86.conditional_move 4Ah,dest,src
end macro

macro cmovpe? dest*,src*
	x86.conditional_move 4Ah,dest,src
end macro

macro cmovnp? dest*,src*
	x86.conditional_move 4Bh,dest,src
end macro

macro cmovpo? dest*,src*
	x86.conditional_move 4Bh,dest,src
end macro

macro cmovl? dest*,src*
	x86.conditional_move 4Ch,dest,src
end macro

macro cmovnge? dest*,src*
	x86.conditional_move 4Ch,dest,src
end macro

macro cmovge? dest*,src*
	x86.conditional_move 4Dh,dest,src
end macro

macro cmovnl? dest*,src*
	x86.conditional_move 4Dh,dest,src
end macro

macro cmovle? dest*,src*
	x86.conditional_move 4Eh,dest,src
end macro

macro cmovng? dest*,src*
	x86.conditional_move 4Eh,dest,src
end macro

macro cmovg? dest*,src*
	x86.conditional_move 4Fh,dest,src
end macro

macro cmovnle? dest*,src*
	x86.conditional_move 4Fh,dest,src
end macro
