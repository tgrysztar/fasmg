
if ~ defined RTM

	restore RTM   ; this ensures that symbol cannot be forward-referenced
	define RTM 1

	macro xbegin? dest*
		x86.parse_jump_operand @dest,dest
		if @dest.type = 'imm' & ~ @dest.jump_type
			if x86.mode shr 3 <> @dest.size
				err 'invalid operand size'
			end if
			if x86.mode = 16
				db 0C7h,0F8h
				dw @dest.imm-($+2)
			else
				if @dest.unresolved | ( @dest.imm relativeto $ & @dest.imm-($+5) < 8000h & @dest.imm-($+5) >= -8000h )
					db 66h,0C7h,0F8h
					dw @dest.imm-($+2)
				else
					db 0C7h,0F8h
					dd @dest.imm-($+4)
				end if
			end if
		else
			err 'invalid operand'
		end if
	end macro

	macro xabort? imm*
		db 0C6h,0F8h,imm
	end macro

	macro xend?
		db 0Fh,1,0D5h
	end macro

	macro xtest?
		db 0Fh,1,0D6h
	end macro

end if
