
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
EM_M32	  = 1
EM_SPARC  = 2
EM_386	  = 3
EM_68K	  = 4
EM_88K	  = 5
EM_860	  = 7
EM_MIPS   = 8
EM_X86_64 = 62

EV_NONE    = 0
EV_CURRENT = 1

PT_NULL 	= 0
PT_LOAD 	= 1
PT_DYNAMIC	= 2
PT_INTERP	= 3
PT_NOTE 	= 4
PT_SHLIB	= 5
PT_PHDR 	= 6
PT_GNU_EH_FRAME = 0x6474e550
PT_GNU_STACK	= 0x6474e551
PT_GNU_RELRO	= 0x6474e552
PT_LOPROC	= 0x70000000
PT_HIPROC	= 0x7fffffff

PF_X	    = 1
PF_W	    = 2
PF_R	    = 4
PF_MASKOS   = 0x0ff00000
PF_MASKPROC = 0xf0000000

calminstruction align? boundary,value:?
	compute boundary, (boundary-1)-($-ELF.DYNAMIC+boundary-1) mod boundary
	arrange value, =db boundary =dup value
	assemble value
end calminstruction

ELF::

namespace ELF

	if defined Settings.Class
		CLASS := Settings.Class
	else
		CLASS := ELFCLASS32
	end if

	if defined Settings.Type
		TYPE := Settings.Type
	else
		TYPE := ET_EXEC
	end if

	if defined Settings.Machine
		MACHINE := Settings.Machine
	else
		MACHINE := EM_386
	end if

	if defined Settings.ABI
		ABI := Settings.ABI
	else
		ABI := ELFOSABI_NONE
	end if

	if TYPE = ET_DYN
		element DYNAMIC
	else
		DYNAMIC := 0
	end if

	if defined Settings.BaseAddress
		BASE_ADDRESS := DYNAMIC + Settings.BaseAddress
	else
		BASE_ADDRESS := DYNAMIC + 8048000h
	end if

	if defined Settings.LoadHeaders
		LOAD_HEADERS := Settings.LoadHeaders 
	else  
		LOAD_HEADERS := 1
	end if

	Header:

	e_ident 	db 0x7F,'ELF',CLASS,ELFDATA2LSB,EV_CURRENT,ABI,(16-$) dup 0
	e_type		dw TYPE
	e_machine	dw MACHINE
	e_version	dd EV_CURRENT
	if CLASS <> ELFCLASS64
	 e_entry	dd start - DYNAMIC
	 e_phoff	dd ProgramHeader
	 e_shoff	dd 0
	 e_flags	dd 0
	 e_ehsize	dw ProgramHeader
	 e_phentsize	dw SEGMENT_HEADER_LENGTH
	 e_phnum	dw NUMBER_OF_SEGMENTS
	 e_shentsize	dw 28h
	 e_shnum	dw 0
	 e_shstrndx	dw 0
	else
	 e_entry	dq start - DYNAMIC
	 e_phoff	dq ProgramHeader
	 e_shoff	dq 0
	 e_flags	dd 0
	 e_ehsize	dw ProgramHeader
	 e_phentsize	dw SEGMENT_HEADER_LENGTH
	 e_phnum	dw NUMBER_OF_SEGMENTS
	 e_shentsize	dw 40h
	 e_shnum	dw 0
	 e_shstrndx	dw 0
	end if

	ProgramHeader:
	if CLASS <> ELFCLASS64
	 p_type 	dd PT_LOAD
	 p_offset	dd 0
	 p_vaddr	dd BASE_ADDRESS - DYNAMIC
	 p_paddr	dd BASE_ADDRESS - DYNAMIC
	 p_filesz	dd 0
	 p_memsz	dd 0
	 p_flags	dd PF_R+PF_W+PF_X
	 p_align	dd 1000h
	else
	 p_type 	dd PT_LOAD
	 p_flags	dd PF_R+PF_W+PF_X
	 p_offset	dq 0
	 p_vaddr	dq BASE_ADDRESS - DYNAMIC
	 p_paddr	dq BASE_ADDRESS - DYNAMIC
	 p_filesz	dq 0
	 p_memsz	dq 0
	 p_align	dq 1000h
	end if

	SEGMENT_HEADER_LENGTH = $ - ProgramHeader

	db (NUMBER_OF_SEGMENTS-1)*SEGMENT_HEADER_LENGTH dup 0

	HEADERS_OFFSET = 0
	HEADERS_BASE = BASE_ADDRESS
	OVERLAY_HEADERS = LOAD_HEADERS

	SEGMENT_INDEX = 0
	DEFINED_SEGMENT = 0
	SEGMENT_TYPE = PT_LOAD
	FILE_OFFSET = $%
	SEGMENT_BASE = BASE_ADDRESS + FILE_OFFSET and 0FFFh
	org SEGMENT_BASE
	start:

end namespace

RVA? equ -ELF.BASE_ADDRESS +

macro entry? address*
	namespace ELF
		store address-DYNAMIC at ELF:e_entry
	end namespace
end macro

macro segment?
	namespace ELF

		DEFINED_SEGMENT_SIZE = $ - SEGMENT_BASE

		if (DEFINED_SEGMENT | DEFINED_SEGMENT_SIZE > 0) & SEGMENT_TYPE = PT_LOAD & OVERLAY_HEADERS
			FILE_OFFSET = HEADERS_OFFSET
			SEGMENT_BASE = HEADERS_BASE
			OVERLAY_HEADERS = 0
		end if

		store SEGMENT_BASE-DYNAMIC at ELF:p_vaddr+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH
		store SEGMENT_BASE-DYNAMIC at ELF:p_paddr+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH
		store FILE_OFFSET at ELF:p_offset+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH

		if SEGMENT_TYPE = PT_LOAD

			RAW_DATA_SIZE = $%% - FILE_OFFSET
			SEGMENT_SIZE = $ - SEGMENT_BASE
			store RAW_DATA_SIZE at ELF:p_filesz+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH
			store SEGMENT_SIZE at ELF:p_memsz+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH

			if DEFINED_SEGMENT | DEFINED_SEGMENT_SIZE > 0
				FILE_OFFSET = $%%
				align 1000h
				SEGMENT_BASE = $ + FILE_OFFSET and 0FFFh
			end if

			section SEGMENT_BASE

		else

			if OVERLAY_HEADERS = 0 & ( LOAD_HEADERS | SEGMENT_TYPE = PT_GNU_RELRO ) & ~ SEGMENT_TYPE = PT_GNU_STACK & ~ SEGMENT_TYPE = PT_NOTE
				OVERLAY_HEADERS = 1
				HEADERS_OFFSET = FILE_OFFSET
				HEADERS_BASE = SEGMENT_BASE
			end if

			FILE_OFFSET = $%
			SEGMENT_SIZE = $ - SEGMENT_BASE
			store SEGMENT_SIZE at ELF:p_filesz+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH
			store SEGMENT_SIZE at ELF:p_memsz+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH

			if OVERLAY_HEADERS = 0
				SEGMENT_BASE = DYNAMIC + (SEGMENT_BASE-DYNAMIC) and not 0FFFh + FILE_OFFSET and 0FFFh
			else
				SEGMENT_BASE = $
			end if

			if $% > $%%
				store 0:byte at $-1  
			end if
			org SEGMENT_BASE

		end if

		if DEFINED_SEGMENT | DEFINED_SEGMENT_SIZE > 0
			SEGMENT_INDEX = SEGMENT_INDEX + 1
		end if

	end namespace
end macro

macro segment? attributes*
	namespace ELF

		segment

		SEGMENT_TYPE = PT_LOAD
		SEGMENT_FLAGS = 0

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
			match =readable?, attribute
				SEGMENT_FLAGS = SEGMENT_FLAGS or PF_R
			else match =writeable?, attribute
				SEGMENT_FLAGS = SEGMENT_FLAGS or PF_W
			else match =executable?, attribute
				SEGMENT_FLAGS = SEGMENT_FLAGS or PF_X
			else match =interpreter?, attribute
				SEGMENT_TYPE = PT_INTERP
			else match =dynamic?, attribute
				SEGMENT_TYPE = PT_DYNAMIC
			else match =note?, attribute
				SEGMENT_TYPE = PT_NOTE
			else match =gnustack?, attribute
				SEGMENT_TYPE = PT_GNU_STACK
			else match =gnuehframe?, attribute
				SEGMENT_TYPE = PT_GNU_EH_FRAME
			else match =gnurelro?, attribute
				SEGMENT_TYPE = PT_GNU_RELRO
			else
				err 'unknown attribute "',`attribute,'"'
			end match
		end irpv

		DEFINED_SEGMENT = 1

		store SEGMENT_TYPE at ELF:p_type+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH
		store SEGMENT_FLAGS at ELF:p_flags+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH

		if SEGMENT_TYPE = PT_LOAD
			store 1000h at ELF:p_align+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH
		else
			store 1 at ELF:p_align+SEGMENT_INDEX*SEGMENT_HEADER_LENGTH
		end if

	end namespace
end macro

postpone
	purge segment?
	segment
	namespace ELF
		NUMBER_OF_SEGMENTS := SEGMENT_INDEX
	end namespace
end postpone
