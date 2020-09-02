
include '../../examples/x86/include/80386.inc'

macro format?.ELF? variant
	match , variant
		format binary as 'o'
		include '../../examples/x86/include/format/elf32.inc'
		use32
	else match =executable? settings, variant:
		match brand =at? base:, settings
			ELF.Settings.ABI = brand
			ELF.Settings.BaseAddress = base
		else match =at? base:, settings
			ELF.Settings.BaseAddress = base
		else match brand:, settings
			ELF.Settings.ABI = brand
		end match
		include '../../examples/x86/include/format/elfexe.inc'
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
	if size
		sub	esp,size
	end if
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
