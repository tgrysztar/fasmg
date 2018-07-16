
if ~ defined MMX

	restore MMX	; this ensures that symbol cannot be forward-referenced
	define MMX 1

	element MMX.reg

	repeat 8, i:0
		element mm#i? : MMX.reg + i
	end repeat

	macro MMX.parse_operand ns,op
		x86.parse_operand ns,op
		if ns.type = 'imm' & ns.size = 0 & ns.imm eq 1 elementof ns.imm & 1 metadataof ns.imm relativeto MMX.reg
			ns.type = 'mmreg'
			ns.mod = 11b
			ns.rm = 1 metadataof ns.imm - MMX.reg
			ns.size = 8
		end if
	end macro

	macro MMX.basic_instruction ext,dest,src
		MMX.parse_operand @dest,dest
		MMX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size and not 8
				err 'invalid operand size'
			end if
			x86.store_instruction <0Fh,ext>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro punpcklbw? dest*,src*
		MMX.basic_instruction 60h,dest,src
	end macro

	macro punpcklwd? dest*,src*
		MMX.basic_instruction 61h,dest,src
	end macro

	macro punpckldq? dest*,src*
		MMX.basic_instruction 62h,dest,src
	end macro

	macro packsswb? dest*,src*
		MMX.basic_instruction 63h,dest,src
	end macro

	macro pcmpgtb? dest*,src*
		MMX.basic_instruction 64h,dest,src
	end macro

	macro pcmpgtw? dest*,src*
		MMX.basic_instruction 65h,dest,src
	end macro

	macro pcmpgtd? dest*,src*
		MMX.basic_instruction 66h,dest,src
	end macro

	macro packuswb? dest*,src*
		MMX.basic_instruction 67h,dest,src
	end macro

	macro punpckhbw? dest*,src*
		MMX.basic_instruction 68h,dest,src
	end macro

	macro punpckhwd? dest*,src*
		MMX.basic_instruction 69h,dest,src
	end macro

	macro punpckhdq? dest*,src*
		MMX.basic_instruction 6Ah,dest,src
	end macro

	macro packssdw? dest*,src*
		MMX.basic_instruction 6Bh,dest,src
	end macro

	macro pcmpeqb? dest*,src*
		MMX.basic_instruction 74h,dest,src
	end macro

	macro pcmpeqw? dest*,src*
		MMX.basic_instruction 75h,dest,src
	end macro

	macro pcmpeqd? dest*,src*
		MMX.basic_instruction 76h,dest,src
	end macro

	macro pmullw? dest*,src*
		MMX.basic_instruction 0D5h,dest,src
	end macro

	macro psubusb? dest*,src*
		MMX.basic_instruction 0D8h,dest,src
	end macro

	macro psubusw? dest*,src*
		MMX.basic_instruction 0D9h,dest,src
	end macro

	macro pand? dest*,src*
		MMX.basic_instruction 0DBh,dest,src
	end macro

	macro paddusb? dest*,src*
		MMX.basic_instruction 0DCh,dest,src
	end macro

	macro paddusw? dest*,src*
		MMX.basic_instruction 0DDh,dest,src
	end macro

	macro pandn? dest*,src*
		MMX.basic_instruction 0DFh,dest,src
	end macro

	macro pmulhw? dest*,src*
		MMX.basic_instruction 0E5h,dest,src
	end macro

	macro psubsb? dest*,src*
		MMX.basic_instruction 0E8h,dest,src
	end macro

	macro psubsw? dest*,src*
		MMX.basic_instruction 0E9h,dest,src
	end macro

	macro por? dest*,src*
		MMX.basic_instruction 0EBh,dest,src
	end macro

	macro paddsb? dest*,src*
		MMX.basic_instruction 0ECh,dest,src
	end macro

	macro paddsw? dest*,src*
		MMX.basic_instruction 0EDh,dest,src
	end macro

	macro pxor? dest*,src*
		MMX.basic_instruction 0EFh,dest,src
	end macro

	macro pmaddwd? dest*,src*
		MMX.basic_instruction 0F5h,dest,src
	end macro

	macro psubb? dest*,src*
		MMX.basic_instruction 0F8h,dest,src
	end macro

	macro psubw? dest*,src*
		MMX.basic_instruction 0F9h,dest,src
	end macro

	macro psubd? dest*,src*
		MMX.basic_instruction 0FAh,dest,src
	end macro

	macro paddb? dest*,src*
		MMX.basic_instruction 0FCh,dest,src
	end macro

	macro paddw? dest*,src*
		MMX.basic_instruction 0FDh,dest,src
	end macro

	macro paddd? dest*,src*
		MMX.basic_instruction 0FEh,dest,src
	end macro

	macro movq? dest*,src*
		MMX.parse_operand @dest,dest
		MMX.parse_operand @src,src
		if (@dest.size or @src.size) and not 8
			err 'invalid operand size'
		end if
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			x86.store_instruction <0Fh,6Fh>,@src,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'mmreg'
			x86.store_instruction <0Fh,7Fh>,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro movd? dest*,src*
		MMX.parse_operand @dest,dest
		MMX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'reg')
			if @src.size and not 4
				err 'invalid operand size'
			end if
			x86.store_instruction <0Fh,6Eh>,@src,@dest.rm
		else if (@dest.type = 'mem' | @dest.type = 'reg') & @src.type = 'mmreg'
			if @dest.size and not 4
				err 'invalid operand size'
			end if
			x86.store_instruction <0Fh,7Eh>,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro MMX.bit_shift_instruction ext,dest,src
		MMX.parse_operand @dest,dest
		MMX.parse_operand @src,src
		if @dest.type = 'mmreg' & (@src.type = 'mem' | @src.type = 'mmreg')
			if @src.size and not 8
				err 'invalid operand size'
			end if
			x86.store_instruction <0Fh,ext>,@src,@dest.rm
		else if @dest.type = 'mmreg' & @src.type = 'imm'
			if @src.size and not 1
				err 'invalid operand size'
			end if
			x86.store_instruction <0Fh,70h+(ext and 0Fh)>,@dest,((ext shr 4)-0Ch) shl 1,1,@src.imm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro psrlw? dest*,src*
		MMX.bit_shift_instruction 0D1h,dest,src
	end macro

	macro psrld? dest*,src*
		MMX.bit_shift_instruction 0D2h,dest,src
	end macro

	macro psrlq? dest*,src*
		MMX.bit_shift_instruction 0D3h,dest,src
	end macro

	macro psrad? dest*,src*
		MMX.bit_shift_instruction 0E2h,dest,src
	end macro

	macro psraw? dest*,src*
		MMX.bit_shift_instruction 0E1h,dest,src
	end macro

	macro psllw? dest*,src*
		MMX.bit_shift_instruction 0F1h,dest,src
	end macro

	macro pslld? dest*,src*
		MMX.bit_shift_instruction 0F2h,dest,src
	end macro

	macro psllq? dest*,src*
		MMX.bit_shift_instruction 0F3h,dest,src
	end macro

	macro emms?
		db 0Fh,77h
	end macro

end if
