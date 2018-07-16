
if ~ defined HLE

	restore HLE   ; this ensures that symbol cannot be forward-referenced
	define HLE 1

	macro xacquire? instr&
		db 0F2h
		instr
	end macro

	macro xrelease? instr&
		db 0F3h
		instr
	end macro

	macro xtest?
		db 0Fh,1,0D6h
	end macro

end if
