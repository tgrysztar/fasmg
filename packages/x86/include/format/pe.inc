
IMAGE_FILE_MACHINE_UNKNOWN   = 0x0
IMAGE_FILE_MACHINE_AM33      = 0x1D3
IMAGE_FILE_MACHINE_AMD64     = 0x8664
IMAGE_FILE_MACHINE_ARM	     = 0x1C0
IMAGE_FILE_MACHINE_ARMNT     = 0x1C4
IMAGE_FILE_MACHINE_ARM64     = 0xAA64
IMAGE_FILE_MACHINE_EBC	     = 0xEBC
IMAGE_FILE_MACHINE_I386      = 0x14C
IMAGE_FILE_MACHINE_IA64      = 0x200
IMAGE_FILE_MACHINE_M32R      = 0x9041
IMAGE_FILE_MACHINE_MIPS16    = 0x266
IMAGE_FILE_MACHINE_MIPSFPU   = 0x366
IMAGE_FILE_MACHINE_MIPSFPU16 = 0x466
IMAGE_FILE_MACHINE_POWERPC   = 0x1F0
IMAGE_FILE_MACHINE_POWERPCFP = 0x1F1
IMAGE_FILE_MACHINE_R4000     = 0x166
IMAGE_FILE_MACHINE_SH3	     = 0x1A2
IMAGE_FILE_MACHINE_SH3DSP    = 0x1A3
IMAGE_FILE_MACHINE_SH4	     = 0x1A6
IMAGE_FILE_MACHINE_SH5	     = 0x1A8
IMAGE_FILE_MACHINE_THUMB     = 0x1C2
IMAGE_FILE_MACHINE_WCEMIPSV2 = 0x169

IMAGE_FILE_RELOCS_STRIPPED	    = 0x0001
IMAGE_FILE_EXECUTABLE_IMAGE	    = 0x0002
IMAGE_FILE_LINE_NUMS_STRIPPED	    = 0x0004
IMAGE_FILE_LOCAL_SYMS_STRIPPED	    = 0x0008
IMAGE_FILE_AGGRESSIVE_WS_TRIM	    = 0x0010
IMAGE_FILE_LARGE_ADDRESS_AWARE	    = 0x0020
IMAGE_FILE_BYTES_REVERSED_LO	    = 0x0080
IMAGE_FILE_32BIT_MACHINE	    = 0x0100
IMAGE_FILE_DEBUG_STRIPPED	    = 0x0200
IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP  = 0x0400
IMAGE_FILE_NET_RUN_FROM_SWAP	    = 0x0800
IMAGE_FILE_SYSTEM		    = 0x1000
IMAGE_FILE_DLL			    = 0x2000
IMAGE_FILE_UP_SYSTEM_ONLY	    = 0x4000
IMAGE_FILE_BYTES_REVERSED_HI	    = 0x8000

IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA       = 0x0020
IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE	       = 0x0040
IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY       = 0x0080
IMAGE_DLLCHARACTERISTICS_NX_COMPAT	       = 0x0100
IMAGE_DLLCHARACTERISTICS_NO_ISOLATION	       = 0x0200
IMAGE_DLLCHARACTERISTICS_NO_SEH 	       = 0x0400
IMAGE_DLLCHARACTERISTICS_NO_BIND	       = 0x0800
IMAGE_DLLCHARACTERISTICS_APPCONTAINER	       = 0x1000
IMAGE_DLLCHARACTERISTICS_WDM_DRIVER	       = 0x2000
IMAGE_DLLCHARACTERISTICS_GUARD_CF	       = 0x4000
IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE = 0x8000

IMAGE_SUBSYSTEM_UNKNOWN 		= 0
IMAGE_SUBSYSTEM_NATIVE			= 1
IMAGE_SUBSYSTEM_WINDOWS_GUI		= 2
IMAGE_SUBSYSTEM_WINDOWS_CUI		= 3
IMAGE_SUBSYSTEM_POSIX_CUI		= 7
IMAGE_SUBSYSTEM_WINDOWS_CE_GUI		= 9
IMAGE_SUBSYSTEM_EFI_APPLICATION 	= 10
IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER = 11
IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER	= 12
IMAGE_SUBSYSTEM_EFI_ROM 		= 13
IMAGE_SUBSYSTEM_XBOX			= 14

IMAGE_SCN_TYPE_NO_PAD		 = 0x00000008
IMAGE_SCN_CNT_CODE		 = 0x00000020
IMAGE_SCN_CNT_INITIALIZED_DATA	 = 0x00000040
IMAGE_SCN_CNT_UNINITIALIZED_DATA = 0x00000080
IMAGE_SCN_LNK_OTHER		 = 0x00000100
IMAGE_SCN_LNK_INFO		 = 0x00000200
IMAGE_SCN_LNK_REMOVE		 = 0x00000800
IMAGE_SCN_LNK_COMDAT		 = 0x00001000
IMAGE_SCN_GPREL 		 = 0x00008000
IMAGE_SCN_MEM_PURGEABLE 	 = 0x00020000
IMAGE_SCN_MEM_16BIT		 = 0x00020000
IMAGE_SCN_MEM_LOCKED		 = 0x00040000
IMAGE_SCN_MEM_PRELOAD		 = 0x00080000
IMAGE_SCN_ALIGN_1BYTES		 = 0x00100000
IMAGE_SCN_ALIGN_2BYTES		 = 0x00200000
IMAGE_SCN_ALIGN_4BYTES		 = 0x00300000
IMAGE_SCN_ALIGN_8BYTES		 = 0x00400000
IMAGE_SCN_ALIGN_16BYTES 	 = 0x00500000
IMAGE_SCN_ALIGN_32BYTES 	 = 0x00600000
IMAGE_SCN_ALIGN_64BYTES 	 = 0x00700000
IMAGE_SCN_ALIGN_128BYTES	 = 0x00800000
IMAGE_SCN_ALIGN_256BYTES	 = 0x00900000
IMAGE_SCN_ALIGN_512BYTES	 = 0x00A00000
IMAGE_SCN_ALIGN_1024BYTES	 = 0x00B00000
IMAGE_SCN_ALIGN_2048BYTES	 = 0x00C00000
IMAGE_SCN_ALIGN_4096BYTES	 = 0x00D00000
IMAGE_SCN_ALIGN_8192BYTES	 = 0x00E00000
IMAGE_SCN_LNK_NRELOC_OVFL	 = 0x01000000
IMAGE_SCN_MEM_DISCARDABLE	 = 0x02000000
IMAGE_SCN_MEM_NOT_CACHED	 = 0x04000000
IMAGE_SCN_MEM_NOT_PAGED 	 = 0x08000000
IMAGE_SCN_MEM_SHARED		 = 0x10000000
IMAGE_SCN_MEM_EXECUTE		 = 0x20000000
IMAGE_SCN_MEM_READ		 = 0x40000000
IMAGE_SCN_MEM_WRITE		 = 0x80000000

IMAGE_REL_BASED_ABSOLUTE = 0
IMAGE_REL_BASED_HIGH	 = 1
IMAGE_REL_BASED_LOW	 = 2
IMAGE_REL_BASED_HIGHLOW  = 3
IMAGE_REL_BASED_HIGHADJ  = 4
IMAGE_REL_BASED_DIR64	 = 10

calminstruction align? boundary,value:?
	check	$ relativeto 0 | ( $ relativeto PE.RELOCATION & PE.SECTION_ALIGNMENT mod boundary = 0 )
	jyes	allowed
	arrange value, =err 'section not aligned enough'
	assemble value
	exit
    allowed:
	compute boundary, (boundary-1)-((0 scaleof $)+boundary-1) mod boundary
	arrange value, =db boundary =dup value
	assemble value
end calminstruction

PE::

namespace PE

	if defined Settings.Magic
		MAGIC = Settings.Magic
	else
		MAGIC = 0x10B
	end if

	if defined Settings.Machine
		MACHINE = Settings.Machine
	else
		MACHINE = IMAGE_FILE_MACHINE_I386
	end if

	if defined Settings.Characteristics
		CHARACTERISTICS = Settings.Characteristics
	else
		CHARACTERISTICS = IMAGE_FILE_EXECUTABLE_IMAGE or IMAGE_FILE_32BIT_MACHINE
	end if

	if defined Settings.DllCharacteristics
		DLL_CHARACTERISTICS = Settings.DllCharacteristics
	else
		DLL_CHARACTERISTICS = 0
	end if

	if defined Settings.Subsystem
		SUBSYSTEM = Settings.Subsystem
	else
		SUBSYSTEM = IMAGE_SUBSYSTEM_WINDOWS_CUI
	end if

	if defined Settings.MajorSubsystemVersion
		MAJOR_SUBSYSTEM_VERSION = Settings.MajorSubsystemVersion
	else
		MAJOR_SUBSYSTEM_VERSION = 3
	end if

	if defined Settings.MinorSubsystemVersion
		MINOR_SUBSYSTEM_VERSION = Settings.MinorSubsystemVersion
	else
		MINOR_SUBSYSTEM_VERSION = 10
	end if

	if defined Fixups
		element RELOCATION
		DLL_CHARACTERISTICS = DLL_CHARACTERISTICS or IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE
	else
		RELOCATION := 0
		CHARACTERISTICS = CHARACTERISTICS or IMAGE_FILE_RELOCS_STRIPPED
	end if

	if defined Settings.ImageBase
		IMAGE_BASE := RELOCATION + Settings.ImageBase
	else
		IMAGE_BASE := RELOCATION + 400000h
	end if

	if defined Settings.SectionAlignment
		SECTION_ALIGNMENT := Settings.SectionAlignment
	else
		SECTION_ALIGNMENT := 1000h
	end if

	if defined Settings.FileAlignment
		FILE_ALIGNMENT := Settings.FileAlignment
	else
		FILE_ALIGNMENT := 512
	end if

	if defined Settings.LegacyHeaders
		LEGACY_HEADERS := Settings.LegacyHeaders
	else
		LEGACY_HEADERS := 1
	end if

	NUMBER_OF_DIRECTORIES := 16

	if defined Settings.Stub

		virtual at 0
			file Settings.Stub
			if $ >= 1Ch
				load SIGNATURE : word from 0
				if SIGNATURE = "MZ" | SIGNATURE = "ZM"
					StubTemplate::
				end if
			end if
		end virtual

		if defined StubTemplate

			load .BYTES_IN_LAST_PAGE : word from StubTemplate:2
			load .NUMBER_OF_PAGES : word from StubTemplate:4
			.TEMPLATE_LENGTH = .NUMBER_OF_PAGES shl 9 - (-.BYTES_IN_LAST_PAGE) and 1FFh

			load .RELOCATIONS_OFFSET : word from StubTemplate:18h
			if .RELOCATIONS_OFFSET >= 40h
				file Settings.Stub,.TEMPLATE_LENGTH
			else
				load .NUMBER_OF_RELOCATIONS : word from StubTemplate:6
				.RELOCATIONS_LENGTH = .NUMBER_OF_RELOCATIONS shl 2
				load .NUMBER_OF_HEADER_PARAGRAPHS : word from StubTemplate:8
				.TEMPLATE_HEADER_LENGTH = .NUMBER_OF_HEADER_PARAGRAPHS shl 4

				file Settings.Stub,1Ch
				rb 40h - $
				file Settings.Stub:.RELOCATIONS_OFFSET,.RELOCATIONS_LENGTH
				align 16
				.HEADER_LENGTH = $
				file Settings.Stub:.TEMPLATE_HEADER_LENGTH,.TEMPLATE_LENGTH-.TEMPLATE_HEADER_LENGTH
				.LENGTH = $

				store 40h : word at 18h
				store .HEADER_LENGTH shr 4 : word at 8
				store .LENGTH and 1FFh : word at 2
				store (.LENGTH-1) shr 9 + 1 : word at 4
			end if

			store Header : dword at 3Ch

		else

			Stub:
			.signature		       dw "MZ"
			.bytes_in_last_page	       dw .LENGTH and 1FFh
			.number_of_pages	       dw (.LENGTH-1) shr 9 + 1
			.number_of_relocations	       dw 0
			.number_of_header_paragraphs   dw .HEADER_LENGTH shr 4
			.minimum_heap		       dw (10000h - (.LENGTH-.HEADER_LENGTH)) shr 4
			.maximum_heap		       dw 0FFFFh
			.initial_ss		       dw (-100h) shr 4
			.initial_sp		       dw 0FFFEh
			.checksum		       dw 0
			.initial_ip		       dw 100h
			.initial_cs		       dw (-100h) shr 4
			.relocations_offset	       dw 40h
			.overlay_number 	       dw 0
						       rb 3Ch - $
			.new_header_offset	       dd Header

			.HEADER_LENGTH = $

			file Settings.Stub

			.LENGTH = $

		end if

	else

		Stub:
		.signature		       dw "MZ"
		.bytes_in_last_page	       dw .LENGTH and 1FFh
		.number_of_pages	       dw (.LENGTH-1) shr 9 + 1
		.number_of_relocations	       dw 0
		.number_of_header_paragraphs   dw .HEADER_LENGTH shr 4
		.minimum_heap		       dw .STACK_LENGTH shr 4
		.maximum_heap		       dw 0FFFFh
		.initial_ss		       dw 0
		.initial_sp		       dw .LENGTH - .HEADER_LENGTH + .STACK_LENGTH
		.checksum		       dw 0
		.initial_ip		       dw 0
		.initial_cs		       dw 0
		.relocations_offset	       dw 40h
		.overlay_number 	       dw 0
					       rb 3Ch - $
		.new_header_offset	       dd Header

		.HEADER_LENGTH = $
		.STACK_LENGTH = 100h

		namespace Stub

		      include 'cpu/8086.inc'

		start:
		      push    cs
		      pop     ds
		      mov     dx,message - start
		      mov     ah,9
		      int     21h
		      mov     ax,4C01h
		      int     21h

		message db 'This program cannot be run in DOS mode.',0Dh,0Ah,24h

		end namespace

		align 16

		.LENGTH = $

	end if

	if defined Settings.Stamp
		TIMESTAMP := Settings.Stamp
	else
		TIMESTAMP := __TIME__
	end if

	align 8

	Header:
	.Signature		      dw "PE",0
	.Machine		      dw MACHINE
	.NumberOfSections	      dw NUMBER_OF_SECTIONS
	.TimeDateStamp		      dd TIMESTAMP
	.PointerToSymbolTable	      dd 0
	.NumberOfSymbols	      dd 0
	.SizeOfOptionalHeader	      dw SectionTable - OptionalHeader
	.Characteristics	      dw CHARACTERISTICS

	OptionalHeader:
	.Magic			      dw MAGIC
	.MajorLinkerVersion	      db 0
	.MinorLinkerVersion	      db 0
	.SizeOfCode		      dd 0
	.SizeOfInitializedData	      dd 0
	.SizeOfUninitializedData      dd 0
	.AddressOfEntryPoint	      dd 0
	.BaseOfCode		      dd 0
	if MAGIC <> 0x20B
	 .BaseOfData		      dd 0
	 .ImageBase		      dd IMAGE_BASE - RELOCATION
	else
	 .ImageBase		      dq IMAGE_BASE - RELOCATION
	end if
	.SectionAlignment	      dd SECTION_ALIGNMENT
	.FileAlignment		      dd FILE_ALIGNMENT
	.MajorOperatingSystemVersion  dw 1
	.MinorOperatingSystemVersion  dw 0
	.MajorImageVersion	      dw 0
	.MinorImageVersion	      dw 0
	.MajorSubsystemVersion	      dw MAJOR_SUBSYSTEM_VERSION
	.MinorSubsystemVersion	      dw MINOR_SUBSYSTEM_VERSION
	.Win32VersionValue	      dd 0
	.SizeOfImage		      dd SIZE_OF_IMAGE
	.SizeOfHeaders		      dd SIZE_OF_HEADERS
	.CheckSum		      dd 0
	.Subsystem		      dw SUBSYSTEM
	.DllCharacteristics	      dw DLL_CHARACTERISTICS
	if MAGIC <> 0x20B
	 .SizeOfStackReserve	      dd 1000h
	 .SizeOfStackCommit	      dd 1000h
	 .SizeOfHeapReserve	      dd 10000h
	 .SizeOfHeapCommit	      dd 0
	else
	 .SizeOfStackReserve	      dq 1000h
	 .SizeOfStackCommit	      dq 1000h
	 .SizeOfHeapReserve	      dq 10000h
	 .SizeOfHeapCommit	      dq 0
	end if
	.LoaderFlags		      dd 0
	.NumberOfRvaAndSizes	      dd NUMBER_OF_DIRECTORIES
	RvaAndSizes:
	.Rva			      dd 0
	.Size			      dd 0
	.ENTRY_LENGTH = $ - RvaAndSizes
				      db (NUMBER_OF_DIRECTORIES-1)*RvaAndSizes.ENTRY_LENGTH dup 0
	SectionTable:
	.Name			      dq '.flat'
	.VirtualSize		      dd 0
	.VirtualAddress 	      dd 0
	.SizeOfRawData		      dd 0
	.PointerToRawData	      dd 0
	.PointerToRelocations	      dd 0
	.PointerToLineNumbers	      dd 0
	.NumberOfRelocations	      dw 0
	.NumberOfLineNumbers	      dw 0
	.Characteristics	      dd IMAGE_SCN_MEM_EXECUTE + IMAGE_SCN_MEM_READ + IMAGE_SCN_MEM_WRITE
	.ENTRY_LENGTH = $ - SectionTable
				      db (NUMBER_OF_SECTIONS-1)*SectionTable.ENTRY_LENGTH dup 0

	HeadersEnd:
	define CheckSumBlocks PE,0,HeadersEnd

	SECTION_INDEX = 0
	RELOCATION_INDEX = 0
	DEFINED_SECTION = 0
	SECTION_DIRECTORIES = 0
	align SECTION_ALIGNMENT
	FIRST_SECTION_RVA:
	section $%%
	align FILE_ALIGNMENT,0
	SIZE_OF_HEADERS = $%
	FILE_OFFSET = $%
	SECTION_BASE = IMAGE_BASE + FIRST_SECTION_RVA
	org SECTION_BASE

	store SECTION_BASE-IMAGE_BASE at PE:OptionalHeader.AddressOfEntryPoint
	store SECTION_BASE-IMAGE_BASE at PE:SectionTable.VirtualAddress
	store FILE_OFFSET at PE:SectionTable.PointerToRawData

	virtual at 0
	     relocated_addresses:: rd NUMBER_OF_RELOCATIONS
	end virtual

	virtual at 0
	     relocation_types:: rw NUMBER_OF_RELOCATIONS
	end virtual

end namespace

RVA? equ -PE.IMAGE_BASE +

macro entry? address*
	namespace PE
		store address-IMAGE_BASE at PE:OptionalHeader.AddressOfEntryPoint
	end namespace
end macro

macro stack? reserve*,commit:1000h
	namespace PE
		store reserve at PE:OptionalHeader.SizeOfStackReserve
		store commit at PE:OptionalHeader.SizeOfStackCommit
	end namespace
end macro

macro heap? reserve*,commit:0
	namespace PE
		store reserve at PE:OptionalHeader.SizeOfHeapReserve
		store commit at PE:OptionalHeader.SizeOfHeapCommit
	end namespace
end macro

calminstruction calminstruction?.init? var*, val:0
	compute val, val
	publish var, val
end calminstruction

calminstruction calminstruction?.initsym? var*, val&
	publish var, val
end calminstruction

calminstruction calminstruction?.unique? name
	local counter, buffer
	init counter
	compute counter, counter + 1
	arrange buffer, name#counter
	publish name, buffer
end calminstruction

calminstruction calminstruction?.asm? line&
	local tmp, ln, buffer
	initsym tmp, unique ln
	assemble tmp
	publish ln, line
	arrange buffer, =assemble ln
	assemble buffer
end calminstruction

macro section?
	namespace PE

		repeat SECTION_DIRECTORIES
			end data
		end repeat

		local AREA,DATA_START,DATA_END
		AREA::
		DATA_START = $$
		DATA_END = $-($%-$%%)
		CheckSumBlocks reequ CheckSumBlocks,AREA,DATA_START,DATA_END

		SECTION_SIZE = $ - SECTION_BASE
		store SECTION_SIZE at PE:SectionTable.VirtualSize+SECTION_INDEX*SectionTable.ENTRY_LENGTH
		align SECTION_ALIGNMENT
		SECTION_BASE = $
		section $%%
		align FILE_ALIGNMENT,0
		RAW_DATA_SIZE = $% - FILE_OFFSET
		store RAW_DATA_SIZE at PE:SectionTable.SizeOfRawData+SECTION_INDEX*SectionTable.ENTRY_LENGTH
		FILE_OFFSET = $%
		org SECTION_BASE

		load SECTION_CHARACTERISTICS from PE:SectionTable.Characteristics+SECTION_INDEX*SectionTable.ENTRY_LENGTH
		if SECTION_SIZE > 0 & RAW_DATA_SIZE = 0
			SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_CNT_UNINITIALIZED_DATA
			store SECTION_CHARACTERISTICS at PE:SectionTable.Characteristics+SECTION_INDEX*SectionTable.ENTRY_LENGTH
		end if
		if LEGACY_HEADERS
			if SECTION_CHARACTERISTICS and IMAGE_SCN_CNT_CODE & RAW_DATA_SIZE > 0
				load CODE_SIZE from PE:OptionalHeader.SizeOfCode
				if CODE_SIZE = 0
					load CODE_BASE from PE:SectionTable.VirtualAddress+SECTION_INDEX*SectionTable.ENTRY_LENGTH
					store CODE_BASE at PE:OptionalHeader.BaseOfCode
				end if
				CODE_SIZE = CODE_SIZE + RAW_DATA_SIZE
				store CODE_SIZE at PE:OptionalHeader.SizeOfCode
			end if
			if SECTION_CHARACTERISTICS and IMAGE_SCN_CNT_INITIALIZED_DATA & RAW_DATA_SIZE > 0
				load DATA_SIZE from PE:OptionalHeader.SizeOfInitializedData
				if DATA_SIZE = 0 & MAGIC <> 0x20B
					load DATA_BASE from PE:SectionTable.VirtualAddress+SECTION_INDEX*SectionTable.ENTRY_LENGTH
					store DATA_BASE at PE:OptionalHeader.BaseOfData
				end if
				DATA_SIZE = DATA_SIZE + RAW_DATA_SIZE
				store DATA_SIZE at PE:OptionalHeader.SizeOfInitializedData
			end if
			if SECTION_CHARACTERISTICS and IMAGE_SCN_CNT_UNINITIALIZED_DATA
				load BSS_SIZE from PE:OptionalHeader.SizeOfUninitializedData
				BSS_SIZE = BSS_SIZE + SECTION_SIZE
				store BSS_SIZE at PE:OptionalHeader.SizeOfUninitializedData
			end if
		end if

		if DEFINED_SECTION | SECTION_SIZE > 0
			SECTION_INDEX = SECTION_INDEX + 1
		end if

	end namespace
end macro

macro section? declaration*
	namespace PE

		section

		DEFINED_SECTION = 1

		store SECTION_BASE-IMAGE_BASE at PE:SectionTable.VirtualAddress+SECTION_INDEX*SectionTable.ENTRY_LENGTH
		store FILE_OFFSET at PE:SectionTable.PointerToRawData+SECTION_INDEX*SectionTable.ENTRY_LENGTH

		SECTION_DIRECTORIES = 0

		match name attributes, declaration

			store name:qword at PE:SectionTable.Name+SECTION_INDEX*SectionTable.ENTRY_LENGTH

			SECTION_CHARACTERISTICS = 0

			local seq
			define seq attributes:
			while 1
				match :, seq
					break
				else match =readable? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_MEM_READ
				else match =writeable? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_MEM_WRITE
				else match =writable? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_MEM_WRITE
				else match =executable? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_MEM_EXECUTE
				else match =discardable? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_MEM_DISCARDABLE
				else match =shareable? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_MEM_SHARED
				else match =import? tail, seq
					redefine seq tail
					SECTION_DIRECTORIES = SECTION_DIRECTORIES + 1
					data import
				else match =export? tail, seq
					redefine seq tail
					SECTION_DIRECTORIES = SECTION_DIRECTORIES + 1
					data export
				else match =resource? =from? path tail, seq
					redefine seq tail
					SECTION_DIRECTORIES = SECTION_DIRECTORIES + 1
					data resource from path
				else match =resource? tail, seq
					redefine seq tail
					SECTION_DIRECTORIES = SECTION_DIRECTORIES + 1
					data resource
				else match =fixups? tail, seq
					redefine seq tail
					SECTION_DIRECTORIES = SECTION_DIRECTORIES + 1
					data fixups
				else match =code? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_CNT_CODE
				else match =data? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_CNT_INITIALIZED_DATA
				else match =udata? tail, seq
					redefine seq tail
					SECTION_CHARACTERISTICS = SECTION_CHARACTERISTICS or IMAGE_SCN_CNT_UNINITIALIZED_DATA
				else match attribute tail, seq
					err 'unknown attribute "',`attribute,'"'
					redefine seq :
				end match
			end while

			store SECTION_CHARACTERISTICS at PE:SectionTable.Characteristics+SECTION_INDEX*SectionTable.ENTRY_LENGTH
		else

			store declaration:qword at PE:SectionTable.Name+SECTION_INDEX*SectionTable.ENTRY_LENGTH

		end match

	end namespace
end macro

macro data? type*
	namespace PE
		local number,content
		define content
		match =export?, type
			number = 0
		else match =import?, type
			number = 1
		else match =resource? =from? path, type
			number = 2
			define content resource_from path
		else match =resource?, type
			number = 2
		else match =fixups?, type
			number = 5
			define content fixups
		else
			number = type
		end match
		define DATA_DIRECTORY number
		load DATA_BASE:dword from PE:RvaAndSizes.Rva+DATA_DIRECTORY*RvaAndSizes.ENTRY_LENGTH
		if DATA_BASE = 0
			store $-IMAGE_BASE:dword at PE:RvaAndSizes.Rva+DATA_DIRECTORY*RvaAndSizes.ENTRY_LENGTH
			match instruction, content
				instruction
			end match
		else
			err 'data already defined'
		end if
	end namespace
end macro

macro end?.data?
	namespace PE
		load DATA_BASE:dword from PE:RvaAndSizes.Rva+DATA_DIRECTORY*RvaAndSizes.ENTRY_LENGTH
		store $-IMAGE_BASE-DATA_BASE:dword at PE:RvaAndSizes.Size+DATA_DIRECTORY*RvaAndSizes.ENTRY_LENGTH
		restore DATA_DIRECTORY
	end namespace
end macro

macro PE.resource_directory?
	namespace PE
		Resource: rb RESOURCE_HEADERS_LENGTH
		Resource.counter = 0
		define RESOURCE_DIRECTORIES_LIST Resource_root
		Resource_root.counter = 0
	end namespace
end macro

macro PE.resource_data? type*,id*,lang*,codepage:0
	namespace PE
		local _type,_id,_lang
		_type = type
		_id = id
		_lang = lang
		if ~ type eqtype 0
			_type = _type shl 32
		end if
		if ~ id eqtype 0
			_id = id shl 32
		end if
		if ~ lang eqtype 0
			_lang = lang shl 32
		end if
		repeat 1, %type:_type, %id:_id, %lang:_lang
			if ~ defined Resource_#%type#_#%id.counter
				if ~ defined Resource_#%type.counter
					repeat 1, i:Resource_root.counter
						Resource_root.entry#i = type
						Resource_root.offset#i =  (Resource_#%type - Resource) or 80000000h
					end repeat
					Resource_root.counter = Resource_root.counter + 1
					match list, RESOURCE_DIRECTORIES_LIST
						define RESOURCE_DIRECTORIES_LIST list,Resource_#%type
					end match
					Resource_#%type.counter = 0
				end if
				repeat 1, i:Resource_#%type.counter
					Resource_#%type.entry#i = id
					Resource_#%type.offset#i =  (Resource_#%type#_#%id - Resource) or 80000000h
				end repeat
				Resource_#%type.counter = Resource_#%type.counter + 1

				match list, RESOURCE_DIRECTORIES_LIST
					define RESOURCE_DIRECTORIES_LIST list,Resource_#%type#_#%id
				end match
				Resource_#%type#_#%id.counter = 0
			end if
			repeat 1, i:Resource_#%type#_#%id.counter
				Resource_#%type#_#%id.entry#i = lang
				Resource_#%type#_#%id.offset#i = Resource_#%type#_#%id#_#%lang - Resource
			end repeat
			Resource_#%type#_#%id.counter = Resource_#%type#_#%id.counter + 1
			repeat 1, i:Resource.counter
				Resource_#%type#_#%id#_#%lang := Resource.entry#i
				Resource.cp#i := codepage
				Resource.data#i:
			end repeat
		end repeat
	end namespace
end macro

macro PE.end_resource_data?
	namespace PE
		repeat 1, i:Resource.counter
			Resource.size#i := $ - Resource.data#i
		end repeat
		Resource.counter = Resource.counter + 1
		align 4
	end namespace
end macro

macro PE.end_resource_directory?
       namespace PE
		RESOURCE_HEADERS_POINTER = 0

		match list, RESOURCE_DIRECTORIES_LIST
			iterate dir, list
				dir := Resource + RESOURCE_HEADERS_POINTER
				RESOURCE_HEADERS_POINTER = RESOURCE_HEADERS_POINTER + 16 + dir.counter * 8
				local x,y,z,a,b
				x = dir.counter shr 1
				while x > 0
					y = x
					while y < dir.counter
						z = y
						while z-x >= 0
							repeat 1, i:z, j:z-x
								if dir.entry#i eqtype 0
									if ~ dir.entry#j eqtype 0 | dir.entry#i >= dir.entry#j
										z = 0
									end if
								else if ~ dir.entry#j eqtype 0
									a = dir.entry#i bswap lengthof dir.entry#i
									b = dir.entry#j bswap lengthof dir.entry#j
									if ( lengthof a >= lengthof b & a shr ((lengthof a - lengthof b)*8) >= b ) | ( lengthof a < lengthof b & a > b shr ((lengthof b - lengthof a)*8) )
										z = 0
									end if
								end if
								if z > 0
									a = dir.entry#i
									b = dir.offset#i
									dir.entry#i = dir.entry#j
									dir.offset#i = dir.offset#j
									dir.entry#j = a
									dir.offset#j = b
									z = z - x
								end if
							end repeat
						end while
						y = y + 1
					end while
					x = x shr 1
				end while
			end iterate
			iterate dir, list
				store __TIME__ : 4 at dir + 4
				dir.names_counter = 0
				repeat dir.counter, i:0
					if dir.entry#i eqtype 0
						store dir.entry#i : 4 at dir + 16 + i * 8
					else
						dir.names_counter = dir.names_counter + 1
						repeat 1, %id:dir.entry#i
							if ~ defined Resource_string#%id
								restore Resource_string#%id
								Resource_string#%id = Resource + RESOURCE_HEADERS_POINTER
								if lengthof dir.entry#i and 1
									err 'a word-aligned string is expected as a name'
								end if
								RESOURCE_HEADERS_POINTER = RESOURCE_HEADERS_POINTER + lengthof dir.entry#i + 2
								store (lengthof dir.entry#i)/2 : 2 at Resource_string#%id
								store dir.entry#i : lengthof dir.entry#i at Resource_string#%id + 2
							end if
							store (Resource_string#%id - Resource) or 80000000h : 4 at dir + 16 + i * 8
						end repeat
					end if
					store dir.offset#i : 4 at dir + 16 + i * 8 + 4
				end repeat
				store dir.names_counter : 2 at dir + 12
				store dir.counter - dir.names_counter : 2 at dir + 14
			end iterate
		end match

		if RESOURCE_HEADERS_POINTER and 11b
			RESOURCE_HEADERS_POINTER = RESOURCE_HEADERS_POINTER + 4 - RESOURCE_HEADERS_POINTER and 11b
		end if

		repeat Resource.counter, i:0
			Resource.entry#i := Resource + RESOURCE_HEADERS_POINTER
			RESOURCE_HEADERS_POINTER = RESOURCE_HEADERS_POINTER + 16
			store RVA(Resource.data#i) : 4 at Resource.entry#i
			store Resource.size#i : 4 at Resource.entry#i + 4
			store Resource.cp#i : 4 at Resource.entry#i + 8
		end repeat

		RESOURCE_HEADERS_LENGTH = RESOURCE_HEADERS_POINTER
	end namespace
end macro

macro PE.resource_from path*

	local res_file,res_size,res_header_size,res_data_size,res_data
	local offset,char,type,id,lang

	virtual at 0
		res_file:: file path
		res_size := $
	end virtual

	PE.resource_directory

		offset = 0
		while offset < res_size
			load res_header_size : 4 from res_file : offset + 4
			load res_data_size : 4 from res_file : offset + 0

			if res_data_size > 0

				offset =: offset + 8
				load char : 2 from res_file : offset
				if char = 0FFFFh
					load char : 2 from res_file : offset + 2
					type = +char
					offset = offset + 4
				else
					while 1
						if char = 0
							load type : (%-1)*2 from res_file : offset
							offset = offset + (% + % and 1)*2
							break
						end if
						load char : 2 from res_file : offset + %*2
					end while
				end if
				load char : 2 from res_file : offset
				if char = 0FFFFh
					load char : 2 from res_file : offset + 2
					id = +char
					offset = offset + 4
				else
					while 1
						if char = 0
							load id : (%-1)*2 from res_file : offset
							offset = offset + (% + % and 1)*2
							break
						end if
						load char : 2 from res_file : offset + %*2
					end while
				end if
				load char : 2 from res_file : offset + 6
				lang = +char

				restore offset

				PE.resource_data type,id,lang
					load res_data : res_data_size from res_file : offset + res_header_size
					db res_data
				PE.end_resource_data

			end if

			offset = offset + res_header_size + res_data_size
			if offset and 11b
				offset = offset + 4 - offset and 11b
			end if
		end while

	PE.end_resource_directory

end macro

macro PE.fixups
	namespace PE
		Fixups:
		calminstruction BuildFixups
			local	PAGE_RVA, BLOCK_HEADER, BLOCK_SIZE
			local	INDEX, ADDRESS, TYPE, FIXUP
			compute PAGE_RVA, -1
			compute BLOCK_HEADER, 0
			compute BLOCK_SIZE, 0
			compute INDEX,0
		    process:
			check	INDEX = NUMBER_OF_RELOCATIONS
			jyes	close_block
			asm	load ADDRESS:4 from relocated_addresses:INDEX shl 2
			check	PAGE_RVA >= 0 & ADDRESS and not 0FFFh = PAGE_RVA
			jyes	append_to_block
		    close_block:
			check	BLOCK_HEADER
			jno	start_new_block
			check	BLOCK_SIZE and 11b
			jno	finish_block
			asm	dw 0
			compute BLOCK_SIZE, BLOCK_SIZE + 2
		    finish_block:
			asm	store BLOCK_SIZE:4 at BLOCK_HEADER+4
		    start_new_block:
			check	INDEX = NUMBER_OF_RELOCATIONS
			jyes	done
			compute PAGE_RVA, ADDRESS and not 0FFFh
			compute BLOCK_HEADER, $
			asm	dd PAGE_RVA, 0
			compute BLOCK_SIZE, 8
		    append_to_block:
			asm	load TYPE:2 from relocation_types:INDEX shl 1
			compute FIXUP, (ADDRESS and 0FFFh) or (TYPE shl 12)
			asm	dw FIXUP
			compute BLOCK_SIZE, BLOCK_SIZE + 2
			compute INDEX, INDEX + 1
			jump	process
		    done:
		end calminstruction
		BuildFixups
	end namespace
end macro

if defined PE.Fixups

	calminstruction dword? value
		compute value, value
		check	value relativeto 0 | ~ value relativeto PE.RELOCATION
		jyes	plain
		local	offset
		compute offset, $%
		asm	emit 4: value - PE.RELOCATION
		check	$% > offset
		jno	done
		asm	store $-4-PE.IMAGE_BASE:4 at PE.relocated_addresses:PE.RELOCATION_INDEX shl 2
		asm	store IMAGE_REL_BASED_HIGHLOW:2 at PE.relocation_types:PE.RELOCATION_INDEX shl 1
		compute PE.RELOCATION_INDEX, PE.RELOCATION_INDEX + 1
	    done:
		exit
	    plain:
		asm	emit 4: value
	end calminstruction

	calminstruction qword? value
		compute value, value
		check	value relativeto 0 | ~ value relativeto PE.RELOCATION
		jyes	plain
		local	offset
		compute offset, $%
		asm	emit 8: value - PE.RELOCATION
		check	$% > offset
		jno	done
		asm	store $-8-PE.IMAGE_BASE:4 at PE.relocated_addresses:PE.RELOCATION_INDEX shl 2
		asm	store IMAGE_REL_BASED_DIR64:2 at PE.relocation_types:PE.RELOCATION_INDEX shl 1
		compute PE.RELOCATION_INDEX, PE.RELOCATION_INDEX + 1
	    done:
		exit
	    plain:
		asm	emit 8: value
	end calminstruction

	iterate <dd,dword>, dd,dword, dq,qword

		calminstruction dd? definitions&
			local	value, n
		    start:
			match	value=,definitions, definitions, ()
			jyes	recognize
			match	value, definitions
			arrange definitions,
		    recognize:
			match	n =dup? value, value, ()
			jyes	duplicate
			match	?, value
			jyes	reserve
			arrange value, =dword value
			assemble value
		    next:
			match	, definitions
			jno	start
			take	, definitions
			take	definitions, definitions
			jyes	next
			exit
		    reserve:
			arrange value, =dd ?
			assemble value
			jump	next
		    duplicate:
			match	(value), value
		    stack:
			check	n
			jno	next
			take	definitions, value
			arrange value, definitions
			compute n, n - 1
			jump	stack
		end calminstruction

		calminstruction (label) dd? definitions&
			local	cmd
			arrange cmd, =label label : =dword
			assemble cmd
			arrange cmd, =dd definitions
			assemble cmd
		end calminstruction

	end iterate

end if

postpone
	purge section?
	section
	namespace PE
		SIZE_OF_IMAGE := SECTION_BASE - IMAGE_BASE
		NUMBER_OF_SECTIONS := SECTION_INDEX
		NUMBER_OF_RELOCATIONS := RELOCATION_INDEX
	end namespace
end postpone

postpone ?
	namespace PE
		CHECKSUM = 0

		calminstruction CheckSum
			local	AREA, DATA_START, DATA_END, POS, H
		    get_block:
			match	AREA=,DATA_START=,DATA_END=,CheckSumBlocks, CheckSumBlocks
			jyes	block_ready
			match	AREA=,DATA_START=,DATA_END, CheckSumBlocks
			jyes	last_block
			exit
		    last_block:
			arrange CheckSumBlocks,
		    block_ready:
			compute POS, DATA_START
		    process_block:
			check	POS + 2 <= DATA_END
			jno	finish_block
			asm	load H:2 from AREA:POS
			compute CHECKSUM, CHECKSUM + H
			compute POS, POS + 2
			jump	process_block
		    finish_block:
			check	POS + 1 = DATA_END
			jno	reduce_checksum
			asm	load H:1 from AREA:POS
			compute CHECKSUM, CHECKSUM + H
		    reduce_checksum:
			check	CHECKSUM shr 16
			jno	get_block
			compute CHECKSUM, CHECKSUM shr 16 + CHECKSUM and 0FFFFh
			jump	reduce_checksum
		    done:
		end calminstruction

		CheckSum
		CHECKSUM = CHECKSUM + $%
		store CHECKSUM at PE:OptionalHeader.CheckSum
	end namespace
end postpone
