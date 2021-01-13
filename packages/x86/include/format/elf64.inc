
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

struct Elf64_Shdr
	sh_name 	dd ?
	sh_type 	dd ?
	sh_flags	dq ?
	sh_addr 	dq ?
	sh_offset	dq ?
	sh_size 	dq ?
	sh_link 	dd ?
	sh_info 	dd ?
	sh_addralign	dq ?
	sh_entsize	dq ?
end struct

struct Elf64_Sym
	st_name 	dd ?
	st_info 	db ?
	st_other	db ?
	st_shndx	dw ?
	st_value	dq ?
	st_size 	dq ?
end struct

struct Elf64_Rel
	r_offset	dq ?
	r_info		dq ?
end struct

struct Elf64_Rela
	r_offset	dq ?
	r_info		dq ?
	r_addend	dq ?
end struct

struct Elf64_Phdr
	p_type		dd ?
	p_flags 	dd ?
	p_offset	dq ?
	p_vaddr 	dq ?
	p_paddr 	dq ?
	p_filesz	dq ?
	p_memsz 	dq ?
	p_align 	dq ?
end struct

purge struct?

ELFCLASSNONE = 0
ELFCLASS32   = 1
ELFCLASS64   = 2

ELFDATANONE = 0
ELFDATA2LSB = 1
ELFDATA2MSB = 2

ELFOSABI_NONE	  = 0
ELFOSABI_HPUX	  = 1
ELFOSABI_NETBSD   = 2
ELFOSABI_GNU	  = 3
ELFOSABI_LINUX	  = 3
ELFOSABI_SOLARIS  = 6
ELFOSABI_AIX	  = 7
ELFOSABI_IRIX	  = 8
ELFOSABI_FREEBSD  = 9
ELFOSABI_TRU64	  = 10
ELFOSABI_MODESTO  = 11
ELFOSABI_OPENBSD  = 12
ELFOSABI_OPENVMS  = 13
ELFOSABI_NSK	  = 14
ELFOSABI_AROS	  = 15
ELFOSABI_FENIXOS  = 16
ELFOSABI_CLOUDABI = 17
ELFOSABI_OPENVOS  = 18

ET_NONE   = 0
ET_REL	  = 1
ET_EXEC   = 2
ET_DYN	  = 3
ET_CORE   = 4
ET_LOPROC = 0xff00
ET_HIPROC = 0xffff

EM_NONE   = 0
EM_IA_64  = 50
EM_X86_64 = 62

EV_NONE    = 0
EV_CURRENT = 1

SHN_UNDEF     = 0
SHN_LORESERVE = 0xff00
SHN_LOPROC    = 0xff00
SHN_HIPROC    = 0xff1f
SHN_ABS       = 0xfff1
SHN_COMMON    = 0xfff2
SHN_HIRESERVE = 0xffff

SHT_NULL	= 0
SHT_PROGBITS	= 1
SHT_SYMTAB	= 2
SHT_STRTAB	= 3
SHT_RELA	= 4
SHT_HASH	= 5
SHT_DYNAMIC	= 6
SHT_NOTE	= 7
SHT_NOBITS	= 8
SHT_REL 	= 9
SHT_SHLIB	= 10
SHT_DYNSYM	= 11
SHT_LOPROC	= 0x70000000
SHT_HIPROC	= 0x7fffffff
SHT_LOUSER	= 0x80000000
SHT_HIUSER	= 0xffffffff

SHF_WRITE	= 0x1
SHF_ALLOC	= 0x2
SHF_EXECINSTR	= 0x4
SHF_MASKPROC	= 0xf0000000

STT_NOTYPE	= 0
STT_OBJECT	= 1
STT_FUNC	= 2
STT_SECTION	= 3
STT_FILE	= 4
STT_LOPROC	= 13
STT_HIPROC	= 15

STB_LOCAL	= 0
STB_GLOBAL	= 1
STB_WEAK	= 2
STB_LOPROC	= 13
STB_HIPROC	= 15

R_X86_64_NONE	   = 0
R_X86_64_64	   = 1
R_X86_64_PC32	   = 2
R_X86_64_GOT32	   = 3
R_X86_64_PLT32	   = 4
R_X86_64_COPY	   = 5
R_X86_64_GLOB_DAT  = 6
R_X86_64_JUMP_SLOT = 7
R_X86_64_RELATIVE  = 8
R_X86_64_GOTPCREL  = 9
R_X86_64_32	   = 10
R_X86_64_32S	   = 11
R_X86_64_16	   = 12
R_X86_64_PC16	   = 13
R_X86_64_8	   = 14
R_X86_64_PC8	   = 15
R_X86_64_DPTMOD64  = 16
R_X86_64_DTPOFF64  = 17
R_X86_64_TPOFF64   = 18
R_X86_64_TLSGD	   = 19
R_X86_64_TLSLD	   = 20
R_X86_64_DTPOFF32  = 21
R_X86_64_GOTTPOFF  = 22
R_X86_64_TPOFF32   = 23
R_X86_64_PC64	   = 24
R_X86_64_GOTOFF64  = 25
R_X86_64_GOTPC32   = 26

ELF::

namespace ELF

	if defined Settings.Machine
		MACHINE := Settings.Machine
	else
		MACHINE := EM_X86_64
	end if

	if defined Settings.ABI
		ABI := Settings.ABI
	else
		ABI := ELFOSABI_NONE
	end if

	if MACHINE = EM_X86_64
		R_64 = R_X86_64_64
		R_32 = R_X86_64_32S
		R_PC32 = R_X86_64_PC32
		R_PLT32 = R_X86_64_PLT32
	end if

	Header:

	e_ident 	db 0x7F,'ELF',ELFCLASS64,ELFDATA2LSB,EV_CURRENT,ABI,(16-$) dup 0
	e_type		dw ET_REL
	e_machine	dw MACHINE
	e_version	dd EV_CURRENT
	e_entry 	dq 0
	e_phoff 	dq 0
	e_shoff 	dq SECTION_TABLE_OFFSET
	e_flags 	dd 0
	e_ehsize	dw Content
	e_phentsize	dw 0
	e_phnum 	dw 0
	e_shentsize	dw sizeof Elf64_Shdr
	e_shnum 	dw NUMBER_OF_SECTIONS
	e_shstrndx	dw STRING_TABLE_SECTION_INDEX

	Content:

	virtual at 0
		section_table:: rb NUMBER_OF_SECTIONS * sizeof Elf64_Shdr
	end virtual

	virtual at 0
		symbol_table:: rb NUMBER_OF_SYMBOLS * sizeof Elf64_Sym
	end virtual

	virtual at 0
		string_table::
		_null db 0
		_symtab db '.symtab',0
		_strtab db '.strtab',0
		SECTION_NAME_POSITION = $
		rb SECTION_NAME_TABLE_SIZE - $
		STRING_POSITION = $
		rb STRING_TABLE_SIZE - $
	end virtual

	virtual at 0
		relocations:: rb NUMBER_OF_RELOCATIONS * sizeof Elf64_Rela
	end virtual

	element relocatable?

	macro section_org
		local sym
		element sym : relocatable * SECTION_INDEX + SECTION_SYMBOL_INDEX
		SECTION_BASE = sym
		org sym
	end macro

	RELOCATION_INDEX = 0
	SECTION_INDEX = 1
	SECTION_SYMBOL_INDEX = SECTION_INDEX
	SECTION_RELOCATION_INDEX = RELOCATION_INDEX
	SYMBOL_INDEX = NUMBER_OF_SECTION_SYMBOLS

	SECTION_OFFSET = $%
	SECTION_ALIGN = 8
	SECTION_NAME = '.flat'
	SECTION_FLAGS = SHF_ALLOC + SHF_WRITE + SHF_EXECINSTR
	DEFINED_SECTION = 0
	section_org

end namespace

macro section?
	namespace ELF

		SECTION_SIZE = $% - SECTION_OFFSET

		if DEFINED_SECTION | SECTION_SIZE > 0

			store SECTION_OFFSET at section_table : Elf64_Shdr.sh_offset + SECTION_INDEX * sizeof Elf64_Shdr
			store SECTION_SIZE at section_table : Elf64_Shdr.sh_size + SECTION_INDEX * sizeof Elf64_Shdr
			store SECTION_ALIGN at section_table : Elf64_Shdr.sh_addralign + SECTION_INDEX * sizeof Elf64_Shdr
			store SECTION_FLAGS at section_table : Elf64_Shdr.sh_flags + SECTION_INDEX * sizeof Elf64_Shdr

			if $%% = SECTION_OFFSET
				store SHT_NOBITS at section_table : Elf64_Shdr.sh_type + SECTION_INDEX * sizeof Elf64_Shdr
				section $
			else
				store SHT_PROGBITS at section_table : Elf64_Shdr.sh_type + SECTION_INDEX * sizeof Elf64_Shdr
				UNINITIALIZED_LENGTH = $% - $%%
				section $
				db UNINITIALIZED_LENGTH dup 0
			end if

			store SECTION_INDEX at symbol_table : Elf64_Sym.st_shndx + SECTION_SYMBOL_INDEX * sizeof Elf64_Sym
			store STT_SECTION + STB_LOCAL shl 4 at symbol_table : Elf64_Sym.st_info + SECTION_SYMBOL_INDEX * sizeof Elf64_Sym

			if RELOCATION_INDEX > SECTION_RELOCATION_INDEX

				store RELOCATIONS_OFFSET + SECTION_RELOCATION_INDEX * sizeof Elf64_Rela at section_table : Elf64_Shdr.sh_offset + (SECTION_INDEX+1) * sizeof Elf64_Shdr
				store (RELOCATION_INDEX - SECTION_RELOCATION_INDEX) * sizeof Elf64_Rela at section_table : Elf64_Shdr.sh_size + (SECTION_INDEX+1) * sizeof Elf64_Shdr
				store SHT_RELA at section_table : Elf64_Shdr.sh_type + (SECTION_INDEX+1) * sizeof Elf64_Shdr
				store SYMBOL_TABLE_SECTION_INDEX at section_table : Elf64_Shdr.sh_link + (SECTION_INDEX+1) * sizeof Elf64_Shdr
				store SECTION_INDEX at section_table : Elf64_Shdr.sh_info + (SECTION_INDEX+1) * sizeof Elf64_Shdr
				store sizeof Elf64_Rela at section_table : Elf64_Shdr.sh_entsize + (SECTION_INDEX+1) * sizeof Elf64_Shdr
				store 8 at section_table : Elf64_Shdr.sh_addralign + (SECTION_INDEX+1) * sizeof Elf64_Shdr

				store SECTION_NAME_POSITION at section_table : Elf64_Shdr.sh_name + (SECTION_INDEX+1) * sizeof Elf64_Shdr
				store SECTION_NAME_POSITION + 5 at section_table : Elf64_Shdr.sh_name + SECTION_INDEX * sizeof Elf64_Shdr
				store SECTION_NAME_POSITION + 5 at symbol_table : Elf64_Sym.st_name + SECTION_SYMBOL_INDEX * sizeof Elf64_Sym
				store '.rela' + SECTION_NAME shl (5*8) : 5 + lengthof (string SECTION_NAME) at string_table:SECTION_NAME_POSITION
				SECTION_NAME_POSITION = SECTION_NAME_POSITION + 5 + lengthof (string SECTION_NAME) + 1

				SECTION_INDEX = SECTION_INDEX + 2
				SECTION_SYMBOL_INDEX = SECTION_SYMBOL_INDEX + 1

			else
				store SECTION_NAME_POSITION at section_table : Elf64_Shdr.sh_name + SECTION_INDEX * sizeof Elf64_Shdr
				store SECTION_NAME_POSITION at symbol_table : Elf64_Sym.st_name + SECTION_SYMBOL_INDEX * sizeof Elf64_Sym
				store SECTION_NAME : lengthof (string SECTION_NAME) at string_table:SECTION_NAME_POSITION
				SECTION_NAME_POSITION = SECTION_NAME_POSITION + lengthof (string SECTION_NAME) + 1

				SECTION_INDEX = SECTION_INDEX + 1
				SECTION_SYMBOL_INDEX = SECTION_SYMBOL_INDEX + 1

			end if

		end if

	end namespace
end macro

macro section? declaration*
	namespace ELF

		section

		DEFINED_SECTION = 1
		SECTION_FLAGS = SHF_ALLOC
		SECTION_OFFSET = $%
		SECTION_ALIGN = 8

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
				match =writeable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or SHF_WRITE
				else match =executable?, attribute
					SECTION_FLAGS = SECTION_FLAGS or SHF_EXECINSTR
				else
					err 'unknown attribute "',`attribute,'"'
				end match
			end irpv

		else

			SECTION_NAME = declaration

		end match

		section_org

		SECTION_RELOCATION_INDEX = RELOCATION_INDEX

	end namespace
end macro

calminstruction align? boundary,value:?
	check	ELF.SECTION_ALIGN mod (boundary) = 0
	jyes	allowed
	arrange value, =err 'section not aligned enough'
	assemble value
	exit
    allowed:
	compute boundary, (boundary-1)-($-ELF.SECTION_BASE+boundary-1) mod boundary
	arrange value, =db boundary =dup value
	assemble value
end calminstruction

macro public? declaration*
	namespace ELF
		match value =as? str, declaration
			SYMBOL_VALUE = value
			SYMBOL_SIZE = sizeof value
			SYMBOL_NAME = string str
		else
			SYMBOL_VALUE = declaration
			SYMBOL_SIZE = sizeof declaration
			SYMBOL_NAME = `declaration
		end match
		if SYMBOL_VALUE relativeto 1 elementof SYMBOL_VALUE & 1 elementof (1 metadataof SYMBOL_VALUE) relativeto relocatable & 1 scaleof (1 metadataof SYMBOL_VALUE) > 0
			SYMBOL_SECTION_INDEX = 1 scaleof (1 metadataof SYMBOL_VALUE)
			SYMBOL_VALUE = SYMBOL_VALUE - 1 elementof SYMBOL_VALUE
		else
			SYMBOL_SECTION_INDEX = SHN_ABS
		end if
		store STRING_POSITION at symbol_table : Elf64_Sym.st_name + SYMBOL_INDEX * sizeof Elf64_Sym
		store SYMBOL_NAME : lengthof SYMBOL_NAME at string_table:STRING_POSITION
		STRING_POSITION = STRING_POSITION + lengthof SYMBOL_NAME + 1
		store SYMBOL_VALUE at symbol_table : Elf64_Sym.st_value + SYMBOL_INDEX * sizeof Elf64_Sym
		store SYMBOL_SIZE at symbol_table : Elf64_Sym.st_size + SYMBOL_INDEX * sizeof Elf64_Sym
		store SYMBOL_SECTION_INDEX at symbol_table : Elf64_Sym.st_shndx + SYMBOL_INDEX * sizeof Elf64_Sym
		if SYMBOL_SIZE
			store STT_OBJECT + STB_GLOBAL shl 4 at symbol_table : Elf64_Sym.st_info + SYMBOL_INDEX * sizeof Elf64_Sym
		else
			store STT_FUNC + STB_GLOBAL shl 4 at symbol_table : Elf64_Sym.st_info + SYMBOL_INDEX * sizeof Elf64_Sym
		end if
		SYMBOL_INDEX = SYMBOL_INDEX + 1
	end namespace
end macro

macro extrn? declaration*
	namespace ELF
		local sym,psym
		element sym : relocatable * (-1) + SYMBOL_INDEX
		element psym : PLT + SYMBOL_INDEX
		match str =as? name:size, declaration
			label name:size at sym
			label PLT.name at psym
			SYMBOL_NAME = string str
			SYMBOL_SIZE = size
		else match name:size, declaration
			label name:size at sym
			label PLT.name at psym
			SYMBOL_NAME = `name
			SYMBOL_SIZE = size
		else match str =as? name, declaration
			label name at sym
			label PLT.name at psym
			SYMBOL_NAME = string str
			SYMBOL_SIZE = 0
		else
			label declaration at sym
			label PLT.declaration at psym
			SYMBOL_NAME = `declaration
			SYMBOL_SIZE = 0
		end match
		store STRING_POSITION at symbol_table : Elf64_Sym.st_name + SYMBOL_INDEX * sizeof Elf64_Sym
		store SYMBOL_NAME : lengthof SYMBOL_NAME at string_table:STRING_POSITION
		STRING_POSITION = STRING_POSITION + lengthof SYMBOL_NAME + 1
		store SYMBOL_SIZE at symbol_table : Elf64_Sym.st_size + SYMBOL_INDEX * sizeof Elf64_Sym
		store STT_NOTYPE + STB_GLOBAL shl 4 at symbol_table : Elf64_Sym.st_info + SYMBOL_INDEX * sizeof Elf64_Sym
		SYMBOL_INDEX = SYMBOL_INDEX + 1
	end namespace
end macro

element PLT?

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
	check	~ value relativeto 0 & value relativeto 1 elementof value & 1 elementof (1 metadataof value) relativeto ELF.relocatable
	jyes	r_32
	check	~ value relativeto 0 & (value + ELF.SECTION_BASE) relativeto 1 elementof (value + ELF.SECTION_BASE)
	jno	plain
	check	1 elementof (1 metadataof (value + ELF.SECTION_BASE)) relativeto ELF.relocatable
	jyes	r_pc32
	check	1 elementof (1 metadataof (value + ELF.SECTION_BASE)) relativeto PLT
	jyes	r_plt32
    plain:
	asm	emit 4: value
	exit
	local	offset, addend, info
    r_32:
	compute offset, $%
	asm	emit 4: ?
	check	$% > offset
	jno	done
	compute offset, offset - ELF.SECTION_OFFSET
	compute addend, 0 scaleof value
	compute info, ELF.R_32 + (0 scaleof (1 metadataof value)) shl 32
	jump	add_relocation
    r_pc32:
	compute offset, $%
	asm	emit 4: ?
	check	$% > offset
	jno	done
	compute offset, offset - ELF.SECTION_OFFSET
	compute addend, 0 scaleof (value + ELF.SECTION_BASE + offset)
	compute info, ELF.R_PC32 + (0 scaleof (1 metadataof value)) shl 32
	jump	add_relocation
    r_plt32:
	compute offset, $%
	asm	emit 4: ?
	check	$% > offset
	jno	done
	compute offset, offset - ELF.SECTION_OFFSET
	compute addend, 0 scaleof (value + ELF.SECTION_BASE + offset)
	compute info, ELF.R_PLT32 + (0 scaleof (1 metadataof value)) shl 32
	jump	add_relocation
    add_relocation:
	local	Rela
	compute Rela, ELF.RELOCATION_INDEX * sizeof Elf64_Rela
	asm	store offset at ELF.relocations : Rela + Elf64_Rela.r_offset
	asm	store addend at ELF.relocations : Rela + Elf64_Rela.r_addend
	asm	store info at ELF.relocations : Rela + Elf64_Rela.r_info
	compute ELF.RELOCATION_INDEX, ELF.RELOCATION_INDEX + 1
    done:
end calminstruction

calminstruction qword? value
	compute value, value
	check	~ value relativeto 0 & value relativeto 1 elementof value & 1 elementof (1 metadataof value) relativeto ELF.relocatable
	jyes	r_64
    plain:
	asm	emit 8: value
	exit
	local	offset, addend, info
    r_64:
	compute offset, $%
	asm	emit 8: ?
	check	$% > offset
	jno	done
	compute offset, offset - ELF.SECTION_OFFSET
	compute addend, 0 scaleof value
	compute info, ELF.R_64 + (0 scaleof (1 metadataof value)) shl 32
    add_relocation:
	local	Rela
	compute Rela, ELF.RELOCATION_INDEX * sizeof Elf64_Rela
	asm	store offset at ELF.relocations : Rela + Elf64_Rela.r_offset
	asm	store addend at ELF.relocations : Rela + Elf64_Rela.r_addend
	asm	store info at ELF.relocations : Rela + Elf64_Rela.r_info
	compute ELF.RELOCATION_INDEX, ELF.RELOCATION_INDEX + 1
    done:
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

postpone
	purge section?
	section
	namespace ELF

		SECTION_NAME_TABLE_SIZE := SECTION_NAME_POSITION
		STRING_TABLE_SIZE := STRING_POSITION

		NUMBER_OF_SECTION_SYMBOLS := SECTION_SYMBOL_INDEX
		NUMBER_OF_SYMBOLS := SYMBOL_INDEX
		SYMBOL_TABLE_SIZE := NUMBER_OF_SYMBOLS * sizeof Elf64_Sym

		NUMBER_OF_RELOCATIONS := RELOCATION_INDEX
		rb (-$%) and 111b
		RELOCATIONS_OFFSET = $%
		load byte_sequence : NUMBER_OF_RELOCATIONS * sizeof Elf64_Rela from relocations:0
		db byte_sequence

		store _symtab at section_table : Elf64_Shdr.sh_name + SECTION_INDEX * sizeof Elf64_Shdr
		store $% at section_table : Elf64_Shdr.sh_offset + SECTION_INDEX * sizeof Elf64_Shdr
		store SYMBOL_TABLE_SIZE at section_table : Elf64_Shdr.sh_size + SECTION_INDEX * sizeof Elf64_Shdr
		store sizeof Elf64_Sym at section_table : Elf64_Shdr.sh_entsize + SECTION_INDEX * sizeof Elf64_Shdr
		store 8 at section_table : Elf64_Shdr.sh_addralign + SECTION_INDEX * sizeof Elf64_Shdr
		store SHT_SYMTAB at section_table : Elf64_Shdr.sh_type + SECTION_INDEX * sizeof Elf64_Shdr
		store STRING_TABLE_SECTION_INDEX at section_table : Elf64_Shdr.sh_link + SECTION_INDEX * sizeof Elf64_Shdr
		store NUMBER_OF_SECTION_SYMBOLS at section_table : Elf64_Shdr.sh_info + SECTION_INDEX * sizeof Elf64_Shdr
		SYMBOL_TABLE_SECTION_INDEX := SECTION_INDEX
		load byte_sequence : SYMBOL_TABLE_SIZE from symbol_table:0
		db byte_sequence
		SECTION_INDEX = SECTION_INDEX + 1

		store _strtab at section_table : Elf64_Shdr.sh_name + SECTION_INDEX * sizeof Elf64_Shdr
		store $% at section_table : Elf64_Shdr.sh_offset + SECTION_INDEX * sizeof Elf64_Shdr
		store STRING_TABLE_SIZE at section_table : Elf64_Shdr.sh_size + SECTION_INDEX * sizeof Elf64_Shdr
		store 1 at section_table : Elf64_Shdr.sh_addralign + SECTION_INDEX * sizeof Elf64_Shdr
		store SHT_STRTAB at section_table : Elf64_Shdr.sh_type + SECTION_INDEX * sizeof Elf64_Shdr
		STRING_TABLE_SECTION_INDEX := SECTION_INDEX
		load byte_sequence : STRING_TABLE_SIZE from string_table:0
		db byte_sequence
		SECTION_INDEX = SECTION_INDEX + 1

		assert SECTION_INDEX <= SHN_LORESERVE

		NUMBER_OF_SECTIONS := SECTION_INDEX
		rb (-$%) and 111b
		SECTION_TABLE_OFFSET := $%
		load byte_sequence : NUMBER_OF_SECTIONS * sizeof Elf64_Shdr from section_table:0
		db byte_sequence

	end namespace
end postpone
