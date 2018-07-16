
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

macro format? clause
	match =PE? settings, clause:
		format binary as 'exe'
		PE.Settings.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE or IMAGE_FILE_32BIT_MACHINE
		PE.Settings.DllCharacteristics = 0
		local seq
		define seq settings
		while 1
			match :, seq
				break
			else match =GUI? V.v more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI
				PE.Settings.MajorSubsystemVersion = V
				PE.Settings.MinorSubsystemVersion = v
				redefine seq more
			else match =console? V.v more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI
				PE.Settings.MajorSubsystemVersion = V
				PE.Settings.MinorSubsystemVersion = v
				redefine seq more
			else match =native? V.v more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_NATIVE
				PE.Settings.MajorSubsystemVersion = V
				PE.Settings.MinorSubsystemVersion = v
				PE.Settings.SectionAlignment = 32
				PE.Settings.FileAlignment = 32
				PE.Settings.ComputeCheckSum = 1
				redefine seq more
			else match =GUI? more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI
				redefine seq more
			else match =console? more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI
				redefine seq more
			else match =native? more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_NATIVE
				PE.Settings.SectionAlignment = 32
				PE.Settings.FileAlignment = 32
				PE.Settings.ComputeCheckSum = 1
				redefine seq more
			else match =DLL? more, seq
				PE.Settings.Characteristics = PE.Settings.Characteristics or IMAGE_FILE_DLL
				redefine seq more
			else match =large? more, seq
				PE.Settings.Characteristics = PE.Settings.Characteristics or IMAGE_FILE_LARGE_ADDRESS_AWARE
				redefine seq more
			else match =WDM? more, seq
				PE.Settings.DllCharacteristics = PE.Settings.DllCharacteristics or IMAGE_DLL_CHARACTERISTICS_WDM_DRIVER
				redefine seq more
			else match =NX? more, seq
				PE.Settings.DllCharacteristics = PE.Settings.DllCharacteristics or IMAGE_DLL_CHARACTERISTICS_NX_COMPAT
				redefine seq more
			else match =at? base =on? stub :, seq
				PE.Settings.ImageBase = base
				PE.Settings.Stub = stub
				break
			else match =at? base :, seq
				PE.Settings.ImageBase = base
				break
			else match =on? stub :, seq
				PE.Settings.Stub = stub
				break
			else
				err 'invalid argument'
				break
			end match
		end while
		include '../../examples/x86/include/format/pe.inc'
		use32
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

macro invoke? proc*,args&
	match any, args
		iterate arg, args
			indx 1+%%-%
			pushd arg
		end iterate
	end match
	call [proc]
end macro

macro library? definitions&
	PE.Imports:
	iterate <name,string>, definitions
	      if ~ name.redundant
		      dd RVA name.lookup,0,0,RVA name.str,RVA name.address
	      end if
	      name.referred = 1
	end iterate
	dd 0,0,0,0,0
	iterate <name,string>, definitions
	      if ~ name.redundant
		      name.str db string,0
			       align 2
	      end if
	end iterate
end macro

macro import? name,definitions&
	align 4
	if defined name.referred
		name.lookup:
		iterate <label,string>, definitions
			if used label
				if string eqtype ''
					dd RVA name.label
				else
					dd 80000000h + string
				end if
			end if
		end iterate
		if $ > name.lookup
			name.redundant = 0
			dd 0
		else
			name.redundant = 1
		end if
		name.address:
		iterate <label,string>, definitions
			if used label
				if string eqtype ''
					label dd RVA name.label
				else
					label dd 80000000h + string
				end if
			end if
		end iterate
		if ~ name.redundant
			dd 0
		end if
		iterate <label,string>, definitions
			if used label & string eqtype ''
				name.label dw 0
					   db string,0
					   align 2
			end if
		end iterate
	end if
end macro

include 'kernel32.inc'
