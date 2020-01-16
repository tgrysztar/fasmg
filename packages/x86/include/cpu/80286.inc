
include '80186.inc'

iterate <instr,opcode>, loadall,<0Fh,05h>, clts,<0Fh,06h>

	define x86.instr? db opcode

	calminstruction instr?
		assemble x86.instr?
	end calminstruction

end iterate

calminstruction arpl? dest*,src*
	asmcmd	=x86.=parse_operand =@dest,dest
	asmcmd	=x86.=parse_operand =@src,src
	check	@src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
	jno	invalid_combination_of_operands
	check	@src.size = 2
	jno	invalid_operand_size
	check	@dest.size and not @src.size
	jno	size_ok
	asmcmd	=err 'operand sizes do not match'
	jump	size_ok
    invalid_operand_size:
	asmcmd	=err 'invalid operand size'
    size_ok:
	asmcmd	=x86.=store_instruction 63h,=@dest,=@src.=rm
	exit
    invalid_combination_of_operands:
	asmcmd	=err 'invalid combination of operands'
end calminstruction

iterate <instr,ext,postbyte>, sldt,0,0, str,0,1, lldt,0,2, ltr,0,3, verr,0,4, verw,0,5, smsw,1,4, lmsw,1,6

	calminstruction instr? dest*
		asmcmd	=x86.=parse_operand =@dest,dest
		check	@dest.size and not 2
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		check	@dest.type = 'reg' | @dest.type = 'mem'
		jyes	operand_ok
		asmcmd	=err 'invalid operand'
		exit
	    operand_ok:
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@dest,postbyte
	end calminstruction

end iterate

iterate <instr,postbyte>, lgdt,2, lidt,3, sgdt,0, sidt,1

	calminstruction instr? dest*
		asmcmd	=x86.=parse_operand =@dest,dest
		check	@dest.type = 'mem'
		jyes	operand_ok
		asmcmd	=err 'invalid operand'
		exit
	    operand_ok:
		check	@dest.size and not 5
		jno	store_instruction
		asmcmd	=err 'invalid operand size'
	    store_instruction:
		asmcmd	=x86.=store_instruction <0Fh,1>,=@dest,postbyte
		exit
	end calminstruction

end iterate

iterate <instr,ext>, lar,2, lsl,3

	calminstruction instr? dest*,src*
		asmcmd	=x86.=parse_operand =@dest,dest
		asmcmd	=x86.=parse_operand =@src,src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jno	invalid_combination_of_operands
		check	@src.size and not 2
		jno	size_ok
		asmcmd	=err 'invalid operand size'
	    size_ok:
		asmcmd	=x86.=store_instruction <0Fh,ext>,=@src,=@dest.=rm
		exit
	    invalid_combination_of_operands:
		asmcmd	=err 'invalid combination of operands'
	end calminstruction

end iterate
