
if ~ defined i8087

	restore i8087	; this ensures that symbol cannot be forward-referenced
	i8087 = 1

	define x87 x87

	element st?

	repeat 8, i:0
		element st#i? : st? + i
	end repeat

	define x86.qword? :8
	define x86.tword? :10
	define x86.tbyte? :10

	calminstruction x87.parse_operand namespace, operand

		local	size, type, mod, rm, imm
		local	i, sym, cmd

		match	=st?(i), operand
		jyes	indexed_streg_operand

		asmcmd	=x86.=parse_operand namespace, operand

		arrange type, namespace.=type
		arrange size, namespace.=size
		arrange imm, namespace.=imm

		check	type = 'imm' & size = 0
		jno	done
		check	imm eq 1 elementof imm & 1 metadataof imm relativeto st?
		jyes	streg_operand
		check	imm relativeto st? & imm = st?
		jno	done

		compute type, 'streg'
		compute mod, 11b
		compute rm, 0

		jump	export_register

	    streg_operand:

		compute type, 'streg'
		compute mod, 11b
		compute rm, 1 metadataof imm - st?

		jump	export_register

	    indexed_streg_operand:

		compute size, 0
		compute type, 'streg'
		compute mod, 11b
		compute rm, +i

		arrange sym, namespace.=size
		publish sym, size

	    export_register:

		arrange sym, namespace.=mod
		publish sym, mod

		arrange sym, namespace.=rm
		publish sym, rm

		arrange sym, namespace.=type
		publish sym, type

	    done:

	end calminstruction

	iterate <instr,opcode>, fwait,9Bh, wait,9Bh, fnop,<0D9h,0D0h>, \
				fchs,<0D9h,0E0h>, fabs,<0D9h,0E1h>, ftst,<0D9h,0E4h>, fxam,<0D9h,0E5h>, fld1,<0D9h,0E8h>, \
				fldl2t,<0D9h,0E9h>, fldl2e,<0D9h,0EAh>, fldpi,<0D9h,0EBh>, fldlg2,<0D9h,0ECh>, fldln2,<0D9h,0EDh>, fldz,<0D9h,0EEh>, \
				f2xm1,<0D9h,0F0h>, fyl2x,<0D9h,0F1h>, fptan,<0D9h,0F2h>, fpatan,<0D9h,0F3h>, fxtract,<0D9h,0F4h>, fdecstp,<0D9h,0F6h>, fincstp,<0D9h,0F7h>, fprem,<0D9h,0F8h>, \
				fyl2xp1,<0D9h,0F9h>, fsqrt,<0D9h,0FAh>, frndint,<0D9h,0FCh>, fscale,<0D9h,0FDh>, \
				fneni,<0DBh,0E0h>, fndisi,<0DBh,0E1h>, fnclex,<0DBh,0E2h>, fninit,<0DBh,0E3h>, \
				fcompp,<0DEh,0D9h>

		define x87.instr? db opcode

		calminstruction instr?
			assemble x87.instr?
		end calminstruction

	end iterate

	iterate op,  eni, disi, clex, init
		calminstruction f#op?
			assemble x87.fwait?
			assemble x87.fn#op?
		end calminstruction
	end iterate

	iterate <instr,postbyte>, fadd,0, fmul,1, fsub,4, fsubr,5, fdiv,6, fdivr,7

		calminstruction instr? operand&
			local	dest, src
			match	dest=,src, operand
			jyes	st
			asmcmd	=x87.=parse_operand =@dest,operand
			check	@dest.type = 'mem'
			jno	invalid_combination_of_operands
			check	@dest.size = 4
			jyes	mem_dword
			check	@dest.size = 8
			jyes	mem_qword
			check	@dest.size
			jno	unknown_size
			asmcmd	=err 'invalid operand size'
			jump	mem_dword
		    unknown_size:
			asmcmd	=err 'operand size not specified'
		    mem_dword:
			asmcmd	=x86.=store_instruction 0D8h,=@dest,postbyte
			exit
		    mem_qword:
			asmcmd	=x86.=store_instruction 0DCh,=@dest,postbyte
			exit
		    st:
			local	modrm
			asmcmd	=x87.=parse_operand =@dest,dest
			asmcmd	=x87.=parse_operand =@src,src
			check	@dest.type = 'streg' & @src.type = 'streg'
			jno	invalid_combination_of_operands
			check	@dest.rm = 0
			jyes	st0_sti
			check	@src.rm = 0
			jyes	sti_st0
			jump	invalid_combination_of_operands
		    st0_sti:
			compute modrm, 11b shl 6 + postbyte shl 3 + @src.rm
			asmcmd	=db 0D8h, modrm
			exit
		    sti_st0:
			check	postbyte >= 4
			jyes	switched
			compute modrm, 11b shl 6 + postbyte shl 3 + @dest.rm
			asmcmd	=db 0DCh, modrm
			exit
		    switched:
			compute modrm, 11b shl 6 + (postbyte xor 1) shl 3 + @dest.rm
			asmcmd	=db 0DCh, modrm
			exit
		    invalid_combination_of_operands:
			asmcmd	=err 'invalid combination of operands'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, faddp,0, fmulp,1, fsubrp,4, fsubp,5, fdivrp,6, fdivp,7

		calminstruction instr? operand&
			local	dest, src, modrm
			match	, operand
			jyes	default
			match	dest=,src, operand
			jno	invalid_combination_of_operands
			asmcmd	=x87.=parse_operand =@dest,dest
			asmcmd	=x87.=parse_operand =@src,src
			check	@dest.type = 'streg' & @src.type = 'streg' & @src.rm = 0
			jyes	ok
		    invalid_combination_of_operands:
			asmcmd	=err 'invalid combination of operands'
			exit
		    default:
			compute @dest.rm, 1
		    ok:
			compute modrm, 11b shl 6 + postbyte shl 3 + @dest.rm
			asmcmd	=db 0DEh, modrm
		end calminstruction

	end iterate

	iterate <instr,postbyte>, fcom,2, fcomp,3

		calminstruction instr? src:st1
			asmcmd	=x87.=parse_operand =@src,src
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
			asmcmd	=err 'invalid operand size'
			jump	mem_dword
		    unknown_size:
			asmcmd	=err 'operand size not specified'
		    mem_dword: st:
			asmcmd	=x86.=store_instruction 0D8h,=@src,postbyte
			exit
		    mem_qword:
			asmcmd	=x86.=store_instruction 0DCh,=@src,postbyte
			exit
		    invalid_operand:
			asmcmd	=err 'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, fiadd,0, fimul,1, ficom,2, ficomp,3, fisub,4, fisubr,5, fidiv,6, fidivr,7

		calminstruction instr? src*
			asmcmd	=x87.=parse_operand =@src,src
			check	@src.type = 'mem'
			jno	invalid_operand
			check	@src.size = 2
			jyes	mem_word
			check	@src.size = 4
			jyes	mem_dword
			check	@src.size
			jno	unknown_size
			asmcmd	=err 'invalid operand size'
			jump	mem_word
		    unknown_size:
			asmcmd	=err 'operand size not specified'
		    mem_word:
			asmcmd	=x86.=store_instruction 0DEh,=@src,postbyte
			exit
		    mem_dword:
			asmcmd	=x86.=store_instruction 0DAh,=@src,postbyte
			exit
		    invalid_operand:
			asmcmd	=err 'invalid operand'
		end calminstruction

	end iterate

	calminstruction fld? src*
		asmcmd	=x87.=parse_operand =@src,src
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
		asmcmd	=err 'invalid operand size'
		jump	mem_dword
	    unknown_size:
		asmcmd	=err 'operand size not specified'
	    mem_dword: st:
		asmcmd	=x86.=store_instruction 0D9h,=@src,0
		exit
	    mem_qword:
		asmcmd	=x86.=store_instruction 0DDh,=@src,0
		exit
	    mem_tword:
		asmcmd	=x86.=store_instruction 0DBh,=@src,5
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	calminstruction fst? dest*
		asmcmd	=x87.=parse_operand =@dest,dest
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
		asmcmd	=err 'invalid operand size'
		jump	mem_dword
	    unknown_size:
		asmcmd	=err 'operand size not specified'
	    mem_dword:
		asmcmd	=x86.=store_instruction 0D9h,=@dest,2
		exit
	    mem_qword: st:
		asmcmd	=x86.=store_instruction 0DDh,=@dest,2
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	calminstruction fstp? dest*
		asmcmd	=x87.=parse_operand =@dest,dest
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
		asmcmd	=err 'invalid operand size'
		jump	mem_dword
	    unknown_size:
		asmcmd	=err 'operand size not specified'
	    mem_dword:
		asmcmd	=x86.=store_instruction 0D9h,=@dest,3
		exit
	    mem_qword: st:
		asmcmd	=x86.=store_instruction 0DDh,=@dest,3
		exit
	    mem_tword:
		asmcmd	=x86.=store_instruction 0DBh,=@dest,7
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	calminstruction fild? src*
		asmcmd	=x87.=parse_operand =@src,src
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
		asmcmd	=err 'invalid operand size'
		jump	mem_word
	    unknown_size:
		asmcmd	=err 'operand size not specified'
	    mem_word:
		asmcmd	=x86.=store_instruction 0DFh,=@src,0
		exit
	    mem_dword:
		asmcmd	=x86.=store_instruction 0DBh,=@src,0
		exit
	    mem_qword:
		asmcmd	=x86.=store_instruction 0DFh,=@src,5
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	calminstruction fist? dest*
		asmcmd	=x87.=parse_operand =@dest,dest
		check	@dest.type = 'mem'
		jno	invalid_operand
		check	@dest.size = 2
		jyes	mem_word
		check	@dest.size = 4
		jyes	mem_dword
		check	@dest.size
		jno	unknown_size
		asmcmd	=err 'invalid operand size'
		jump	mem_word
	    unknown_size:
		asmcmd	=err 'operand size not specified'
	    mem_word:
		asmcmd	=x86.=store_instruction 0DFh,=@dest,2
		exit
	    mem_dword:
		asmcmd	=x86.=store_instruction 0DBh,=@dest,2
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	calminstruction fistp? dest*
		asmcmd	=x87.=parse_operand =@dest,dest
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
		asmcmd	=err 'invalid operand size'
		jump	mem_word
	    unknown_size:
		asmcmd	=err 'operand size not specified'
	    mem_word:
		asmcmd	=x86.=store_instruction 0DFh,=@dest,3
		exit
	    mem_dword:
		asmcmd	=x86.=store_instruction 0DBh,=@dest,3
		exit
	    mem_qword:
		asmcmd	=x86.=store_instruction 0DFh,=@dest,7
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	calminstruction fisttp? dest*
		asmcmd	=x87.=parse_operand =@dest,dest
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
		asmcmd	=err 'invalid operand size'
		jump	mem_word
	    unknown_size:
		asmcmd	=err 'operand size not specified'
	    mem_word:
		asmcmd	=x86.=store_instruction 0DFh,=@dest,1
		exit
	    mem_dword:
		asmcmd	=x86.=store_instruction 0DBh,=@dest,1
		exit
	    mem_qword:
		asmcmd	=x86.=store_instruction 0DDh,=@dest,1
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	iterate <instr,postbyte>, fbld,4, fbstp,6

		calminstruction instr? src*
			asmcmd	=x87.=parse_operand =@src,src
			check	@src.type = 'mem'
			jno	invalid_operand
			check	@src.size and not 10
			jyes	invalid_operand_size
			asmcmd	=x86.=store_instruction 0DFh,=@src,postbyte
			exit
		    invalid_operand_size:
			asmcmd	=err 'invalid operand size'
			exit
		    invalid_operand:
			asmcmd	=err 'invalid operand'
		end calminstruction

	end iterate

	calminstruction fxch? src:st1
		asmcmd	=x87.=parse_operand =@src,src
		check	@src.type = 'streg'
		jno	invalid_operand
		local	modrm
		compute modrm, 11b shl 6 + 1 shl 3 + @src.rm
		asmcmd	=db 0D9h, modrm
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	iterate <instr,basecode>, ffree,0DDh, ffreep,0DFh

		calminstruction instr? src*
			asmcmd	=x87.=parse_operand =@src,src
			check	@src.type = 'streg'
			jno	invalid_operand
			local	modrm
			compute modrm, 11b shl 6 + @src.rm
			asmcmd	=db basecode, modrm
			exit
		    invalid_operand:
			asmcmd	=err 'invalid operand'
		end calminstruction

	end iterate

	define x87.fnstsw_ax? db 0DFh,0E0h

	calminstruction fnstsw? dest*
		asmcmd	=x86.=parse_operand =@dest,dest
		check	@dest.size and not 2
		jyes	invalid_operand_size
		check	@dest.type = 'mem'
		jyes	mem
		check	@dest.type = 'reg' & @dest.rm = 0
		jno	invalid_operand
		assemble x87.fnstsw_ax
		exit
	    mem:
		asmcmd	=x86.=store_instruction 0DDh,=@dest,7
		exit
	    invalid_operand_size:
		asmcmd	=err 'invalid operand size'
		exit
	    invalid_operand:
		asmcmd	=err 'invalid operand'
	end calminstruction

	iterate <instr,postbyte>, fldcw,5, fnstcw,7

		calminstruction instr? dest*
			asmcmd	=x86.=parse_operand =@dest,dest
			check	@dest.size and not 2
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			asmcmd	=x86.=store_instruction 0D9h,=@dest,postbyte
			exit
		    invalid_operand_size:
			asmcmd	=err 'invalid operand size'
			exit
		    invalid_operand:
			asmcmd	=err 'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, fldenv,4, fnstenv,6

		calminstruction instr? dest*
			asmcmd	=x86.=parse_operand =@dest,dest
			check	@dest.size and not 14
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			asmcmd	=x86.=store_instruction 0D9h,=@dest,postbyte
			exit
		    invalid_operand_size:
			asmcmd	=err 'invalid operand size'
			exit
		    invalid_operand:
			asmcmd	=err 'invalid operand'
		end calminstruction

	end iterate

	iterate <instr,postbyte>, frstor,4, fnsave,6

		calminstruction instr? dest*
			asmcmd	=x86.=parse_operand =@dest,dest
			check	@dest.size and not 94
			jyes	invalid_operand_size
			check	@dest.type = 'mem'
			jno	invalid_operand
			asmcmd	=x86.=store_instruction 0DDh,=@dest,postbyte
			exit
		    invalid_operand_size:
			asmcmd	=err 'invalid operand size'
			exit
		    invalid_operand:
			asmcmd	=err 'invalid operand'
		end calminstruction

	end iterate

	iterate op,  stsw, stcw, stenv, save
		calminstruction f#op? dest*
			assemble x87.fwait?
			asmcmd	=fn#op? dest
		end calminstruction
	end iterate

end if