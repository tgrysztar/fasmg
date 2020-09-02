
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

macro use32on64?

	define esp rsp

	define promote

	iterate <reg32,reg64>, eax,rax, ebx,rbx, ecx,rcx, edx,rdx, esi,rsi, edi,rdi, esp,rsp, ebp,rbp
		promote.reg32? equ reg64
	end iterate

	iterate instr, jmp,call
		calminstruction instr? arg
			local	tmp
			match	[tmp], arg
			jyes	zero_extend
			transform arg, promote
			arrange tmp, =instr arg
			assemble tmp
			exit
		    zero_extend:
			arrange tmp, =mov =r8d,[tmp]
			assemble tmp
			arrange tmp, =instr =r8
			assemble tmp
		end calminstruction
	end iterate

	calminstruction push? arg
		local	car, cdr
		match	car cdr?, arg
	    loop:
		transform car, promote
		jno	non_reg
		arrange car, =push car
		assemble car
		match	car cdr?, cdr
		jyes	loop
		exit
	    non_reg:
		arrange tmp, =mov =r8d,arg
		assemble tmp
		arrange tmp, =push =r8
		assemble tmp
		exit
	end calminstruction

	calminstruction pop? arg
		local	car, cdr
		match	car cdr?, arg
	    loop:
		transform car, promote
		jno	non_reg
		arrange car, =pop car
		assemble car
		match	car cdr?, cdr
		jyes	loop
		exit
	    non_reg:
		arrange tmp, =pop =r8
		assemble tmp
		arrange tmp, =mov arg,=r8d
		assemble tmp
		exit
	end calminstruction

	macro jecxz? target
		if target-($+1) < 80h & target-($+1) >= -80h
			jecxz	target
		else
			local	j,k
			jecxz	j
			jmp	k
		    j:	jmp	target
		    k:
		end if
	end macro

end macro
