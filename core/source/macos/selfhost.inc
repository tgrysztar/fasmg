
include '../../examples/x86/include/80386.inc'

macro format?.MachO? variant
	match , variant
		MachO.Settings.FileType equ MH_OBJECT
		include '../../examples/x86/include/format/macho.inc'
		use32
	else match =executable?, variant
		MachO.Settings.BaseAddress = 0x1000
		include '../../examples/x86/include/format/macho.inc'
		use32
	else
		err 'invalid argument'
	end match
end macro

macro struct? name
	macro ends?!
			end namespace
		esc end struc
		virtual at 0
			name name
			sizeof.name = $
		end virtual
		purge ends?
	end macro
	esc struc name
		label . : sizeof.name
		namespace .
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
