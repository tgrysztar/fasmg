
calminstruction element? definition
	local	name, meta
	match	=tr? any, definition
	jyes	skip
	arrange definition, =element? definition
	assemble definition
    skip:
end calminstruction

include '80486.inc'

purge element?

iterate <instr,opcode>, wrmsr,<0Fh,30h>, rdtsc,<0Fh,31h>, rdmsr,<0Fh,32h>, rdpmc,<0Fh,33h>, cpuid,<0Fh,0A2h>, rsm,<0Fh,0AAh>

	calminstruction instr?
		asm	db opcode
	end calminstruction

end iterate

calminstruction cmpxchg8b? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.type = 'mem'
	jyes	operand_ok
	err	'invalid operand'
	exit
    operand_ok:
	check	@dest.size and not 8
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	xcall	x86.store_instruction@dest, <0Fh,0C7h>,(1)
end calminstruction

include '80387.inc'
