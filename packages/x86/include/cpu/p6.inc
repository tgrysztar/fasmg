
include 'p5.inc'

macro sysenter?
	db 0Fh,34h
end macro

macro sysexit?
	db 0Fh,35h
end macro

iterate <cond,code>, o,0, no,1, c,2, b,2, nae,2, nc,3, nb,3, ae,3, z,4, e,4, nz,5, ne,5, na,6, be,6, a,7, nbe,7, \
		     s,8, ns,9, p,0Ah, pe,0Ah, np,0Bh, po,0Bh, l,0Ch, nge,0Ch, nl,0Dh, ge,0Dh, ng,0Eh, le,0Eh, g,0Fh, nle,0Fh

	macro cmov#cond? dest*,src*
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
			x86.store_instruction <0Fh,40h+code>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, fcmovb,<0DAh,0C0h>, fcmove,<0DAh,0C8h>, fcmovbe,<0DAh,0D0h>, fcmovu,<0DAh,0D8h>, \
			fcmovnb,<0DBh,0C0h>, fcmovne,<0DBh,0C8h>, fcmovnbe,<0DBh,0D0h>, fcmovnu,<0DBh,0D8h>

	macro instr? dest*,src*
		x87.parse_operand @dest,dest
		x87.parse_operand @src,src
		if @dest.type = 'streg' & @dest.rm = 0 & @src.type = 'streg'
			db opcode + @src.rm
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,opcode,postbyte>, fucomi,0DBh,5, fucomip,0DFh,5, fcomi,0DBh,6, fcomip,0DFh,6

	macro instr? src:st1
		x87.parse_operand @src,src
		if @src.type = 'streg'
			db opcode, 11b shl 6 + postbyte shl 3 + @src.rm
		else
			err 'invalid operand'
		end if
	end macro

end iterate
