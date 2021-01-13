
macro struct? name
	macro end?.struct?!
			end namespace
		esc end struc
		virtual at 0
			name name
			sizeof.name = $
		end virtual
		purge end?.struct?
	end macro
	esc struc name
		label . : sizeof.name
		namespace .
end macro

struct IMAGE_SECTION_HEADER
	Name			db 8 dup ?
	VirtualSize		dd ?
	VirtualAddress		dd ?
	SizeOfRawData		dd ?
	PointerToRawData	dd ?
	PointerToRelocations	dd ?
	PointerToLinenumbers	dd ?
	NumberOfRelocations	dw ?
	NumberOfLinenumbers	dw ?
	Characteristics 	dd ?
end struct

struct IMAGE_RELOCATION
	VirtualAddress		dd ?
	SymbolTableIndex	dd ?
	Type			dw ?
end struct

struct IMAGE_SYMBOL
	ShortName		db 8 dup ?
    virtual at ShortName
	Zeroes			dd ?
	Offset			dd ?
    end virtual
	Value			dd ?
	SectionNumber		dw ?
	Type			dw ?
	StorageClass		db ?
	NumberOfAuxSymbols	db ?
end struct

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

IMAGE_REL_AMD64_ABSOLUTE = 0x0000
IMAGE_REL_AMD64_ADDR64	 = 0x0001
IMAGE_REL_AMD64_ADDR32	 = 0x0002
IMAGE_REL_AMD64_ADDR32NB = 0x0003
IMAGE_REL_AMD64_REL32	 = 0x0004
IMAGE_REL_AMD64_REL32_1  = 0x0005
IMAGE_REL_AMD64_REL32_2  = 0x0006
IMAGE_REL_AMD64_REL32_3  = 0x0007
IMAGE_REL_AMD64_REL32_4  = 0x0008
IMAGE_REL_AMD64_REL32_5  = 0x0009
IMAGE_REL_AMD64_SECTION  = 0x000A
IMAGE_REL_AMD64_SECREL	 = 0x000B
IMAGE_REL_AMD64_SECREL7  = 0x000C
IMAGE_REL_AMD64_TOKEN	 = 0x000D
IMAGE_REL_AMD64_SREL32	 = 0x000E
IMAGE_REL_AMD64_PAIR	 = 0x000F
IMAGE_REL_AMD64_SSPAN32  = 0x0010

IMAGE_REL_I386_ABSOLUTE = 0x0000
IMAGE_REL_I386_DIR16	= 0x0001
IMAGE_REL_I386_REL16	= 0x0002
IMAGE_REL_I386_DIR32	= 0x0006
IMAGE_REL_I386_DIR32NB	= 0x0007
IMAGE_REL_I386_SEG12	= 0x0009
IMAGE_REL_I386_SECTION	= 0x000A
IMAGE_REL_I386_SECREL	= 0x000B
IMAGE_REL_I386_TOKEN	= 0x000C
IMAGE_REL_I386_SECREL7	= 0x000D
IMAGE_REL_I386_REL32	= 0x0014

IMAGE_SYM_UNDEFINED  = 0
IMAGE_SYM_ABSOLUTE   = -1
IMAGE_SYM_DEBUG      = -2

IMAGE_SYM_TYPE_NULL   = 0
IMAGE_SYM_TYPE_VOID   = 1
IMAGE_SYM_TYPE_CHAR   = 2
IMAGE_SYM_TYPE_SHORT  = 3
IMAGE_SYM_TYPE_INT    = 4
IMAGE_SYM_TYPE_LONG   = 5
IMAGE_SYM_TYPE_FLOAT  = 6
IMAGE_SYM_TYPE_DOUBLE = 7
IMAGE_SYM_TYPE_STRUCT = 8
IMAGE_SYM_TYPE_UNION  = 9
IMAGE_SYM_TYPE_ENUM   = 10
IMAGE_SYM_TYPE_MOE    = 11
IMAGE_SYM_TYPE_BYTE   = 12
IMAGE_SYM_TYPE_WORD   = 13
IMAGE_SYM_TYPE_UINT   = 14
IMAGE_SYM_TYPE_DWORD  = 15

IMAGE_SYM_DTYPE_NULL	 = 0
IMAGE_SYM_DTYPE_POINTER  = 1
IMAGE_SYM_DTYPE_FUNCTION = 2
IMAGE_SYM_DTYPE_ARRAY	 = 3

IMAGE_SYM_CLASS_END_OF_FUNCTION  = -1
IMAGE_SYM_CLASS_NULL		 = 0
IMAGE_SYM_CLASS_AUTOMATIC	 = 1
IMAGE_SYM_CLASS_EXTERNAL	 = 2
IMAGE_SYM_CLASS_STATIC		 = 3
IMAGE_SYM_CLASS_REGISTER	 = 4
IMAGE_SYM_CLASS_EXTERNAL_DEF	 = 5
IMAGE_SYM_CLASS_LABEL		 = 6
IMAGE_SYM_CLASS_UNDEFINED_LABEL  = 7
IMAGE_SYM_CLASS_MEMBER_OF_STRUCT = 8
IMAGE_SYM_CLASS_ARGUMENT	 = 9
IMAGE_SYM_CLASS_STRUCT_TAG	 = 10
IMAGE_SYM_CLASS_MEMBER_OF_UNION  = 11
IMAGE_SYM_CLASS_UNION_TAG	 = 12
IMAGE_SYM_CLASS_TYPE_DEFINITION  = 13
IMAGE_SYM_CLASS_UNDEFINED_STATIC = 14
IMAGE_SYM_CLASS_ENUM_TAG	 = 15
IMAGE_SYM_CLASS_MEMBER_OF_ENUM	 = 16
IMAGE_SYM_CLASS_REGISTER_PARAM	 = 17
IMAGE_SYM_CLASS_BIT_FIELD	 = 18
IMAGE_SYM_CLASS_BLOCK		 = 100
IMAGE_SYM_CLASS_FUNCTION	 = 101
IMAGE_SYM_CLASS_END_OF_STRUCT	 = 102
IMAGE_SYM_CLASS_FILE		 = 103
IMAGE_SYM_CLASS_SECTION 	 = 104
IMAGE_SYM_CLASS_WEAK_EXTERNAL	 = 105
IMAGE_SYM_CLASS_CLR_TOKEN	 = 107

IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY = 1
IMAGE_WEAK_EXTERN_SEARCH_LIBRARY   = 2
IMAGE_WEAK_EXTERN_SEARCH_ALIAS	   = 3

COFF::

namespace COFF

	if defined Settings.Machine
	      MACHINE := Settings.Machine
	else
	      MACHINE := IMAGE_FILE_MACHINE_I386
	end if

	if defined Settings.Characteristics
	      CHARACTERISTICS = Settings.Characteristics
	else
	      CHARACTERISTICS = IMAGE_FILE_32BIT_MACHINE
	end if

	Header:

	Machine 		dw MACHINE
	NumberOfSections	dw NUMBER_OF_SECTIONS
	TimeDateStamp		dd __TIME__
	PointerToSymbolTable	dd SYMBOL_TABLE_OFFSET
	NumberOfSymbols 	dd NUMBER_OF_SYMBOLS
	SizeOfOptionalHeader	dw 0
	Characteristics 	dw CHARACTERISTICS

	Sections:		db NUMBER_OF_SECTIONS * sizeof IMAGE_SECTION_HEADER dup 0

	virtual at 0
		symbol_table:: rb NUMBER_OF_SYMBOLS * sizeof IMAGE_SYMBOL
	end virtual

	virtual at 0
		string_table:: dd STRING_TABLE_SIZE
		STRING_POSITION = $
		rb STRING_TABLE_SIZE - $
	end virtual

	virtual at 0
		relocations:: rb NUMBER_OF_RELOCATIONS * sizeof IMAGE_RELOCATION
	end virtual

	element relocatable?

	macro section_start
		local sym
		element sym : relocatable * (1+SECTION_INDEX) + SYMBOL_INDEX
		SECTION_BASE = sym
		org sym
		if DEFINED_SECTION | DEFAULT_SECTION_PRESENT
			store SECTION_NAME : 8 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.ShortName
			store IMAGE_SYM_CLASS_STATIC at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.StorageClass
			store 1+SECTION_INDEX at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.SectionNumber
			SYMBOL_INDEX = SYMBOL_INDEX + 1
		end if
	end macro

	RELOCATION_INDEX = 0
	SECTION_INDEX = 0
	SECTION_RELOCATION_INDEX = RELOCATION_INDEX
	SYMBOL_INDEX = 0

	SECTION_OFFSET = $%
	SECTION_ALIGN = 4
	SECTION_NAME = '.flat'
	SECTION_FLAGS = IMAGE_SCN_CNT_CODE + IMAGE_SCN_CNT_INITIALIZED_DATA
	DEFINED_SECTION = 0
	DEFAULT_SECTION_PRESENT = 0
	section_start

end namespace

macro section?
	namespace COFF

		SECTION_SIZE = $% - SECTION_OFFSET

		if DEFINED_SECTION | SECTION_SIZE > 0

			if ~ DEFINED_SECTION
				DEFAULT_SECTION_PRESENT = 1
			end if

			if $%% = SECTION_OFFSET
				SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_CNT_UNINITIALIZED_DATA
				SECTION_OFFSET = 0
				section $
			else
				UNINITIALIZED_LENGTH = $% - $%%
				section $
				db UNINITIALIZED_LENGTH dup 0
			end if

			if RELOCATION_INDEX > SECTION_RELOCATION_INDEX
				store RELOCATIONS_OFFSET + SECTION_RELOCATION_INDEX * sizeof IMAGE_RELOCATION at COFF:Sections + SECTION_INDEX * sizeof IMAGE_SECTION_HEADER + IMAGE_SECTION_HEADER.PointerToRelocations
				if RELOCATION_INDEX - SECTION_RELOCATION_INDEX > 65535
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_LNK_NRELOC_OVFL
					store 0FFFFh at COFF:Sections + SECTION_INDEX * sizeof IMAGE_SECTION_HEADER + IMAGE_SECTION_HEADER.NumberOfRelocations
					load RELOCATIONS : (RELOCATION_INDEX - SECTION_RELOCATION_INDEX) * sizeof IMAGE_RELOCATION from relocations : SECTION_RELOCATION_INDEX * sizeof IMAGE_RELOCATION
					store RELOCATIONS : (RELOCATION_INDEX - SECTION_RELOCATION_INDEX) * sizeof IMAGE_RELOCATION at relocations : (SECTION_RELOCATION_INDEX+1) * sizeof IMAGE_RELOCATION
					RELOCATION_INDEX = RELOCATION_INDEX + 1
					store RELOCATION_INDEX - SECTION_RELOCATION_INDEX at relocations : SECTION_RELOCATION_INDEX * sizeof IMAGE_RELOCATION + IMAGE_RELOCATION.VirtualAddress
					store 0 at relocations : SECTION_RELOCATION_INDEX * sizeof IMAGE_RELOCATION + IMAGE_RELOCATION.SymbolTableIndex
					store 0 at relocations : SECTION_RELOCATION_INDEX * sizeof IMAGE_RELOCATION + IMAGE_RELOCATION.Type
				else
					store RELOCATION_INDEX - SECTION_RELOCATION_INDEX at COFF:Sections + SECTION_INDEX * sizeof IMAGE_SECTION_HEADER + IMAGE_SECTION_HEADER.NumberOfRelocations
				end if
			end if

			store SECTION_NAME : 8 at COFF:Sections + SECTION_INDEX * sizeof IMAGE_SECTION_HEADER + IMAGE_SECTION_HEADER.Name
			store SECTION_OFFSET at COFF:Sections + SECTION_INDEX * sizeof IMAGE_SECTION_HEADER + IMAGE_SECTION_HEADER.PointerToRawData
			store SECTION_SIZE at COFF:Sections + SECTION_INDEX * sizeof IMAGE_SECTION_HEADER + IMAGE_SECTION_HEADER.SizeOfRawData
			store SECTION_FLAGS at COFF:Sections + SECTION_INDEX * sizeof IMAGE_SECTION_HEADER + IMAGE_SECTION_HEADER.Characteristics

			SECTION_INDEX = SECTION_INDEX + 1

		end if

	end namespace
end macro

macro section? declaration*
	namespace COFF

		section

		DEFINED_SECTION = 1
		SECTION_FLAGS = 0
		SECTION_OFFSET = $%
		SECTION_ALIGN = 4

		match name attributes, declaration

			SECTION_NAME = name

			local seq,list
			match flags =align? boundary, attributes
				SECTION_ALIGN = boundary
				define seq flags
			else match =align? boundary, attributes
				SECTION_ALIGN = boundary
				define seq
			else
				define seq attributes
			end match
			while 1
				match car cdr, seq
					define list car
					define seq cdr
				else
					match any, seq
						define list any
					end match
					break
				end match
			end while
			irpv attribute, list
				match =code?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_CNT_CODE
				else match =data?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_CNT_INITIALIZED_DATA
				else match =readable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_MEM_READ
				else match =writeable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_MEM_WRITE
				else match =executable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_MEM_EXECUTE
				else match =shareable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_MEM_SHARED
				else match =discardable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_MEM_DISCARDABLE
				else match =notpageable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_MEM_NOT_PAGED
				else match =linkremove?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_LNK_REMOVE
				else match =linkinfo?, attribute
					SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_LNK_INFO
				else
					err 'unknown attribute "',`attribute,'"'
				end match
			end irpv

		else

			SECTION_NAME = declaration

		end match

		repeat 1, i:SECTION_ALIGN
			if defined IMAGE_SCN_ALIGN_#i#BYTES
				SECTION_FLAGS = SECTION_FLAGS or IMAGE_SCN_ALIGN_#i#BYTES
			else
				err 'invalid section alignment'
			end if
		end repeat

		section_start

		SECTION_RELOCATION_INDEX = RELOCATION_INDEX

	end namespace
end macro

calminstruction align? boundary,value:?
	check	COFF.SECTION_ALIGN mod (boundary) = 0
	jyes	allowed
	arrange value, =err 'section not aligned enough'
	assemble value
	exit
    allowed:
	compute boundary, (boundary-1)-($-COFF.SECTION_BASE+boundary-1) mod boundary
	arrange value, =db boundary =dup value
	assemble value
end calminstruction

macro public? declaration*
	namespace COFF
		match =static? value =as? str, declaration
			SYMBOL_VALUE = value
			SYMBOL_NAME = string str
			SYMBOL_CLASS = IMAGE_SYM_CLASS_STATIC
		else match value =as? str, declaration
			SYMBOL_VALUE = value
			SYMBOL_NAME = string str
			SYMBOL_CLASS = IMAGE_SYM_CLASS_EXTERNAL
		else match =static? value, declaration
			SYMBOL_VALUE = value
			SYMBOL_NAME = `value
			SYMBOL_CLASS = IMAGE_SYM_CLASS_STATIC
		else
			SYMBOL_VALUE = declaration
			SYMBOL_NAME = `declaration
			SYMBOL_CLASS = IMAGE_SYM_CLASS_EXTERNAL
		end match
		if SYMBOL_VALUE relativeto 1 elementof SYMBOL_VALUE & 1 elementof (1 metadataof SYMBOL_VALUE) relativeto relocatable & SYMBOL_CLASS = IMAGE_SYM_CLASS_EXTERNAL
			if 1 scaleof (1 metadataof SYMBOL_VALUE) > 0
				SYMBOL_SECTION_INDEX = 1 scaleof (1 metadataof SYMBOL_VALUE)
				SYMBOL_VALUE = SYMBOL_VALUE - 1 elementof SYMBOL_VALUE
			else
				SYMBOL_SECTION_INDEX = IMAGE_SYM_UNDEFINED
				SYMBOL_VALUE = 0
				SYMBOL_CLASS = IMAGE_SYM_CLASS_WEAK_EXTERNAL
				SYMBOL_TAG = 0 scaleof (1 metadataof SYMBOL_VALUE)
			end if
		else
			SYMBOL_SECTION_INDEX = IMAGE_SYM_ABSOLUTE
		end if
		if lengthof SYMBOL_NAME > 8
			store 0 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.Zeroes
			store STRING_POSITION at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.Offset
			store SYMBOL_NAME : lengthof SYMBOL_NAME at string_table:STRING_POSITION
			STRING_POSITION = STRING_POSITION + lengthof SYMBOL_NAME + 1
		else
			store SYMBOL_NAME : 8 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.ShortName
		end if
		store SYMBOL_VALUE at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.Value
		store SYMBOL_SECTION_INDEX at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.SectionNumber
		store SYMBOL_CLASS at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.StorageClass
		if SYMBOL_CLASS = IMAGE_SYM_CLASS_WEAK_EXTERNAL
			store 1 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.NumberOfAuxSymbols
			SYMBOL_INDEX = SYMBOL_INDEX + 1
			store SYMBOL_TAG : 4 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL
			store IMAGE_WEAK_EXTERN_SEARCH_ALIAS : 4 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + 4
		end if
		SYMBOL_INDEX = SYMBOL_INDEX + 1
	end namespace
end macro

macro extrn? declaration*
	namespace COFF
		local sym,psym
		element sym : relocatable * (-1) + SYMBOL_INDEX
		match str =as? name:size, declaration
			label name:size at sym
			SYMBOL_NAME = string str
		else match name:size, declaration
			label name:size at sym
			SYMBOL_NAME = `name
		else match str =as? name, declaration
			label name at sym
			SYMBOL_NAME = string str
		else
			label declaration at sym
			SYMBOL_NAME = `declaration
		end match
		if lengthof SYMBOL_NAME > 8
			store 0 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.Zeroes
			store STRING_POSITION at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.Offset
			store SYMBOL_NAME : lengthof SYMBOL_NAME at string_table:STRING_POSITION
			STRING_POSITION = STRING_POSITION + lengthof SYMBOL_NAME + 1
		else
			store SYMBOL_NAME : 8 at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.ShortName
		end if
		store IMAGE_SYM_CLASS_EXTERNAL at symbol_table : SYMBOL_INDEX * sizeof IMAGE_SYMBOL + IMAGE_SYMBOL.StorageClass
		SYMBOL_INDEX = SYMBOL_INDEX + 1
	end namespace
end macro

element COFF.IMAGE_BASE
RVA? equ -COFF.IMAGE_BASE+

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

if COFF.MACHINE = IMAGE_FILE_MACHINE_I386

	calminstruction dword? value
		compute value, value
		check	~ value relativeto 0 & value relativeto 1 elementof value & 1 elementof (1 metadataof value) relativeto COFF.relocatable
		jyes	dir32
		check	~ value relativeto 0 & (value + COFF.IMAGE_BASE) relativeto 1 elementof (value + COFF.IMAGE_BASE) & 1 elementof (1 metadataof (value + COFF.IMAGE_BASE)) relativeto COFF.relocatable
		jyes	dir32nb
		check	~ value relativeto 0 & (value + COFF.SECTION_BASE) relativeto 1 elementof (value + COFF.SECTION_BASE)
		jno	plain
		check	1 elementof (1 metadataof (value + COFF.SECTION_BASE)) relativeto COFF.relocatable
		jyes	rel32
	    plain:
		asm	emit 4: value
		exit
		local	offset, symndx, type
	    dir32:
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_I386_DIR32
		jump	add_relocation
	    dir32nb:
		compute value, value + COFF.IMAGE_BASE
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_I386_DIR32NB
		jump	add_relocation
	    rel32:
		compute value, value + (COFF.SECTION_BASE + $% + 4 - COFF.SECTION_OFFSET)
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_I386_REL32
		jump	add_relocation
	    add_relocation:
		compute offset, $%
		asm	emit 4: 0 scaleof value
		check	$% > offset
		jno	done
		compute offset, offset - COFF.SECTION_OFFSET
		local	relocation
		compute relocation, COFF.RELOCATION_INDEX * sizeof IMAGE_RELOCATION
		asm	store offset at COFF.relocations : relocation + IMAGE_RELOCATION.VirtualAddress
		asm	store symndx at COFF.relocations : relocation + IMAGE_RELOCATION.SymbolTableIndex
		asm	store type at COFF.relocations : relocation + IMAGE_RELOCATION.Type
		compute COFF.RELOCATION_INDEX, COFF.RELOCATION_INDEX + 1
	    done:
	end calminstruction

	calminstruction qword? value
		compute value, value
		check	~ value relativeto 0 & value relativeto 1 elementof value & 1 elementof (1 metadataof value) relativeto COFF.relocatable
		jyes	dir32
		check	~ value relativeto 0 & (value + COFF.IMAGE_BASE) relativeto 1 elementof (value + COFF.IMAGE_BASE) & 1 elementof (1 metadataof (value + COFF.IMAGE_BASE)) relativeto COFF.relocatable
		jyes	dir32nb
	    plain:
		asm	emit 8: value
		exit
		local	offset, symndx, type
	    dir32:
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_I386_DIR32
		jump	add_relocation
	    dir32nb:
		compute value, value + COFF.IMAGE_BASE
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_I386_DIR32NB
		jump	add_relocation
	    add_relocation:
		compute offset, $%
		asm	emit 8: 0 scaleof value
		check	$% > offset
		jno	done
		compute offset, offset - COFF.SECTION_OFFSET
		local	relocation
		compute relocation, COFF.RELOCATION_INDEX * sizeof IMAGE_RELOCATION
		asm	store offset at COFF.relocations : relocation + IMAGE_RELOCATION.VirtualAddress
		asm	store symndx at COFF.relocations : relocation + IMAGE_RELOCATION.SymbolTableIndex
		asm	store type at COFF.relocations : relocation + IMAGE_RELOCATION.Type
		compute COFF.RELOCATION_INDEX, COFF.RELOCATION_INDEX + 1
	    done:
	end calminstruction

else if COFF.MACHINE = IMAGE_FILE_MACHINE_AMD64

	calminstruction dword? value
		compute value, value
		check	~ value relativeto 0 & value relativeto 1 elementof value & 1 elementof (1 metadataof value) relativeto COFF.relocatable
		jyes	addr32
		check	~ value relativeto 0 & (value + COFF.IMAGE_BASE) relativeto 1 elementof (value + COFF.IMAGE_BASE) & 1 elementof (1 metadataof (value + COFF.IMAGE_BASE)) relativeto COFF.relocatable
		jyes	addr32nb
		check	~ value relativeto 0 & (value + COFF.SECTION_BASE) relativeto 1 elementof (value + COFF.SECTION_BASE)
		jno	plain
		check	1 elementof (1 metadataof (value + COFF.SECTION_BASE)) relativeto COFF.relocatable
		jyes	rel32
	    plain:
		asm	emit 4: value
		exit
		local	offset, symndx, type
	    addr32:
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_AMD64_ADDR32
		jump	add_relocation
	    addr32nb:
		compute value, value + COFF.IMAGE_BASE
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_AMD64_ADDR32NB
		jump	add_relocation
	    rel32:
		compute value, value + (COFF.SECTION_BASE + $% + 4 - COFF.SECTION_OFFSET)
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_AMD64_REL32
		jump	add_relocation
	    add_relocation:
		compute offset, $%
		asm	emit 4: 0 scaleof value
		check	$% > offset
		jno	done
		compute offset, offset - COFF.SECTION_OFFSET
		local	relocation
		compute relocation, COFF.RELOCATION_INDEX * sizeof IMAGE_RELOCATION
		asm	store offset at COFF.relocations : relocation + IMAGE_RELOCATION.VirtualAddress
		asm	store symndx at COFF.relocations : relocation + IMAGE_RELOCATION.SymbolTableIndex
		asm	store type at COFF.relocations : relocation + IMAGE_RELOCATION.Type
		compute COFF.RELOCATION_INDEX, COFF.RELOCATION_INDEX + 1
	    done:
	end calminstruction

	calminstruction qword? value
		compute value, value
		check	~ value relativeto 0 & value relativeto 1 elementof value & 1 elementof (1 metadataof value) relativeto COFF.relocatable
		jyes	addr64
		check	~ value relativeto 0 & (value + COFF.IMAGE_BASE) relativeto 1 elementof (value + COFF.IMAGE_BASE) & 1 elementof (1 metadataof (value + COFF.IMAGE_BASE)) relativeto COFF.relocatable
		jyes	addr64nb
	    plain:
		asm	emit 8: value
		exit
		local	offset, symndx, type
	    addr64:
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_AMD64_ADDR64
		jump	add_relocation
	    addr64nb:
		compute value, value + COFF.IMAGE_BASE
		compute symndx, 0 scaleof (1 metadataof value)
		compute type, IMAGE_REL_AMD64_ADDR64NB
		jump	add_relocation
	    add_relocation:
		compute offset, $%
		asm	emit 8: 0 scaleof value
		check	$% > offset
		jno	done
		compute offset, offset - COFF.SECTION_OFFSET
		local	relocation
		compute relocation, COFF.RELOCATION_INDEX * sizeof IMAGE_RELOCATION
		asm	store offset at COFF.relocations : relocation + IMAGE_RELOCATION.VirtualAddress
		asm	store symndx at COFF.relocations : relocation + IMAGE_RELOCATION.SymbolTableIndex
		asm	store type at COFF.relocations : relocation + IMAGE_RELOCATION.Type
		compute COFF.RELOCATION_INDEX, COFF.RELOCATION_INDEX + 1
	    done:
	end calminstruction

end if

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

postpone
	purge section?
	section
	namespace COFF

		NUMBER_OF_SECTIONS := SECTION_INDEX
		STRING_TABLE_SIZE := STRING_POSITION
		NUMBER_OF_SYMBOLS := SYMBOL_INDEX
		NUMBER_OF_RELOCATIONS := RELOCATION_INDEX

		RELOCATIONS_OFFSET = $%
		load byte_sequence : NUMBER_OF_RELOCATIONS * sizeof IMAGE_RELOCATION from relocations:0
		db byte_sequence

		SYMBOL_TABLE_OFFSET = $%
		load byte_sequence : NUMBER_OF_SYMBOLS * sizeof IMAGE_SYMBOL from symbol_table:0
		db byte_sequence

		load byte_sequence : STRING_TABLE_SIZE from string_table:0
		db byte_sequence

	end namespace
end postpone
