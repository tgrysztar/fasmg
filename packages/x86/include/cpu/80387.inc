
if ~ defined i80387

	restore i80387	; this ensures that symbol cannot be forward-referenced
	i80387 = 1

	include '80287.inc'

	purge fsetpm?

	iterate <instr,opcode>, fprem1,<0D9h,0F5h>, fsincos,<0D9h,0FBh>, fsin,<0D9h,0FEh>, fcos,<0D9h,0FFh>, fucompp,<0DAh,0E9h>

		calminstruction instr?
			asm	db opcode
		end calminstruction

	end iterate

	iterate <instr,postbyte>, fucom,4, fucomp,5

		calminstruction instr? src:st1
			call	x87.parse_operand@src, src
			check	@src.type = 'streg'
			jno	invalid_operand
			xcall	x86.store_instruction@src, (0DDh),(postbyte)
			exit
		    invalid_operand:
			err	'invalid operand'
		end calminstruction

	end iterate


	iterate <instr,postbyte>, fldenv,4, fnstenv,6

		calminstruction instr? dest*
			call	x86.parse_operand@dest, dest
			check	@dest.size & ( ( x86.mode = 16 & @dest.size <> 14 ) | ( x86.mode = 32 & @dest.size <> 28 ) )
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			xcall	x86.store_instruction@dest, (0D9h),(postbyte)
			exit
		    invalid_operand_size:
			err	'invalid operand size'
			exit
		    invalid_operand:
			err	'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, fldenvw,4, fnstenvw,6

		calminstruction instr? dest*
			call	x86.parse_operand@dest, dest
			check	@dest.size and not 14
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			assemble x86.o16
			xcall	x86.store_instruction@dest, (0D9h),(postbyte)
			exit
		    invalid_operand_size:
			err	'invalid operand size'
			exit
		    invalid_operand:
			err	'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, fldenvd,4, fnstenvd,6

		calminstruction instr? dest*
			call	x86.parse_operand@dest, dest
			check	@dest.size and not 28
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			xcall	x86.store_operand_prefix, (4)
			xcall	x86.store_instruction@dest, (0D9h),(postbyte)
			exit
		    invalid_operand_size:
			err	'invalid operand size'
			exit
		    invalid_operand:
			err	'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, frstor,4, fnsave,6

		calminstruction instr? dest*
			call	x86.parse_operand@dest, dest
			check	@dest.size & ( ( x86.mode = 16 & @dest.size <> 94 ) | ( x86.mode = 32 & @dest.size <> 108 ) )
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			xcall	x86.store_instruction@dest, (0DDh),(postbyte)
			exit
		    invalid_operand_size:
			err	'invalid operand size'
			exit
		    invalid_operand:
			err	'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, frstorw,4, fnsavew,6

		calminstruction instr? dest*
			call	x86.parse_operand@dest, dest
			check	@dest.size and not 94
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			assemble x86.o16
			xcall	x86.store_instruction@dest, (0DDh),(postbyte)
			exit
		    invalid_operand_size:
			err	'invalid operand size'
			exit
		    invalid_operand:
			err	'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, frstord,4, fnsaved,6
		calminstruction instr? dest*
			call	x86.parse_operand@dest, dest
			check	@dest.size and not 108
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			xcall	x86.store_operand_prefix, (4)
			xcall	x86.store_instruction@dest, (0DDh),(postbyte)
			exit
		    invalid_operand_size:
			err	'invalid operand size'
			exit
		    invalid_operand:
			err	'invalid operand'
		end calminstruction

	end iterate

	iterate op,  stenvw, stenvd, savew, saved
		calminstruction f#op? dest*
			assemble x87.fwait?
			asm	fn#op? dest
		end calminstruction
	end iterate

end if