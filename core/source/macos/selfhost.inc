
include '../../examples/x86/include/80386.inc'

iterate instr, push,pop
	macro instr? op
		local sequence
		sequence equ op --
		while 1
			match --, sequence
				break
			else match car= cdr, sequence
				redefine sequence cdr
				match :sz, x86.car
					match --, sequence
						instr car
						break
					else match head= tail, sequence
						redefine sequence tail
						instr car head
					end match
				else
					instr car
				end match
			end match
		end while
	end macro
end iterate

macro struct? name
	macro ends?!
			end namespace
		end struc
		virtual at 0
			name name
			sizeof.name = $
		end virtual
		purge ends?
	end macro
	struc name
		label . : sizeof.name
		namespace .
end macro

macro format? clause
	match =MachO? =executable?, clause
		MachO.Settings.BaseAddress = 0x1000
		include 'macho.inc'
		use32
	else match =MachO?, clause
		MachO.Settings.FileType equ MH_OBJECT
		include 'macho.inc'
		use32
	else
		err 'invalid argument'
	end match
end macro

macro ccall? proc*,args&
	local size
	mov	ebp,esp
	sub	esp,size
	and	esp,0FFFFFFF0h
	match any, args
		iterate arg, args
			mov	dword [esp+(%-1)*4],arg
			if % = 1
				size := %%*4
			end if
		end iterate
	else
		size := 0
	end match
	call	proc
	mov	esp,ebp
end macro
