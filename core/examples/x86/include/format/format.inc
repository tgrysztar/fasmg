
macro format?.MZ?
	format binary as 'exe'
	include 'format/mz.inc'
	include 'p6.inc'
end macro

macro format?.PE? settings
	PE.Settings.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE or IMAGE_FILE_32BIT_MACHINE or IMAGE_FILE_LINE_NUMS_STRIPPED or IMAGE_FILE_LOCAL_SYMS_STRIPPED
	PE.Settings.DllCharacteristics = 0
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
	include 'format/pe.inc'
	include 'p6.inc'
	use32
end macro

macro format?.PE64? settings
	PE.Settings.Magic = 0x20B
	PE.Settings.Machine = IMAGE_FILE_MACHINE_AMD64
	PE.Settings.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE or IMAGE_FILE_LARGE_ADDRESS_AWARE or IMAGE_FILE_LINE_NUMS_STRIPPED or IMAGE_FILE_LOCAL_SYMS_STRIPPED
	PE.Settings.DllCharacteristics = 0
	PE.Settings.MajorSubsystemVersion = 5
	PE.Settings.MinorSubsystemVersion = 0
	local seq
	define seq settings:
	while 1
		match :, seq
			break
		else match =DLL? more, seq
			PE.Settings.Characteristics = PE.Settings.Characteristics or IMAGE_FILE_DLL
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
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_EFI_APPLICATION
				redefine seq more
			else match =EFIboot? more, seq
				PE.Settings.Subsystem = IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER
				redefine seq more
			else match =EFIruntime? more, seq
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
	include 'format/pe.inc'
	include 'x64.inc'
	use64
end macro

macro format?.ELF? variant
	match , variant
		format binary as 'o'
		include 'format/elf32.inc'
		include 'p6.inc'
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
		include 'format/elfexe.inc'
		include 'p6.inc'
		use32
	else match =dynamic? settings, variant:
		ELF.Settings.Type = ET_DYN
		match brand =at? base:, settings
			ELF.Settings.ABI = brand
			ELF.Settings.BaseAddress = base
		else match =at? base:, settings
			ELF.Settings.BaseAddress = base
		else match brand:, settings
			ELF.Settings.ABI = brand
		end match
		include 'format/elfexe.inc'
		include 'p6.inc'
		use32
	else
		err 'invalid argument'
	end match
end macro

macro format?.ELF64? variant
	match , variant
		format binary as 'o'
		include 'format/elf64.inc'
		include 'x64.inc'
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
		include 'format/elfexe.inc'
		include 'x64.inc'
		use64
	else match =dynamic? settings, variant:
		ELF.Settings.Class = ELFCLASS64
		ELF.Settings.Type = ET_DYN
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
		include 'format/elfexe.inc'
		include 'x64.inc'
		use64
	else
		err 'invalid argument'
	end match
end macro

macro format?.ELFx32? variant
	match , variant
		format binary as 'o'
		ELF.Settings.Machine = EM_X86_64
		include 'format/elf32.inc'
		include 'x64.inc'
		use64
	else match =executable? settings, variant:
		ELF.Settings.Class = ELFCLASS32
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
		include 'format/elfexe.inc'
		include 'x64.inc'
		use64
	else
		err 'invalid argument'
	end match
end macro

macro format?.COFF?
	format binary as 'obj'
	include 'format/coff.inc'
	include 'p6.inc'
	use32
end macro

macro format?.MS? variant
	match =COFF?, variant
		format binary as 'obj'
		COFF.Settings.Characteristics = IMAGE_FILE_32BIT_MACHINE or IMAGE_FILE_LINE_NUMS_STRIPPED or IMAGE_FILE_BYTES_REVERSED_LO
		include 'format/coffms.inc'
		include 'p6.inc'
		use32
	else
		err 'invalid argument'
	end match
end macro

macro format?.MS64? variant
	match =COFF?, variant
		format binary as 'obj'
		COFF.Settings.Machine = IMAGE_FILE_MACHINE_AMD64
		COFF.Settings.Characteristics = IMAGE_FILE_32BIT_MACHINE or IMAGE_FILE_LINE_NUMS_STRIPPED or IMAGE_FILE_BYTES_REVERSED_LO
		include 'format/coffms.inc'
		include 'x64.inc'
		use64
	else
		err 'invalid argument'
	end match
end macro

macro format?.MachO? variant
	match , variant
		format binary as 'o'
		MachO.Settings.FileType equ MH_OBJECT
		include 'format/macho.inc'
		include 'p6.inc'
		use32
	else match =executable?, variant
		MachO.Settings.BaseAddress = 0x1000
		include 'format/macho.inc'
		include 'p6.inc'
		use32
	else
		err 'invalid argument'
	end match
end macro

macro format?.MachO64? variant
	MachO.Settings.ProcessorType equ CPU_TYPE_X86_64
	MachO.Settings.ProcessorSubtype equ CPU_SUBTYPE_X86_64_ALL
	match , variant
		format binary as 'o'
		MachO.Settings.FileType equ MH_OBJECT
		include 'format/macho.inc'
		include 'x64.inc'
		use64
	else match =executable?, variant
		MachO.Settings.BaseAddress = 0x100000000
		include 'format/macho.inc'
		include 'x64.inc'
		use64
	else
		err 'invalid argument'
	end match
end macro

