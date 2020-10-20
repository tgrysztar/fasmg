
include '../../examples/x86/include/80386.inc'

macro format?.PE? settings
	PE.Settings.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE or IMAGE_FILE_32BIT_MACHINE or IMAGE_FILE_LINE_NUMS_STRIPPED or IMAGE_FILE_LOCAL_SYMS_STRIPPED
	PE.Settings.DllCharacteristics = 0
	PE.Settings.Stamp = +VERSION
	local seq
	define seq settings:
	while 1
		match :, seq
			break
		else match =DLL? more, seq
			PE.Settings.Characteristics = PE.Settings.Characteristics or IMAGE_FILE_DLL
			redefine seq more
		else match =large? more, seq
			PE.Settings.Characteristics = PE.Settings.Characteristics or IMAGE_FILE_LARGE_ADDRESS_AWARE
			redefine seq more
		else match =WDM? more, seq
			PE.Settings.DllCharacteristics = PE.Settings.DllCharacteristics or IMAGE_DLLCHARACTERISTICS_WDM_DRIVER
			redefine seq more
		else match =NX? more, seq
			PE.Settings.DllCharacteristics = PE.Settings.DllCharacteristics or IMAGE_DLLCHARACTERISTICS_NX_COMPAT
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
			match =GUI? more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI
				redefine seq more
			else match =console? more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI
				redefine seq more
			else match =native? more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_NATIVE
				PE.Settings.SectionAlignment = 32
				PE.Settings.FileAlignment = 32
				redefine seq more
			else match =EFI? more, seq
				PE.Settings.Magic = 0x20B
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_EFI_APPLICATION
				redefine seq more
			else match =EFIboot? more, seq
				PE.Settings.Magic = 0x20B
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER
				redefine seq more
			else match =EFIruntime? more, seq
				PE.Settings.Magic = 0x20B
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER
				redefine seq more
			else
				err 'invalid argument'
				break
			end match
			match V.v more, seq
				PE.Settings.MajorSubsystemVersion = V
				PE.Settings.MinorSubsystemVersion = v
				redefine seq more
			end match
		end match
	end while
	if PE.Settings.Characteristics and IMAGE_FILE_DLL
		format binary as 'dll'
	else
		format binary as 'exe'
	end if
	include '../../examples/x86/include/format/pe.inc'
	use32
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

calminstruction invoke? proc*,args&
	local	tmp, stack
	match	, args
	jyes	call
    collect:
	match	tmp=,args, args
	take	stack, tmp
	jyes	collect
    push:
	arrange args, =pushd args
	assemble args
	take	args, stack
	jyes	push
    call:
	arrange proc, =call [proc]
	assemble proc
end calminstruction

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
