define Listing

namespace Listing
	offset = $%%
	virtual at 0
		HexDigits:: db '0123456789ABCDEF'
	end virtual
	virtual as 'lst'
		Text::
	end virtual
end namespace

macro ? line&
	line
	namespace Listing
		undefined_bytes = $% - $%%
		defined_bytes = $%% - offset
		if $ - undefined_bytes - defined_bytes < $$
			defined_bytes = $ - undefined_bytes - $$
		end if
		offset = $%%
		virtual Text
			if defined_bytes
				repeat 8
					load digit:byte from HexDigits:((offset-defined_bytes) shr ((%%-%) shl 2)) and 0Fh
					db digit
				end repeat
				db ': '
			else
				db '          '
			end if
			end virtual
		column = 0
		source = `line
		while defined_bytes
			load data:byte from : $% - undefined_bytes - defined_bytes
			virtual Text
				if column = 8
					column = 0
					db ' ',source,13,10,'          '
					source = ''
				end if
				load digit:byte from HexDigits:data shr 4
				db digit
				load digit:byte from HexDigits:data and 0Fh
				db digit,' '
			end virtual
			defined_bytes = defined_bytes - 1
			column = column + 1
		end while
		virtual Text
			repeat 8-column
				db '   '
			end repeat
			db ' ',source,13,10
		end virtual
	end namespace
end macro