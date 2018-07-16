
macro element definition
	match name:meta, definition
		if ~ meta relativeto x86.trx
			element? name:meta
		end if
	else
		element? definition
	end match
end macro

include '80486.inc'
include '80387.inc'

purge element

macro cpuid?
	db 0Fh,0A2h
end macro

macro cmpxchg8b? dest*
	x86.parse_operand @dest,dest
	if @dest.size <> 0 & @dest.size <> 8
		err 'invalid operand size'
	end if
	if @dest.type = 'mem'
		x86.store_instruction <0Fh,0C7h>,@dest,1
	else
		err 'invalid operand'
	end if
end macro

macro wrmsr?
	db 0Fh,30h
end macro

macro rdtsc?
	db 0Fh,31h
end macro

macro rdmsr?
	db 0Fh,32h
end macro

macro rdpmc?
	db 0Fh,33h
end macro

macro rsm?
	db 0Fh,0AAh
end macro
