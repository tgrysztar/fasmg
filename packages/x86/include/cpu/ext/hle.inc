
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


