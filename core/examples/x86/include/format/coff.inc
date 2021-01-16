
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

struct SCNHDR
	s_name		db 8 dup ?
	s_paddr 	dd ?
	s_vaddr 	dd ?
	s_size		dd ?
	s_scnptr	dd ?
	s_relptr	dd ?
	s_lnnoptr	dd ?
	s_nreloc	dw ?
	s_nlnno 	dw ?
	s_flags 	dd ?
end struct

SCNHSZ = sizeof SCNHDR

struct RELOC
	r_vaddr 	dd ?
	r_symndx	dd ?
	r_type		dw ?
end struct

RELSZ = sizeof RELOC

struct SYMENT
	e_name		db 8 dup ?
    virtual at e_name
	e_zeroes	dd ?
	e_offset	dd ?
    end virtual
	e_value 	dd ?
	e_scnum 	dw ?
	e_type		dw ?
	e_sclass	db ?
	e_numaux	db ?
end struct

SYMESZ = sizeof SYMENT

I386MAGIC = 0x14c

F_RELFLG = 0x0001
F_EXEC	 = 0x0002
F_LNNO	 = 0x0004
F_LSYMS  = 0x0008
F_AR32WR = 0x0100

STYP_TEXT = 0x0020
STYP_DATA = 0x0040
STYP_BSS  = 0x0080

N_UNDEF = 0
N_ABS	= -1
N_DEBUG = -2

T_NULL	 = 0000b
T_VOID	 = 0001b
T_CHAR	 = 0010b
T_SHORT  = 0011b
T_INT	 = 0100b
T_LONG	 = 0101b
T_FLOAT  = 0110b
T_DOUBLE = 0111b
T_STRUCT = 1000b
T_UNION  = 1001b
T_ENUM	 = 1010b
T_MOE	 = 1011b
T_UCHAR  = 1100b
T_USHORT = 1101b
T_UINT	 = 1110b
T_ULONG  = 1111b
T_LNGDBL = 01_0000b

DT_NON	 = 00b
DT_PTR	 = 01b
DT_FCN	 = 10b
DT_ARY	 = 11b

C_NULL	  = 0
C_AUTO	  = 1
C_EXT	  = 2
C_STAT	  = 3
C_REG	  = 4
C_EXTDEF  = 5
C_LABEL   = 6
C_ULABEL  = 7
C_MOS	  = 8
C_ARG	  = 9
C_STRTAG  = 10
C_MOU	  = 11
C_UNTAG   = 12
C_TPDEF   = 13
C_USTATIC = 14
C_ENTAG   = 15
C_MOE	  = 16
C_REGPARM = 17
C_FIELD   = 18
C_AUTOARG = 19
C_LASTENT = 20
C_BLOCK   = 100
C_FCN	  = 101
C_EOS	  = 102
C_FILE	  = 103
C_LINE	  = 104
C_ALIAS   = 105
C_HIDDEN  = 106
C_EFCN	  = 255

RELOC_ADDR32 = 6
RELOC_REL32  = 20

COFF::

namespace COFF

	Header:

	f_magic 	dw I386MAGIC
	f_nscns 	dw NUMBER_OF_SECTIONS
	f_timdat	dd __TIME__
	f_symptr	dd SYMBOL_TABLE_OFFSET
	f_nsyms 	dd NUMBER_OF_SYMBOLS
	f_opthdr	dw 0
	f_flags 	dw F_AR32WR + F_LNNO

	Sections:	db NUMBER_OF_SECTIONS * SCNHSZ dup 0

	virtual at 0
		symbol_table:: rb NUMBER_OF_SYMBOLS * SYMESZ
	end virtual

	virtual at 0
		string_table:: dd STRING_TABLE_SIZE
		STRING_POSITION = $
		rb STRING_TABLE_SIZE - $
	end virtual

	virtual at 0
		relocations:: rb NUMBER_OF_RELOCATIONS * RELSZ
	end virtual

	element relocatable?

	macro section_start
		local sym
		element sym : relocatable * (1+SECTION_INDEX) + SYMBOL_INDEX
		SECTION_BASE = sym
		org sym
		if DEFINED_SECTION | DEFAULT_SECTION_PRESENT
			store SECTION_NAME : 8 at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_name
			store C_STAT at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_sclass
			store 1+SECTION_INDEX at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_scnum
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
	SECTION_FLAGS = STYP_TEXT + STYP_DATA
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
				SECTION_FLAGS = SECTION_FLAGS or STYP_BSS
				SECTION_OFFSET = 0
				section $
			else
				UNINITIALIZED_LENGTH = $% - $%%
				section $
				db UNINITIALIZED_LENGTH dup 0
			end if

			store SECTION_NAME : 8 at COFF:Sections + SECTION_INDEX * SCNHSZ + SCNHDR.s_name
			store SECTION_OFFSET at COFF:Sections + SECTION_INDEX * SCNHSZ + SCNHDR.s_scnptr
			store SECTION_SIZE at COFF:Sections + SECTION_INDEX * SCNHSZ + SCNHDR.s_size
			store SECTION_FLAGS at COFF:Sections + SECTION_INDEX * SCNHSZ + SCNHDR.s_flags

			if RELOCATION_INDEX > SECTION_RELOCATION_INDEX
				store RELOCATION_INDEX - SECTION_RELOCATION_INDEX at COFF:Sections + SECTION_INDEX * SCNHSZ + SCNHDR.s_nreloc
				store RELOCATIONS_OFFSET + SECTION_RELOCATION_INDEX * RELSZ at COFF:Sections + SECTION_INDEX * SCNHSZ + SCNHDR.s_relptr
			end if

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
			define seq attributes
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
					SECTION_FLAGS = SECTION_FLAGS or STYP_TEXT
				else match =data?, attribute
					SECTION_FLAGS = SECTION_FLAGS or STYP_DATA
				else
					err 'unknown attribute "',`attribute,'"'
				end match
			end irpv

		else

			SECTION_NAME = declaration

		end match

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
			SYMBOL_CLASS = C_STAT
		else match value =as? str, declaration
			SYMBOL_VALUE = value
			SYMBOL_NAME = string str
			SYMBOL_CLASS = C_EXT
		else match =static? value, declaration
			SYMBOL_VALUE = value
			SYMBOL_NAME = `value
			SYMBOL_CLASS = C_STAT
		else
			SYMBOL_VALUE = declaration
			SYMBOL_NAME = `declaration
			SYMBOL_CLASS = C_EXT
		end match
		if SYMBOL_VALUE relativeto 1 elementof SYMBOL_VALUE & 1 elementof (1 metadataof SYMBOL_VALUE) relativeto relocatable & 1 scaleof (1 metadataof SYMBOL_VALUE) > 0
			SYMBOL_SECTION_INDEX = 1 scaleof (1 metadataof SYMBOL_VALUE)
			SYMBOL_VALUE = SYMBOL_VALUE - 1 elementof SYMBOL_VALUE
		else
			SYMBOL_SECTION_INDEX = N_ABS
		end if
		if lengthof SYMBOL_NAME > 8
			store 0 at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_zeroes
			store STRING_POSITION at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_offset
			store SYMBOL_NAME : lengthof SYMBOL_NAME at string_table:STRING_POSITION
			STRING_POSITION = STRING_POSITION + lengthof SYMBOL_NAME + 1
		else
			store SYMBOL_NAME : 8 at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_name
		end if
		store SYMBOL_VALUE at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_value
		store SYMBOL_SECTION_INDEX at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_scnum
		store SYMBOL_CLASS at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_sclass
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
			store 0 at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_zeroes
			store STRING_POSITION at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_offset
			store SYMBOL_NAME : lengthof SYMBOL_NAME at string_table:STRING_POSITION
			STRING_POSITION = STRING_POSITION + lengthof SYMBOL_NAME + 1
		else
			store SYMBOL_NAME : 8 at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_name
		end if
		store C_EXT at symbol_table : SYMBOL_INDEX * SYMESZ + SYMENT.e_sclass
		SYMBOL_INDEX = SYMBOL_INDEX + 1
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

calminstruction dword? value
	compute value, value
	check	~ value relativeto 0 & value relativeto 1 elementof value & 1 elementof (1 metadataof value) relativeto COFF.relocatable
	jyes	addr32
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
	compute type, RELOC_ADDR32
	jump	add_relocation
    rel32:
	compute value, value + COFF.SECTION_BASE
	compute symndx, 0 scaleof (1 metadataof value)
	compute type, RELOC_REL32
	jump	add_relocation
    add_relocation:
	compute offset, $%
	asm	emit 4: 0 scaleof value
	check	$% > offset
	jno	done
	compute offset, offset - COFF.SECTION_OFFSET
	local	reloc
	compute reloc, COFF.RELOCATION_INDEX * RELSZ
	asm	store offset at COFF.relocations : reloc + RELOC.r_vaddr
	asm	store symndx at COFF.relocations : reloc + RELOC.r_symndx
	asm	store type at COFF.relocations : reloc + RELOC.r_type
	compute COFF.RELOCATION_INDEX, COFF.RELOCATION_INDEX + 1
    done:
end calminstruction

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

postpone
	purge section?
	section
	namespace COFF

		NUMBER_OF_SECTIONS := SECTION_INDEX
		STRING_TABLE_SIZE := STRING_POSITION
		NUMBER_OF_SYMBOLS := SYMBOL_INDEX
		NUMBER_OF_RELOCATIONS := RELOCATION_INDEX

		RELOCATIONS_OFFSET = $%
		load byte_sequence : NUMBER_OF_RELOCATIONS * RELSZ from relocations:0
		db byte_sequence

		SYMBOL_TABLE_OFFSET = $%
		load byte_sequence : NUMBER_OF_SYMBOLS * SYMESZ from symbol_table:0
		db byte_sequence

		load byte_sequence : STRING_TABLE_SIZE from string_table:0
		db byte_sequence

	end namespace
end postpone
