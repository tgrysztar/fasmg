
if ~ defined SMX

	restore SMX   ; this ensures that symbol cannot be forward-referenced
	define SMX 1

	macro getsec?
		db 0Fh,37h
	end macro

end if
