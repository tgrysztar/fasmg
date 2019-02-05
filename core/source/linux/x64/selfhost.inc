
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

macro use32on64?

	macro x86.encode_address_32_64 ns
		if ns.address_registers eq esp & x86.mode = 64
			ns.displacement = ns.displacement shl 1
			x86.encode_address_32_64 ns
			ns.mode = 64
		else
			x86.encode_address_32_64 ns
		       ; the following allows to avoid 67h prefixes (leading to reduced code size and slightly improved performance),
		       ; but it requires that in entire affected code there are no cases when an offset held in register would be negative:
		       ; if ns.mode = 32 & x86.mode = 64
		       ;	 ns.mode = 64
		       ; end if
		end if
	end macro

	iterate <reg32,reg64>, eax,rax, ebx,rbx, ecx,rcx, edx,rdx, esi,rsi, edi,rdi, esp,rsp, ebp,rbp
		promote.reg32? equ :reg64
	end iterate

	iterate instr, jmp,call
		macro instr? arg
			match [mem], arg
				mov	r8d,[mem]
				instr	r8
			else match :reg64, promote.arg
				instr	reg64
			else
				instr	arg
			end match
		end macro
	end iterate

	macro push64?: arg
		match :reg64 tail, promote.arg:
			push	reg64
			match more:, tail
				push64 more
			end match
		else
			mov	r8d,arg
			push	r8
		end match
	end macro

	macro pop64?: arg
		match :reg64 tail, promote.arg:
			pop	reg64
			match more:, tail
				pop64 more
			end match
		else
			pop	r8
			mov	arg,r8d
		end match
	end macro

	macro push? arg
		push64	arg
	end macro

	macro pop? arg
		pop64	arg
	end macro

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
