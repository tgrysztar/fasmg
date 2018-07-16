
if ~ defined i8087

	restore i8087	; this ensures that symbol cannot be forward-referenced
	define i8087

	define x87 1

	element st?

	repeat 8, i:0
		element st#i? : st? + i
	end repeat

	define x86.qword? :8
	define x86.tword? :10
	define x86.tbyte? :10

	macro x87.parse_operand ns,op
		match =st?(i), op
			ns.size = 0
			ns.type = 'streg'
			ns.mod = 11b
			ns.rm = +i
		else
			x86.parse_operand ns,op
			if ns.type = 'imm' & ns.size = 0
				if ns.imm relativeto st? & ns.imm = st?
					ns.type = 'streg'
					ns.mod = 11b
					ns.rm = 0
				else if ns.imm eq 1 elementof ns.imm & 1 metadataof ns.imm relativeto st?
					ns.type = 'streg'
					ns.mod = 11b
					ns.rm = 1 metadataof ns.imm - st?
				end if
			end if
		end match
	end macro

	macro fwait?
		db 9Bh
	end macro

	macro wait?
		fwait
	end macro

	macro fnop?
		db 0D9h,0D0h
	end macro

	macro fchs?
		db 0D9h,0E0h
	end macro

	macro fabs?
		db 0D9h,0E1h
	end macro

	macro ftst?
		db 0D9h,0E4h
	end macro

	macro fxam?
		db 0D9h,0E5h
	end macro

	macro fld1?
		db 0D9h,0E8h
	end macro

	macro fldl2t?
		db 0D9h,0E9h
	end macro

	macro fldl2e?
		db 0D9h,0EAh
	end macro

	macro fldpi?
		db 0D9h,0EBh
	end macro

	macro fldlg2?
		db 0D9h,0ECh
	end macro

	macro fldln2?
		db 0D9h,0EDh
	end macro

	macro fldz?
		db 0D9h,0EEh
	end macro

	macro f2xm1?
		db 0D9h,0F0h
	end macro

	macro fyl2x?
		db 0D9h,0F1h
	end macro

	macro fptan?
		db 0D9h,0F2h
	end macro

	macro fpatan?
		db 0D9h,0F3h
	end macro

	macro fxtract?
		db 0D9h,0F4h
	end macro

	macro fdecstp?
		db 0D9h,0F6h
	end macro

	macro fincstp?
		db 0D9h,0F7h
	end macro

	macro fprem?
		db 0D9h,0F8h
	end macro

	macro fyl2xp1?
		db 0D9h,0F9h
	end macro

	macro fsqrt?
		db 0D9h,0FAh
	end macro

	macro frndint?
		db 0D9h,0FCh
	end macro

	macro fscale?
		db 0D9h,0FDh
	end macro

	macro fneni?
		db 0DBh,0E0h
	end macro

	macro feni?
		fwait
		fneni
	end macro

	macro fndisi?
		db 0DBh,0E1h
	end macro

	macro fdisi?
		fwait
		fndisi
	end macro

	macro fnclex?
		db 0DBh,0E2h
	end macro

	macro fclex?
		fwait
		fnclex
	end macro

	macro fninit?
		db 0DBh,0E3h
	end macro

	macro finit?
		fwait
		fninit
	end macro

	iterate <instr,postbyte>, fadd,0, fmul,1, fsub,4, fsubr,5, fdiv,6, fdivr,7
		macro instr? dest,src
			x87.parse_operand @dest,dest
			if @dest.type = 'mem'
				match , src
					if @dest.size = 4
						x86.store_instruction 0D8h,@dest,postbyte
					else if @dest.size = 8
						x86.store_instruction 0DCh,@dest,postbyte
					else if @dest.size
						err 'invalid operand size'
					else
						err 'operand size not specified'
					end if
				else
					err 'invalid combination of operands'
				end match
			else if @dest.type = 'streg'
				x87.parse_operand @src,src
				if @src.type = 'streg'
					if @dest.rm = 0
						db 0D8h, 11b shl 6 + postbyte shl 3 + @src.rm
					else if @src.rm = 0
						if postbyte < 4
							db 0DCh, 11b shl 6 + postbyte shl 3 + @dest.rm
						else
							db 0DCh, 11b shl 6 + (postbyte xor 1) shl 3 + @dest.rm
						end if
					else
						err 'invalid combination of operands'
					end if
				else
					err 'invalid combination of operands'
				end if
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	iterate <instr,postbyte>, faddp,0, fmulp,1, fsubrp,4, fsubp,5, fdivrp,6, fdivp,7
		macro instr? dest,src
			match , dest src
				db 0DEh, 11b shl 6 + postbyte shl 3 + 1
			else
				x87.parse_operand @dest,dest
				x87.parse_operand @src,src
				if @dest.type = 'streg' & @src.type = 'streg'
					if @src.rm = 0
						db 0DEh, 11b shl 6 + postbyte shl 3 + @dest.rm
					else
						err 'invalid combination of operands'
					end if
				else
					err 'invalid combination of operands'
				end if
			end match
		end macro
	end iterate

	iterate <instr,postbyte>, fcom,2, fcomp,3
		macro instr? src:st1
			x87.parse_operand @src,src
			if @src.type = 'mem'
				if @src.size = 4
					x86.store_instruction 0D8h,@src,postbyte
				else if @src.size = 8
					x86.store_instruction 0DCh,@src,postbyte
				else if @src.size
					err 'invalid operand size'
				else
					err 'operand size not specified'
				end if
			else if @src.type = 'streg'
				db 0D8h, 11b shl 6 + postbyte shl 3 + @src.rm
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fcompp?
		db 0DEh,0D9h
	end macro

	iterate <instr,postbyte>, fiadd,0, fimul,1, ficom,2, ficomp,3, fisub,4, fisubr,5, fidiv,6, fidivr,7
		macro instr? src*
			x87.parse_operand @src,src
			if @src.type = 'mem'
				if @src.size = 2
					x86.store_instruction 0DEh,@src,postbyte
				else if @src.size = 4
					x86.store_instruction 0DAh,@src,postbyte
				else if @src.size
					err 'invalid operand size'
				else
					err 'operand size not specified'
				end if
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fld? src*
		x87.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size = 4
				x86.store_instruction 0D9h,@src,0
			else if @src.size = 8
				x86.store_instruction 0DDh,@src,0
			else if @src.size = 10
				x86.store_instruction 0DBh,@src,5
			else if @src.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else if @src.type = 'streg'
			x86.store_instruction 0D9h,@src,0
		else
			err 'invalid operand'
		end if
	end macro

	macro fst? src*
		x87.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size = 4
				x86.store_instruction 0D9h,@src,2
			else if @src.size = 8
				x86.store_instruction 0DDh,@src,2
			else if @src.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else if @src.type = 'streg'
			x86.store_instruction 0DDh,@src,2
		else
			err 'invalid operand'
		end if
	end macro

	macro fstp? src*
		x87.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size = 4
				x86.store_instruction 0D9h,@src,3
			else if @src.size = 8
				x86.store_instruction 0DDh,@src,3
			else if @src.size = 10
				x86.store_instruction 0DBh,@src,7
			else if @src.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else if @src.type = 'streg'
			x86.store_instruction 0DDh,@src,3
		else
			err 'invalid operand'
		end if
	end macro

	macro fild? src*
		x87.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size = 2
				x86.store_instruction 0DFh,@src,0
			else if @src.size = 4
				x86.store_instruction 0DBh,@src,0
			else if @src.size = 8
				x86.store_instruction 0DFh,@src,5
			else if @src.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid operand'
		end if
	end macro

	macro fist? src*
		x87.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size = 2
				x86.store_instruction 0DFh,@src,2
			else if @src.size = 4
				x86.store_instruction 0DBh,@src,2
			else if @src.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid operand'
		end if
	end macro

	macro fistp? src*
		x87.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size = 2
				x86.store_instruction 0DFh,@src,3
			else if @src.size = 4
				x86.store_instruction 0DBh,@src,3
			else if @src.size = 8
				x86.store_instruction 0DFh,@src,7
			else if @src.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid operand'
		end if
	end macro

	macro fisttp? src*
		x87.parse_operand @src,src
		if @src.type = 'mem'
			if @src.size = 2
				x86.store_instruction 0DFh,@src,1
			else if @src.size = 4
				x86.store_instruction 0DBh,@src,1
			else if @src.size = 8
				x86.store_instruction 0DDh,@src,1
			else if @src.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid operand'
		end if
	end macro

	iterate <instr,postbyte>, fbld,4, fbstp,6
		macro instr? src*
			x87.parse_operand @src,src
			if @src.type = 'mem'
				if @src.size = 0 | @src.size = 10
					x86.store_instruction 0DFh,@src,postbyte
				else
					err 'invalid operand size'
				end if
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fxch? src:st1
		x87.parse_operand @src,src
		if @src.type = 'streg'
			db 0D9h, 11b shl 6 + 1 shl 3 + @src.rm
		else
			err 'invalid operand'
		end if
	end macro

	macro ffree? src*
		x87.parse_operand @src,src
		if @src.type = 'streg'
			db 0DDh, 11b shl 6 + @src.rm
		else
			err 'invalid operand'
		end if
	end macro

	macro ffreep? src*
		x87.parse_operand @src,src
		if @src.type = 'streg'
			db 0DFh, 11b shl 6 + @src.rm
		else
			err 'invalid operand'
		end if
	end macro

	macro fnstsw? dest*
		x86.parse_operand @dest,dest
		if @dest.size & @dest.size <> 2
			err 'invalid operand size'
		end if
		if @dest.type = 'reg' & @dest.rm = 0
			db 0DFh,0E0h
		else if @dest.type = 'mem'
			x86.store_instruction 0DDh,@dest,7
		else
			err 'invalid operand'
		end if
	end macro

	macro fstsw dest*
		fwait
		fnstsw dest
	end macro

	iterate <instr,postbyte>, fldcw,5, fnstcw,7
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & @dest.size <> 2
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_instruction 0D9h,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fstcw dest*
		fwait
		fnstcw dest
	end macro

	iterate <instr,postbyte>, fldenv,4, fnstenv,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & @dest.size <> 14
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_instruction 0D9h,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fstenv dest*
		fwait
		fnstenv dest
	end macro

	iterate <instr,postbyte>, frstor,4, fnsave,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & @dest.size <> 94
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_instruction 0DDh,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fsave dest*
		fwait
		fnsave dest
	end macro

end if