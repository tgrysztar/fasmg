
element st?

repeat 8, i:0
	element st#i? : st? + i
end repeat

iterate context, @dest,@src,@src2,@aux

	namespace context

		calminstruction x87.parse_operand#context operand

			local	i

			match	=st?(i), operand
			jyes	indexed_streg_operand

			call	x86.parse_operand#context, operand

			check	type = 'imm' & size = 0
			jno	done
			check	imm eq 1 elementof imm & 1 metadataof imm relativeto st?
			jyes	streg_operand
			check	imm relativeto st? & imm = st?
			jno	done

			compute type, 'streg'
			compute mod, 11b
			compute rm, 0

			exit

		    streg_operand:

			compute type, 'streg'
			compute mod, 11b
			compute rm, 1 metadataof imm - st?

			exit

		    indexed_streg_operand:

			compute size, 0
			compute type, 'streg'
			compute mod, 11b
			compute rm, +i

		    done:

		end calminstruction

	end namespace

end iterate

iterate <instr,opcode>, fwait,9Bh, wait,9Bh

	calminstruction instr?
		call	x86.require.8087
		emit	1, opcode
	end calminstruction

end iterate

iterate <instr,byte1,byte2>, fnop,0D9h,0D0h, fchs,0D9h,0E0h, fabs,0D9h,0E1h, ftst,0D9h,0E4h, fxam,0D9h,0E5h, fld1,0D9h,0E8h, \
			     fldl2t,0D9h,0E9h, fldl2e,0D9h,0EAh, fldpi,0D9h,0EBh, fldlg2,0D9h,0ECh, fldln2,0D9h,0EDh, fldz,0D9h,0EEh, \
			     f2xm1,0D9h,0F0h, fyl2x,0D9h,0F1h, fptan,0D9h,0F2h, fpatan,0D9h,0F3h, fxtract,0D9h,0F4h, \
			     fdecstp,0D9h,0F6h, fincstp,0D9h,0F7h, fprem,0D9h,0F8h, \
			     fyl2xp1,0D9h,0F9h, fsqrt,0D9h,0FAh, frndint,0D9h,0FCh, fscale,0D9h,0FDh, \
			     fneni,0DBh,0E0h, fndisi,0DBh,0E1h, fnclex,0DBh,0E2h, fninit,0DBh,0E3h, \
			     fcompp,0DEh,0D9h

	calminstruction instr?
		call	x86.require.8087
		emit	1, byte1
		emit	1, byte2
	end calminstruction

end iterate

iterate <instr,opcode>, fneni,0E0h, fndisi,0E1h

	calminstruction instr?
		call	x86.requireexact.8087
		emit	1, 0DBh
		emit	1, opcode
	end calminstruction

end iterate

iterate op,  eni, disi, clex, init
	calminstruction f#op?
		asm	fwait?
		asm	fn#op?
	end calminstruction
end iterate

calminstruction fsetpm?
	call	x86.requireexact.80287
	emit	1, 0DBh
	emit	1, 0E4h
end calminstruction

iterate <instr,postbyte>, fadd,0, fmul,1, fsub,4, fsubr,5, fdiv,6, fdivr,7

	calminstruction instr? operand&
		call	x86.require.8087
		local	dest, src
		match	dest=,src, operand
		jyes	st
		call	x87.parse_operand@dest, operand
		check	@dest.type = 'mem'
		jno	invalid_combination_of_operands
		check	@dest.size = 4
		jyes	mem_dword
		check	@dest.size = 8
		jyes	mem_qword
		check	@dest.size
		jno	unknown_size
		err	'invalid operand size'
		jump	mem_dword
	    unknown_size:
		err	'operand size not specified'
	    mem_dword:
		xcall	x86.store_instruction@dest, (0D8h),(postbyte)
		exit
	    mem_qword:
		xcall	x86.store_instruction@dest, (0DCh),(postbyte)
		exit
	    st:
		call	x87.parse_operand@dest, dest
		call	x87.parse_operand@src, src
		check	@dest.type = 'streg' & @src.type = 'streg'
		jno	invalid_combination_of_operands
		check	@dest.rm = 0
		jyes	st0_sti
		check	@src.rm = 0
		jyes	sti_st0
		jump	invalid_combination_of_operands
	    st0_sti:
		emit	1, 0D8h
		emit	1, 11b shl 6 + postbyte shl 3 + @src.rm
		exit
	    sti_st0:
		check	postbyte >= 4
		jyes	switched
		emit	1, 0DCh
		emit	1, 11b shl 6 + postbyte shl 3 + @dest.rm
		exit
	    switched:
		emit	1, 0DCh
		emit	1, 11b shl 6 + (postbyte xor 1) shl 3 + @dest.rm
		exit
	    invalid_combination_of_operands:
		err	'invalid combination of operands'
	end calminstruction

end iterate

iterate <instr,postbyte>, faddp,0, fmulp,1, fsubrp,4, fsubp,5, fdivrp,6, fdivp,7

	calminstruction instr? operand&
		call	x86.require.8087
		local	dest, src
		match	, operand
		jyes	default
		match	dest=,src, operand
		jno	invalid_combination_of_operands
		call	x87.parse_operand@dest, dest
		call	x87.parse_operand@src, src
		check	@dest.type = 'streg' & @src.type = 'streg' & @src.rm = 0
		jyes	ok
	    invalid_combination_of_operands:
		err	'invalid combination of operands'
		exit
	    default:
		compute @dest.rm, 1
	    ok:
		emit	1, 0DEh
		emit	1, 11b shl 6 + postbyte shl 3 + @dest.rm
	end calminstruction

end iterate

iterate <instr,postbyte>, fcom,2, fcomp,3

	calminstruction instr? src:st1
		call	x86.require.8087
		call	x87.parse_operand@src, src
		check	@src.type = 'streg'
		jyes	st
		check	@src.type = 'mem'
		jno	invalid_operand
		check	@src.size = 4
		jyes	mem_dword
		check	@src.size = 8
		jyes	mem_qword
		check	@src.size
		jno	unknown_size
		err	'invalid operand size'
		jump	mem_dword
	    unknown_size:
		err	'operand size not specified'
	    mem_dword: st:
		xcall	x86.store_instruction@src, (0D8h),(postbyte)
		exit
	    mem_qword:
		xcall	x86.store_instruction@src, (0DCh),(postbyte)
		exit
	    invalid_operand:
		err	'invalid operand'
	end calminstruction

end iterate

iterate <instr,postbyte>, fiadd,0, fimul,1, ficom,2, ficomp,3, fisub,4, fisubr,5, fidiv,6, fidivr,7

	calminstruction instr? src*
		call	x86.require.8087
		call	x87.parse_operand@src, src
		check	@src.type = 'mem'
		jno	invalid_operand
		check	@src.size = 2
		jyes	mem_word
		check	@src.size = 4
		jyes	mem_dword
		check	@src.size
		jno	unknown_size
		err	'invalid operand size'
		jump	mem_word
	    unknown_size:
		err	'operand size not specified'
	    mem_word:
		xcall	x86.store_instruction@src, (0DEh),(postbyte)
		exit
	    mem_dword:
		xcall	x86.store_instruction@src, (0DAh),(postbyte)
		exit
	    invalid_operand:
		err	'invalid operand'
	end calminstruction

end iterate

calminstruction fld? src*
	call	x86.require.8087
	call	x87.parse_operand@src, src
	check	@src.type = 'streg'
	jyes	st
	check	@src.type = 'mem'
	jno	invalid_operand
	check	@src.size = 4
	jyes	mem_dword
	check	@src.size = 8
	jyes	mem_qword
	check	@src.size = 10
	jyes	mem_tword
	check	@src.size
	jno	unknown_size
	err	'invalid operand size'
	jump	mem_dword
    unknown_size:
	err	'operand size not specified'
    mem_dword: st:
	xcall	x86.store_instruction@src, (0D9h),(0)
	exit
    mem_qword:
	xcall	x86.store_instruction@src, (0DDh),(0)
	exit
    mem_tword:
	xcall	x86.store_instruction@src, (0DBh),(5)
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

calminstruction fst? dest*
	call	x86.require.8087
	call	x87.parse_operand@dest, dest
	check	@dest.type = 'streg'
	jyes	st
	check	@dest.type = 'mem'
	jno	invalid_operand
	check	@dest.size = 4
	jyes	mem_dword
	check	@dest.size = 8
	jyes	mem_qword
	check	@dest.size
	jno	unknown_size
	err	'invalid operand size'
	jump	mem_dword
    unknown_size:
	err	'operand size not specified'
    mem_dword:
	xcall	x86.store_instruction@dest, (0D9h),(2)
	exit
    mem_qword: st:
	xcall	x86.store_instruction@dest, (0DDh),(2)
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

calminstruction fstp? dest*
	call	x86.require.8087
	call	x87.parse_operand@dest, dest
	check	@dest.type = 'streg'
	jyes	st
	check	@dest.type = 'mem'
	jno	invalid_operand
	check	@dest.size = 4
	jyes	mem_dword
	check	@dest.size = 8
	jyes	mem_qword
	check	@dest.size = 10
	jyes	mem_tword
	check	@dest.size
	jno	unknown_size
	err	'invalid operand size'
	jump	mem_dword
    unknown_size:
	err	'operand size not specified'
    mem_dword:
	xcall	x86.store_instruction@dest, (0D9h),(3)
	exit
    mem_qword: st:
	xcall	x86.store_instruction@dest, (0DDh),(3)
	exit
    mem_tword:
	xcall	x86.store_instruction@dest, (0DBh),(7)
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

calminstruction fild? src*
	call	x86.require.8087
	call	x87.parse_operand@src, src
	check	@src.type = 'mem'
	jno	invalid_operand
	check	@src.size = 2
	jyes	mem_word
	check	@src.size = 4
	jyes	mem_dword
	check	@src.size = 8
	jyes	mem_qword
	check	@src.size
	jno	unknown_size
	err	'invalid operand size'
	jump	mem_word
    unknown_size:
	err	'operand size not specified'
    mem_word:
	xcall	x86.store_instruction@src, (0DFh),(0)
	exit
    mem_dword:
	xcall	x86.store_instruction@src, (0DBh),(0)
	exit
    mem_qword:
	xcall	x86.store_instruction@src, (0DFh),(5)
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

calminstruction fist? dest*
	call	x86.require.8087
	call	x87.parse_operand@dest, dest
	check	@dest.type = 'mem'
	jno	invalid_operand
	check	@dest.size = 2
	jyes	mem_word
	check	@dest.size = 4
	jyes	mem_dword
	check	@dest.size
	jno	unknown_size
	err	'invalid operand size'
	jump	mem_word
    unknown_size:
	err	'operand size not specified'
    mem_word:
	xcall	x86.store_instruction@dest, (0DFh),(2)
	exit
    mem_dword:
	xcall	x86.store_instruction@dest, (0DBh),(2)
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

calminstruction fistp? dest*
	call	x86.require.8087
	call	x87.parse_operand@dest, dest
	check	@dest.type = 'mem'
	jno	invalid_operand
	check	@dest.size = 2
	jyes	mem_word
	check	@dest.size = 4
	jyes	mem_dword
	check	@dest.size = 8
	jyes	mem_qword
	check	@dest.size
	jno	unknown_size
	err	'invalid operand size'
	jump	mem_word
    unknown_size:
	err	'operand size not specified'
    mem_word:
	xcall	x86.store_instruction@dest, (0DFh),(3)
	exit
    mem_dword:
	xcall	x86.store_instruction@dest, (0DBh),(3)
	exit
    mem_qword:
	xcall	x86.store_instruction@dest, (0DFh),(7)
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

calminstruction fisttp? dest*
	call	x86.require.8087
	call	x87.parse_operand@dest, dest
	check	@dest.type = 'mem'
	jno	invalid_operand
	check	@dest.size = 2
	jyes	mem_word
	check	@dest.size = 4
	jyes	mem_dword
	check	@dest.size = 8
	jyes	mem_qword
	check	@dest.size
	jno	unknown_size
	err	'invalid operand size'
	jump	mem_word
    unknown_size:
	err	'operand size not specified'
    mem_word:
	xcall	x86.store_instruction@dest, (0DFh),(1)
	exit
    mem_dword:
	xcall	x86.store_instruction@dest, (0DBh),(1)
	exit
    mem_qword:
	xcall	x86.store_instruction@dest, (0DDh),(1)
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

iterate <instr,postbyte>, fbld,4, fbstp,6

	calminstruction instr? src*
		call	x86.require.8087
		call	x87.parse_operand@src, src
		check	@src.type = 'mem'
		jno	invalid_operand
		check	@src.size and not 10
		jyes	invalid_operand_size
		xcall	x86.store_instruction@src, (0DFh),(postbyte)
		exit
	    invalid_operand_size:
		err	'invalid operand size'
		exit
	    invalid_operand:
		err	'invalid operand'
	end calminstruction

end iterate

calminstruction fxch? src:st1
	call	x86.require.8087
	call	x87.parse_operand@src, src
	check	@src.type = 'streg'
	jno	invalid_operand
	emit	1, 0D9h
	emit	1, 11b shl 6 + 1 shl 3 + @src.rm
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

iterate <instr,basecode>, ffree,0DDh, ffreep,0DFh

	calminstruction instr? src*
		call	x86.require.8087
		call	x87.parse_operand@src, src
		check	@src.type = 'streg'
		jno	invalid_operand
		emit	1, basecode
		emit	1, 11b shl 6 + @src.rm
		exit
	    invalid_operand:
		err	'invalid operand'
	end calminstruction

end iterate

calminstruction fnstsw? dest*
	call	x86.require.8087
	call	x86.parse_operand@dest, dest
	check	@dest.size and not 2
	jyes	invalid_operand_size
	check	@dest.type = 'mem'
	jyes	mem
	check	@dest.type = 'reg' & @dest.rm = 0
	jno	invalid_operand
	emit	1, 0DFh
	emit	1, 0E0h
	exit
    mem:
	xcall	x86.store_instruction@dest, (0DDh),(7)
	exit
    invalid_operand_size:
	err	'invalid operand size'
	exit
    invalid_operand:
	err	'invalid operand'
end calminstruction

iterate <instr,postbyte>, fldcw,5, fnstcw,7

	calminstruction instr? dest*
		call	x86.parse_operand@dest, dest
		check	@dest.size and not 2
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

iterate <instr,postbyte>, fldenv,4, fnstenv,6

	calminstruction instr? dest*
		call	x86.require.8087
		call	x86.parse_operand@dest, dest
		check	@dest.size and not 14
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

iterate <instr,postbyte>, frstor,4, fnsave,6

	calminstruction instr? dest*
		call	x86.require.8087
		call	x86.parse_operand@dest, dest
		check	@dest.size and not 94
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

iterate op,  stsw, stcw, stenv, save
	calminstruction f#op? dest*
		asm	fwait?
		asm	fn#op? dest
	end calminstruction
end iterate

iterate <instr,byte1,byte2>, fprem1,0D9h,0F5h, fsincos,0D9h,0FBh, fsin,0D9h,0FEh, fcos,0D9h,0FFh, fucompp,0DAh,0E9h

	calminstruction instr?
		call	x86.require.80387
		emit	1, byte1
		emit	1, byte2
	end calminstruction

end iterate

iterate <instr,postbyte>, fucom,4, fucomp,5

	calminstruction instr? src:st1
		call	x86.require.80387
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
		call	x86.require.80387
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
		call	x86.require.80387
		call	x86.parse_operand@dest, dest
		check	@dest.size and not 14
		jyes	invalid_operand_size
		check	@dest.type = 'mem'
		jno	invalid_operand
		xcall	x86.store_operand_prefix, (2)
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
		call	x86.require.80387
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
		call	x86.require.80387
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
		call	x86.require.80387
		call	x86.parse_operand@dest, dest
		check	@dest.size and not 94
		jyes	invalid_operand_size
		check	@dest.type = 'mem'
		jno	invalid_operand
		xcall	x86.store_operand_prefix, (2)
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
		call	x86.require.80387
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
		asm	fwait?
		asm	fn#op? dest
	end calminstruction
end iterate
