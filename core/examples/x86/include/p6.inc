
include 'p5.inc'

iterate <instr,opcode>, sysenter,<0Fh,34h>, sysexit,<0Fh,35h>

	calminstruction instr?
		asm	db opcode
	end calminstruction

end iterate

iterate <cond,code>, o,0, no,1, c,2, b,2, nae,2, nc,3, nb,3, ae,3, z,4, e,4, nz,5, ne,5, na,6, be,6, a,7, nbe,7, \
		     s,8, ns,9, p,0Ah, pe,0Ah, np,0Bh, po,0Bh, l,0Ch, nge,0Ch, nl,0Dh, ge,0Dh, ng,0Eh, le,0Eh, g,0Fh, nle,0Fh

	calminstruction cmov#cond? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	cmov_rm_reg
		err	'invalid combination of operands'
		exit
	    cmov_rm_reg:
		check	@src.size and not @dest.size
		jno	cmov_rm_reg_ok
		err	'operand sizes do not match'
	      cmov_rm_reg_ok:
		call	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, <0Fh,40h+code>,@dest.rm
	end calminstruction

end iterate

iterate <instr,opcode>, fcmovb,<0DAh,0C0h>, fcmove,<0DAh,0C8h>, fcmovbe,<0DAh,0D0h>, fcmovu,<0DAh,0D8h>, \
			fcmovnb,<0DBh,0C0h>, fcmovne,<0DBh,0C8h>, fcmovnbe,<0DBh,0D0h>, fcmovnu,<0DBh,0D8h>

	calminstruction instr? dest*,src*
		call	x87.parse_operand@dest, dest
		call	x87.parse_operand@src, src
		check	@dest.type = 'streg' & @dest.rm = 0 & @src.type = 'streg'
		jyes	ok
		err	'invalid operand'
		exit
	    ok:
		asm	db opcode + @src.rm
	end calminstruction

end iterate

iterate <instr,opcode,postbyte>, fucomi,0DBh,5, fucomip,0DFh,5, fcomi,0DBh,6, fcomip,0DFh,6

	calminstruction instr? src:st1
		call	x87.parse_operand@src, src
		check	@src.type = 'streg'
		jyes	ok
		err	'invalid operand'
		exit
	    ok:
		emit	1, opcode
		emit	1, 11b shl 6 + postbyte shl 3 + @src.rm
	end calminstruction

end iterate
