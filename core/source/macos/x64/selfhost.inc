
; Adapted and tested by Jacob Young (jacobly.alt@gmail.com)

include '../../linux/x64/selfhost.inc'

macro format?.MachO64? variant
	match , variant
		MachO.Settings.ProcessorType = CPU_TYPE_X86_64
		MachO.Settings.FileType equ MH_OBJECT
		include '../../../examples/x86/include/format/macho.inc'
		use64
	else match =executable?, variant
		MachO.Settings.ProcessorType = CPU_TYPE_X86_64
		MachO.Settings.BaseAddress = 0x1000
		include '../../../examples/x86/include/format/macho.inc'
		use64
	else
		err 'invalid argument'
	end match
end macro

iterate reg, rdi, rsi, rdx, rcx, r8, r9
	arguments.%? equ reg
end iterate

macro ccall? proc*,args&
	local size
	mov	rbp,rsp
	and	rsp,0FFFFFFFFFFFFFFF0h
	match any, args
		iterate arg, args
			if sizeof (arg)
				lea	arguments.%,[arg]
			else
				mov	arguments.%,arg
			end if
		end iterate
	end match
	call	proc
	mov	rsp,rbp
end macro
