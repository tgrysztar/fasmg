
include '80386.inc'

purge loadall?
purge xbts?,ibts?

iterate <instr,ext>, cmpxchg,0B0h, xadd,0C0h

	calminstruction instr? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	xadd_rm_reg
		asmcmd	=err 'invalid combination of operands'
		exit
	    xadd_rm_reg:
		check	@dest.size and not @src.size
		jno	size_ok
		asmcmd	=err 'operand sizes do not match'
	      size_ok:
		check	@src.size > 1
		jno	xadd_rm_reg_8bit
		asmcmd	=x86.=select_operand_prefix =@dest,=@src.=size
		asmcmd	=x86.=store_instruction <0Fh,ext+1>,=@dest,=@src.=rm
		exit
	      xadd_rm_reg_8bit:
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@dest,=@src.=rm
	end calminstruction

end iterate

calminstruction bswap? dest*
	asmcmd	=x86.=parse_operand =@dest,dest
	check	@dest.type = 'reg' & @dest.size = 4
	jyes	operand_ok
	asmcmd	=err 'invalid operand'
	exit
    operand_ok:
	asmcmd	=x86.=store_operand_prefix =@dest.=size
	local	opcode
	compute opcode, 0C8h + @dest.rm and 111b
	asmcmd	=db 0Fh,opcode
end calminstruction

calminstruction invlpg? dest*
	asmcmd	=x86.=parse_operand =@dest,dest
	check	@dest.type = 'mem'
	jyes	operand_ok
	asmcmd	=err 'invalid operand'
	exit
    operand_ok:
	asmcmd	=x86.=store_instruction <0Fh,1>,=@dest,7
end calminstruction

iterate <instr,opcode>, invd,<0Fh,8>, wbinvd,<0Fh,9>

	define x86.instr? db opcode

	calminstruction instr?
		assemble x86.instr?
	end calminstruction

end iterate
