
Listing? = 1

namespace Listing
	base = $$  
	offset = $
	virtual at 0
		HexDigits:: db '0123456789ABCDEF'
	end virtual
	virtual as 'lst'
		Text::
	end virtual
end namespace

macro ? line&
	line
	rmatch =nolist?, line
		Listing? =: 0
	else rmatch =list?, line
		restore Listing?
	else
		if Listing
			namespace Listing  
				if ~ $$ eq base  
					base = $$  
					offset = $$  
				end if	
				bytes = $ - offset  
				if $ - bytes < $$  
					bytes = $ - $$	
				end if	
				offset = $
				address = (offset scale 0)-bytes
				virtual Text
				repeat 8
					load digit:byte from HexDigits:((address) shr ((%%-%) shl 2)) and 0Fh
					db digit
				end repeat
				db ': '
				end virtual
				if bytes < 0
					bytes = 0
				end if
				while bytes > 0
					if bytes > 8
						bytes_in_row = 8
					else
						bytes_in_row = bytes
					end if
					if $ - bytes + bytes_in_row <= $@
						readable_bytes = bytes_in_row
					else if $ - bytes >= $@
						readable_bytes = 0
					else
						readable_bytes = bytes_in_row - ($ - $@)
					end if
					load data:readable_bytes from $ - bytes
					virtual Text
					repeat readable_bytes
						load digit:byte from HexDigits:(data shr ((%-1) shl 3 + 4)) and 0Fh
						db digit
						load digit:byte from HexDigits:(data shr ((%-1) shl 3)) and 0Fh
						db digit,' '
					end repeat
					repeat bytes_in_row - readable_bytes
						db '?? '
					end repeat
					if bytes > bytes_in_row
						db 13,10,'          '
						end virtual
						bytes = bytes - bytes_in_row
					else
						end virtual
						break
					end if
				end while
				virtual Text
				repeat 8-bytes
					db '   '
				end repeat
				db `line,13,10
				end virtual
			end namespace  
		end if
	end rmatch
end macro

macro nolist?
end macro

macro list?
end macro

postpone
	nolist 
end postpone