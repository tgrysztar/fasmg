
iterate <instr,opcode>, pf2id,1Dh, pfacc,0AEh, pfadd,9Eh, pfmax,0A4h, pfmin,94h, \
			pfmul,0B4h, pfrcp,96h, pfsub,9Ah, pi2fd,0Dh, pi2fw,0Ch, \
			pfsubr,0AAh, pavgusb,0BFh, pfcmpeq,0B0h, pfcmpge,90h, pfcmpgt,0A0h, \
			pfrsqrt,97h, pmulhrw,0B7h, pfrcpit2,0B6h, pfrsqit1,0A7h

	calminstruction instr? dest*,src*
		call	x86.require.3DNow
		call	MMX.parse_operand@dest, dest
		call	MMX.parse_operand@src, src
		check	@src.size and not @dest.size
		jno	size_ok
		err	'operand sizes do not match'
	    size_ok:
		check	@dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
		jno	invalid_combination_of_operands
		xcall	x86.store_instruction@src, <0Fh,0Fh>,@dest.rm,(1),(opcode)
		exit
	    invalid_combination_of_operands:
		err	'invalid combination of operands'
	end calminstruction

end iterate

calminstruction femms?
	call	x86.require.3DNow
	emit	1, 0Fh
	emit	1, 0Eh
end calminstruction

iterate <instr,postbyte>, prefetch,0, prefetchw,1
	calminstruction instr? src*
		call	x86.require.3DNow
		call	MMX.parse_operand@src, src
		check	@src.type = 'mem'
		jno	invalid_operand
		check	@src.size and not 1
		jno	size_ok
		err	'invalid operand size'
	    size_ok:
		xcall	x86.store_instruction@src, <0Fh,0Dh>,(postbyte)
		exit
	    invalid_operand:
		err	'invalid operand'
	end calminstruction
end iterate
