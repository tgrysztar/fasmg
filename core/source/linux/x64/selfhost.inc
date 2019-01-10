
include '../../../examples/x86/include/x64.inc'

macro format?.ELF64? variant
	match , variant
		format binary as 'o'
		include '../../../examples/x86/include/format/elf64.inc'
		use64
	else match =executable? settings, variant:
		ELF.Settings.Class = ELFCLASS64
		ELF.Settings.Machine = EM_X86_64
		ELF.Settings.BaseAddress = 400000h
		match brand =at? base:, settings
			ELF.Settings.ABI = brand
			ELF.Settings.BaseAddress = base
		else match =at? base:, settings
			ELF.Settings.BaseAddress = base
		else match brand:, settings
			ELF.Settings.ABI = brand
		end match
		include '../../../examples/x86/include/format/elfexe.inc'
		use64
	else
		err 'invalid argument'
	end match
end macro

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
