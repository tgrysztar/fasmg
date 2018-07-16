
if ~ defined i80387

	restore i80387	; this ensures that symbol cannot be forward-referenced
	define i80387 1

	include '80287.inc'

	purge fsetpm?

	macro fprem1?
		db 0D9h,0F5h
	end macro

	macro fsincos?
		db 0D9h,0FBh
	end macro

	macro fsin?
		db 0D9h,0FEh
	end macro

	macro fcos?
		db 0D9h,0FFh
	end macro

	iterate <instr,postbyte>, fucom,4, fucomp,5
		macro instr? src
			match , src
				db 0DDh, 11b shl 6 + postbyte shl 3 + 1
			else
				x87.parse_operand @src,src
				if @src.type = 'streg'
					db 0DDh, 11b shl 6 + postbyte shl 3 + @src.rm
				else
					err 'invalid operand'
				end if
			end match
		end macro
	end iterate

	macro fucompp?
		db 0DAh,0E9h
	end macro

	iterate <instr,postbyte>, fldenv,4, fnstenv,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & ( ( x86.mode = 16 & @dest.size <> 14 ) | ( x86.mode = 32 & @dest.size <> 28 ) )
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_instruction 0D9h,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	iterate <instr,postbyte>, fldenvw,4, fnstenvw,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & @dest.size <> 14
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_operand_prefix 2
				x86.store_instruction 0D9h,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fstenvw dest*
		fwait
		fnstenvw dest
	end macro

	iterate <instr,postbyte>, fldenvd,4, fnstenvd,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & @dest.size <> 28
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_operand_prefix 4
				x86.store_instruction 0D9h,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fstenvd dest*
		fwait
		fnstenvd dest
	end macro

	iterate <instr,postbyte>, frstor,4, fnsave,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & ( ( x86.mode = 16 & @dest.size <> 94 ) | ( x86.mode = 32 & @dest.size <> 108 ) )
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_operand_prefix 2
				x86.store_instruction 0DDh,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	iterate <instr,postbyte>, frstorw,4, fnsavew,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & @dest.size <> 94
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_operand_prefix 2
				x86.store_instruction 0DDh,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fsavew dest*
		fwait
		fnsavew dest
	end macro

	iterate <instr,postbyte>, frstord,4, fnsaved,6
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size & @dest.size <> 108
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				x86.store_operand_prefix 4
				x86.store_instruction 0DDh,@dest,postbyte
			else
				err 'invalid operand'
			end if
		end macro
	end iterate

	macro fsaved dest*
		fwait
		fnsaved dest
	end macro

end if