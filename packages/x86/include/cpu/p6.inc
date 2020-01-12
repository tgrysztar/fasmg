
include 'p5.inc'

iterate <instr,opcode>, sysenter,<0Fh,34h>, sysexit,<0Fh,35h>

	define x86.instr? db opcode

	calminstruction instr?
		assemble x86.instr?
	end calminstruction

end iterate

iterate <cond,code>, o,0, no,1, c,2, b,2, nae,2, nc,3, nb,3, ae,3, z,4, e,4, nz,5, ne,5, na,6, be,6, a,7, nbe,7, \
		     s,8, ns,9, p,0Ah, pe,0Ah, np,0Bh, po,0Bh, l,0Ch, nge,0Ch, nl,0Dh, ge,0Dh, ng,0Eh, le,0Eh, g,0Fh, nle,0Fh

	calminstruction cmov#cond? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	cmov_rm_reg
		asmcmd	=err 'invalid combination of operands'
		exit
	    cmov_rm_reg:
		check	@src.size and not @dest.size
		jno	cmov_rm_reg_ok
		asmcmd	=err 'operand sizes do not match'
	      cmov_rm_reg_ok:
		asmcmd	=x86.=select_operand_prefix =@src,=@dest.=size
		asmcmd	=x86.=store_instruction <0Fh,40h+code>,=@src,=@dest.=rm
	end calminstruction

end iterate

iterate <instr,opcode>, fcmovb,<0DAh,0C0h>, fcmove,<0DAh,0C8h>, fcmovbe,<0DAh,0D0h>, fcmovu,<0DAh,0D8h>, \
			fcmovnb,<0DBh,0C0h>, fcmovne,<0DBh,0C8h>, fcmovnbe,<0DBh,0D0h>, fcmovnu,<0DBh,0D8h>

	calminstruction instr? dest*,src*
		asmcmd	=x87.=parse_operand =@dest,dest
		asmcmd	=x87.=parse_operand =@src,src
		check	@dest.type = 'streg' & @dest.rm = 0 & @src.type = 'streg'
		jyes	ok
		asmcmd	=err 'invalid operand'
		exit
	    ok:
		asmcmd	=db opcode + =@src.=rm
	end calminstruction

end iterate

iterate <instr,opcode,postbyte>, fucomi,0DBh,5, fucomip,0DFh,5, fcomi,0DBh,6, fcomip,0DFh,6

	calminstruction instr? src:st1
		asmcmd	=x87.=parse_operand =@src,src
		check	@src.type = 'streg'
		jyes	ok
		asmcmd	=err 'invalid operand'
		exit
	    ok:
		local	modrm
		compute modrm, 11b shl 6 + postbyte shl 3 + @src.rm
		asmcmd	=db opcode, modrm
	end calminstruction

end iterate
