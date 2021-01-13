
CPU_TYPE_ANY		= -1
CPU_ARCH_ABI64		= 0x1000000
CPU_TYPE_VAX		= 1
CPU_TYPE_ROMP		= 2
CPU_TYPE_NS32032	= 4
CPU_TYPE_NS32332	= 5
CPU_TYPE_MC680x0	= 6
CPU_TYPE_I386		= 7
CPU_TYPE_X86_64 	= CPU_TYPE_I386 or CPU_ARCH_ABI64
CPU_TYPE_MIPS		= 8
CPU_TYPE_NS32532	= 9
CPU_TYPE_HPPA		= 11
CPU_TYPE_ARM		= 12
CPU_TYPE_MC88000	= 13
CPU_TYPE_SPARC		= 14
CPU_TYPE_I860		= 15
CPU_TYPE_I860_LITTLE	= 16
CPU_TYPE_RS6000 	= 17
CPU_TYPE_MC98000	= 18
CPU_TYPE_POWERPC	= 18
CPU_TYPE_POWERPC64	= CPU_TYPE_POWERPC or CPU_ARCH_ABI64
CPU_TYPE_VEO		= 255

CPU_SUBTYPE_MASK	= 0xff000000
CPU_SUBTYPE_LIB64	= 0x80000000

CPU_SUBTYPE_I386_ALL	= 3
CPU_SUBTYPE_X86_64_ALL	= CPU_SUBTYPE_I386_ALL
CPU_SUBTYPE_386 	= 3
CPU_SUBTYPE_486 	= 4
CPU_SUBTYPE_486SX	= 4 + 128
CPU_SUBTYPE_586 	= 5
CPU_SUBTYPE_PENT	= 5 + 0 shl 4
CPU_SUBTYPE_PENTPRO	= 6 + 1 shl 4
CPU_SUBTYPE_PENTII_M3	= 6 + 3 shl 4
CPU_SUBTYPE_PENTII_M5	= 6 + 5 shl 4
CPU_SUBTYPE_PENTIUM_4	= 10 + 0 shl 4

MH_OBJECT      = 0x1
MH_EXECUTE     = 0x2
MH_FVMLIB      = 0x3
MH_CORE        = 0x4
MH_PRELOAD     = 0x5
MH_DYLIB       = 0x6
MH_DYLINKER    = 0x7
MH_BUNDLE      = 0x8
MH_DYLIB_STUB  = 0x9
MH_DSYM        = 0xA
MH_KEXT_BUNDLE = 0xB

MH_NOUNDEFS		   = 0x1
MH_INCRLINK		   = 0x2
MH_DYLDLINK		   = 0x4
MH_BINDATLOAD		   = 0x8
MH_PREBOUND		   = 0x10
MH_SPLIT_SEGS		   = 0x20
MH_LAZY_INIT		   = 0x40
MH_TWOLEVEL		   = 0x80
MH_FORCE_FLAT		   = 0x100
MH_NOMULTIDEFS		   = 0x200
MH_NOFIXPREBINDING	   = 0x400
MH_PREBINDABLE		   = 0x800
MH_ALLMODSBOUND 	   = 0x1000
MH_SUBSECTIONS_VIA_SYMBOLS = 0x2000
MH_CANONICAL		   = 0x4000
MH_WEAK_DEFINES 	   = 0x8000
MH_BINDS_TO_WEAK	   = 0x10000
MH_ALLOW_STACK_EXECUTION   = 0x20000
MH_ROOT_SAFE		   = 0x40000
MH_SETUID_SAFE		   = 0x80000
MH_NO_REEXPORTED_DYLIBS    = 0x100000
MH_PIE			   = 0x200000
MH_DEAD_STRIPPABLE_DYLIB   = 0x400000
MH_HAS_TLV_DESCRIPTORS	   = 0x800000
MH_NO_HEAP_EXECUTION	   = 0x1000000
MH_APP_EXTENSION_SAFE	   = 0x2000000

LC_REQ_DYLD	      = 0x80000000
LC_SEGMENT	      = 0x1
LC_SYMTAB	      = 0x2
LC_SYMSEG	      = 0x3
LC_THREAD	      = 0x4
LC_UNIXTHREAD	      = 0x5
LC_LOADFVMLIB	      = 0x6
LC_IDFVMLIB	      = 0x7
LC_IDENT	      = 0x8
LC_FVMFILE	      = 0x9
LC_PREPAGE	      = 0xa
LC_DYSYMTAB	      = 0xb
LC_LOAD_DYLIB	      = 0xc
LC_ID_DYLIB	      = 0xd
LC_LOAD_DYLINKER      = 0xe
LC_ID_DYLINKER	      = 0xf
LC_PREBOUND_DYLIB     = 0x10
LC_ROUTINES	      = 0x11
LC_SUB_FRAMEWORK      = 0x12
LC_SUB_UMBRELLA       = 0x13
LC_SUB_CLIENT	      = 0x14
LC_SUB_LIBRARY	      = 0x15
LC_TWOLEVEL_HINTS     = 0x16
LC_PREBIND_CKSUM      = 0x17
LC_LOAD_WEAK_DYLIB    = 0x18
LC_SEGMENT_64	      = 0x19
LC_ROUTINES_64	      = 0x1a
LC_UUID 	      = 0x1b
LC_RPATH	      = 0x1c + LC_REQ_DYLD
LC_CODE_SIGNATURE     = 0x1d
LC_SEGMENT_SPLIT_INFO = 0x1e
LC_REEXPORT_DYLIB     = 0x1f + LC_REQ_DYLD
LC_LAZY_LOAD_DYLIB    = 0x20
LC_ENCRYPTION_INFO    = 0x21
LC_DYLD_INFO	      = 0x22
LC_DYLD_INFO_ONLY     = 0x22 + LC_REQ_DYLD

SG_HIGHVM  = 0x1
SG_FVMLIB  = 0x2
SG_NORELOC = 0x4

SECTION_TYPE	   = 0x000000ff
SECTION_ATTRIBUTES = 0xffffff00

S_REGULAR			      = 0x0
S_ZEROFILL			      = 0x1
S_CSTRING_LITERALS		      = 0x2
S_4BYTE_LITERALS		      = 0x3
S_8BYTE_LITERALS		      = 0x4
S_LITERAL_POINTERS		      = 0x5
S_NON_LAZY_SYMBOL_POINTERS	      = 0x6
S_LAZY_SYMBOL_POINTERS		      = 0x7
S_SYMBOL_STUBS			      = 0x8
S_MOD_INIT_FUNC_POINTERS	      = 0x9
S_MOD_TERM_FUNC_POINTERS	      = 0x0a
S_COALESCED			      = 0x0b
S_GB_ZEROFILL			      = 0x0c
S_INTERPOSING			      = 0x0d
S_16BYTE_LITERALS		      = 0x0e
S_DTRACE_DOF			      = 0x0f
S_LAZY_DYLIB_SYMBOL_POINTERS	      = 0x10
S_THREAD_LOCAL_REGULAR		      = 0x11
S_THREAD_LOCAL_ZEROFILL 	      = 0x12
S_THREAD_LOCAL_VARIABLES	      = 0x13
S_THREAD_LOCAL_VARIABLE_POINTERS      = 0x14
S_THREAD_LOCAL_INIT_FUNCTION_POINTERS = 0x15

SECTION_ATTRIBUTES_USR	   = 0xff000000
S_ATTR_PURE_INSTRUCTIONS   = 0x80000000
S_ATTR_NO_TOC		   = 0x40000000
S_ATTR_STRIP_STATIC_SYMS   = 0x20000000
S_ATTR_NO_DEAD_STRIP	   = 0x10000000
S_ATTR_LIVE_SUPPORT	   = 0x08000000
S_ATTR_SELF_MODIFYING_CODE = 0x04000000
S_ATTR_DEBUG		   = 0x02000000

SECTION_ATTRIBUTES_SYS	   = 0x00ffff00
S_ATTR_SOME_INSTRUCTIONS   = 0x00000400
S_ATTR_EXT_RELOC	   = 0x00000200
S_ATTR_LOC_RELOC	   = 0x00000100

VM_PROT_NONE	   = 0x00
VM_PROT_READ	   = 0x01
VM_PROT_WRITE	   = 0x02
VM_PROT_EXECUTE    = 0x04
VM_PROT_DEFAULT    = VM_PROT_READ or VM_PROT_WRITE
VM_PROT_ALL	   = VM_PROT_READ or VM_PROT_WRITE or VM_PROT_EXECUTE
VM_PROT_NO_CHANGE  = 0x08
VM_PROT_COPY	   = 0x10
VM_PROT_WANTS_COPY = 0x10

x86_THREAD_STATE32    = 1
x86_FLOAT_STATE32     = 2
x86_EXCEPTION_STATE32 = 3
x86_THREAD_STATE64    = 4
x86_FLOAT_STATE64     = 5
x86_EXCEPTION_STATE64 = 6
x86_THREAD_STATE      = 7
x86_FLOAT_STATE       = 8
x86_EXCEPTION_STATE   = 9
x86_DEBUG_STATE32     = 10
x86_DEBUG_STATE64     = 11
x86_DEBUG_STATE       = 12
THREAD_STATE_NONE     = 13

N_STAB = 0xe0
N_PEXT = 0x10
N_TYPE = 0x0e
N_EXT  = 0x01
N_UNDF = 0x0
N_ABS  = 0x2
N_SECT = 0xe
N_PBUD = 0xc
N_INDR = 0xa

NO_SECT  = 0
MAX_SECT = 255

REFERENCE_TYPE				  = 0xf
REFERENCE_FLAG_UNDEFINED_NON_LAZY	  = 0
REFERENCE_FLAG_UNDEFINED_LAZY		  = 1
REFERENCE_FLAG_DEFINED			  = 2
REFERENCE_FLAG_PRIVATE_DEFINED		  = 3
REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY = 4
REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY	  = 5
REFERENCED_DYNAMICALLY			  = 0x0010

GENERIC_RELOC_VANILLA	     = 0
GENERIC_RELOC_PAIR	     = 1
GENERIC_RELOC_SECTDIFF	     = 2
GENERIC_RELOC_PB_LA_PTR      = 3
GENERIC_RELOC_LOCAL_SECTDIFF = 4
GENERIC_RELOC_TLV	     = 5

X86_64_RELOC_UNSIGNED = 0
X86_64_RELOC_SIGNED = 1
X86_64_RELOC_BRANCH = 2
X86_64_RELOC_GOT_LOAD = 3
X86_64_RELOC_GOT = 4
X86_64_RELOC_SUBTRACTOR = 5
X86_64_RELOC_SIGNED_1 = 6
X86_64_RELOC_SIGNED_2 = 7
X86_64_RELOC_SIGNED_4 = 8
X86_64_RELOC_TLV = 9

; Basic layer: command headers

MachO::

namespace MachO

	if defined Settings.ProcessorType
		CPUTYPE := Settings.ProcessorType
	else
		CPUTYPE := CPU_TYPE_I386
	end if

	if defined Settings.ProcessorSubtype
		CPUSUBTYPE := Settings.ProcessorSubtype
	else
		if CPUTYPE and CPU_ARCH_ABI64
			CPUSUBTYPE := CPU_SUBTYPE_I386_ALL + CPU_SUBTYPE_LIB64
		else
			CPUSUBTYPE := CPU_SUBTYPE_I386_ALL
		end if
	end if

	if defined Settings.FileType
		FILETYPE := Settings.FileType
	else
		FILETYPE := MH_EXECUTE
	end if

	if defined Settings.Flags
		FLAGS := Settings.Flags
	else if FILETYPE <> MH_OBJECT
		FLAGS := MH_NOUNDEFS + MH_DYLDLINK
	else
		FLAGS := 0
	end if

	if defined Settings.SegmentAlignment
		SEGMENT_ALIGNMENT := Settings.SegmentAlignment
	else
		SEGMENT_ALIGNMENT := 1000h
	end if

	if defined Settings.FileAlignment
		FILE_ALIGNMENT := Settings.FileAlignment
	else
		FILE_ALIGNMENT := 1000h
	end if

	if defined Settings.HeaderPad
		HEADER_PAD := Settings.HeaderPad
	else
		HEADER_PAD := 16
	end if

	if CPUTYPE and CPU_ARCH_ABI64
		magic		dd	0xFEEDFACF
	else
		magic		dd	0xFEEDFACE
	end if
		cputype 	dd	CPUTYPE
		cpusubtype	dd	CPUSUBTYPE
		filetype	dd	FILETYPE
		ncmds		dd	NUMBER_OF_COMMANDS
		sizeofcmds	dd	SIZE_OF_COMMANDS
		flags		dd	FLAGS
	if CPUTYPE and CPU_ARCH_ABI64
		reserved	dd	?
	end if

	COMMAND_NUMBER = 0
	COMMAND_POSITION = $

	commands db SIZE_OF_COMMANDS dup 0, HEADER_PAD dup ?

	org $

	macro command type
		if MachO.COMMAND_POSITION > 0
			virtual at MachO.COMMAND_POSITION
			MachO.COMMAND_POSITION = 0
		end if
		match t, type
			if MachO.COMMAND_NUMBER > 0
				repeat 1, p:MachO.COMMAND_NUMBER
					MachO.load#p.SIZE := $ - MachO.load#p.cmd
				end repeat
			end if
			MachO.COMMAND_NUMBER = MachO.COMMAND_NUMBER + 1
			MachO.COMMAND = $
			repeat 1, n:MachO.COMMAND_NUMBER
				namespace MachO.load#n
					cmd dd t
					cmdsize dd SIZE
				end namespace
			end repeat
		end match
	end macro

	macro text
		if MachO.COMMAND_POSITION = 0
			MachO.COMMAND_POSITION = $
			load MachO.COMMAND_DATA:$-$$ from $$
			store MachO.COMMAND_DATA:$-$$ at MachO:$$
			end virtual
		end if
	end macro

end namespace

calminstruction align? boundary,value:?
	compute boundary, (boundary-1)-($+boundary-1) mod boundary
	arrange value, =db boundary =dup value
	assemble value
end calminstruction

postpone
	MachO.text
	namespace MachO
		if COMMAND_NUMBER > 0
			repeat 1, p:COMMAND_NUMBER
				load#p.SIZE := COMMAND_POSITION - MachO.load#p.cmd
			end repeat
		end if
		NUMBER_OF_COMMANDS := COMMAND_NUMBER
		SIZE_OF_COMMANDS := COMMAND_POSITION - commands
	end namespace
end postpone

; Intermediate layer: segments and sections

namespace MachO

	if defined Settings.BaseAddress

		VM_ADDRESS = Settings.BaseAddress

		if CPUTYPE and CPU_ARCH_ABI64
			MachO.command LC_SEGMENT_64
		else
			MachO.command LC_SEGMENT
		end if
		namespace MachO.segment1
			segname        emit 16: '__PAGEZERO'
		if CPUTYPE and CPU_ARCH_ABI64
			vmaddr	       dq	0
			vmsize	       dq	VM_ADDRESS
			fileoff        dq	0
			filesize       dq	0
		else
			vmaddr	       dd	0
			vmsize	       dd	VM_ADDRESS
			fileoff        dd	0
			filesize       dd	0
		end if
			maxprot        dd	0
			initprot       dd	0
			nsects	       dd	0
			flags	       dd	0
		end namespace

		SEGMENT_NUMBER = 1
	else
		VM_ADDRESS = 0
		SEGMENT_NUMBER = 0
	end if

	GLOBAL_SECTION_NUMBER = 0
	TEXT_OFFSET = $%

end namespace

macro MachO.segment declaration
	MachO.close_segment

	local NAME,VMADDR,VMSIZE,FILEOFF,FILESIZE,MAXPROT,INITPROT,NSECTS

	INITPROT = VM_PROT_NONE
	MAXPROT = VM_PROT_ALL
	match name attributes, declaration
		NAME = name
		local sequence
		define sequence attributes:
		while 1
			match attribute tail, sequence
				match =readable?, attribute
					INITPROT = INITPROT or VM_PROT_READ
					MAXPROT = MAXPROT or VM_PROT_READ
				else match =notreadable?, attribute
					INITPROT = INITPROT and not VM_PROT_READ
					MAXPROT = MAXPROT and not VM_PROT_READ
				else match =writeable?, attribute
					INITPROT = INITPROT or VM_PROT_WRITE
					MAXPROT = MAXPROT or VM_PROT_WRITE
				else match =writable?, attribute
					INITPROT = INITPROT or VM_PROT_WRITE
					MAXPROT = MAXPROT or VM_PROT_WRITE
				else match =notwriteable?, attribute
					INITPROT = INITPROT and not VM_PROT_WRITE
					MAXPROT = MAXPROT and not VM_PROT_WRITE
				else match =notwritable?, attribute
					INITPROT = INITPROT and not VM_PROT_WRITE
					MAXPROT = MAXPROT and not VM_PROT_WRITE
				else match =executable?, attribute
					INITPROT = INITPROT or VM_PROT_EXECUTE
					MAXPROT = MAXPROT or VM_PROT_EXECUTE
				else match =notexecutable?, attribute
					INITPROT = INITPROT and not VM_PROT_EXECUTE
					MAXPROT = MAXPROT and not VM_PROT_EXECUTE
				else
					err 'unknown attribute "',`attribute,'"'
				end match
				redefine sequence tail
			else
				break
			end match
		end while
	else
		NAME = definition
	end match
	if NAME eqtype '' & lengthof NAME > 16
		err 'name too long'
	end if

	VMADDR := MachO.VM_ADDRESS

	if $% = MachO.TEXT_OFFSET & FILESIZE > 0 & MachO.FILETYPE <> MH_OBJECT
		MachO.FILE_OFFSET = 0
	else
		MachO.FILE_OFFSET = $%
	end if

	if FILESIZE
		FILEOFF := MachO.FILE_OFFSET
	else
		FILEOFF := 0
	end if

	MachO.SEGMENT_NUMBER = MachO.SEGMENT_NUMBER + 1
	if MachO.CPUTYPE and CPU_ARCH_ABI64
		MachO.command LC_SEGMENT_64
	else
		MachO.command LC_SEGMENT
	end if
	repeat 1, s:MachO.SEGMENT_NUMBER
		namespace MachO.segment#s
			segname        emit 16: NAME
		if MachO.CPUTYPE and CPU_ARCH_ABI64
			vmaddr		dq	VMADDR
			vmsize		dq	VMSIZE
			fileoff 	dq	FILEOFF
			filesize	dq	FILESIZE
		else
			vmaddr		dd	VMADDR
			vmsize		dd	VMSIZE
			fileoff 	dd	FILEOFF
			filesize	dd	FILESIZE
		end if
			maxprot 	dd	MAXPROT
			initprot	dd	INITPROT
			nsects		dd	NSECTS
			flags		dd	0
		end namespace
		repeat NSECTS
			namespace MachO.segment#s#_section#%
				sectname       emit 16: ?
				segname        emit 16: ?
			if MachO.CPUTYPE and CPU_ARCH_ABI64
				addr	       dq	?
				size	       dq	?
			else
				addr	       dd	?
				size	       dd	?
			end if
				offset	       dd	?
				align.	       dd	?
				reloff	       dd	?
				nreloc	       dd	?
				flags	       dd	?
				reserved1      dd	?
				reserved2      dd	?
			if MachO.CPUTYPE and CPU_ARCH_ABI64
					       dd	?
			end if
			end namespace
		end repeat
	end repeat

	macro MachO.close_segment
		MachO.close_section
		local OFFSET,NEXT,TOP,FILL
		TOP = $%
		NSECTS := MachO.SECTION_NUMBER
		if MachO.FILE_OFFSET >= $%%
			FILL = MachO.FILE_OFFSET
		else
			FILL = $%%
		end if
		repeat 1, s:MachO.SEGMENT_NUMBER
			repeat NSECTS, n:2
				if % = 1
					load OFFSET from MachO:MachO.segment#s#_section#%.offset
				else
					OFFSET = NEXT
				end if
				if % < %%
					load NEXT from MachO:MachO.segment#s#_section#n.offset
				else
					NEXT = TOP
				end if
				if OFFSET >= $%%
					store S_ZEROFILL at MachO:MachO.segment#s#_section#%.flags
					store 0 at MachO:MachO.segment#s#_section#%.size
				else
					FILL = NEXT
				end if
			end repeat
		end repeat
		FILESIZE := FILL - MachO.FILE_OFFSET
		FILL = FILL - $%%
		MachO.SECTION_BYPASS = 1
		section MachO.VM_ADDRESS + FILESIZE - FILL
		restore MachO.SECTION_BYPASS
		db FILL dup 0
		if FILESIZE
			rb (MachO.FILE_ALIGNMENT-$%) and (MachO.FILE_ALIGNMENT-1)
		end if
		MachO.VM_ADDRESS = MachO.VM_ADDRESS + TOP - MachO.FILE_OFFSET
		MachO.VM_ADDRESS = MachO.VM_ADDRESS + (MachO.SEGMENT_ALIGNMENT-MachO.VM_ADDRESS) and (MachO.SEGMENT_ALIGNMENT-1)
		VMSIZE := MachO.VM_ADDRESS - VMADDR
		purge MachO.close_segment
	end macro

	MachO.text
	org VMADDR + $% - MachO.FILE_OFFSET
	MachO.SECTION_NUMBER = 0

end macro

macro MachO.section declaration
	if MachO.SEGMENT_NUMBER = 0
		MachO.segment '__TEXT' readable writable executable
	end if
	MachO.close_section

	local SECTNAME,SEGNAME,FLAGS,ALIGN,RESERVED1,RESERVED2

	FLAGS = S_REGULAR
	ALIGN = 0
	RESERVED1 = 0
	RESERVED2 = 0

	local sequence
	match segname:sectname tail, declaration:
		SECTNAME = sectname
		SEGNAME = segname
		define sequence tail
	else match name tail, declaration:
		SECTNAME = name
		repeat 1, s:MachO.SEGMENT_NUMBER
			load SEGNAME from MachO:MachO.segment#s.segname
		end repeat
		define sequence tail
	end match

	while 1
		match :, sequence
			break
		else match =align? boundary tail, sequence
			ALIGN = bsf boundary
			if bsf boundary <> bsr boundary
				err 'invalid alignment value'
			end if
			redefine sequence tail
		else match =flags?(value) tail, sequence
			FLAGS = value
			redefine sequence tail
		else match =reserved1?(value) tail, sequence
			RESERVED1 = value
			redefine sequence tail
		else match =reserved2?(value) tail, sequence
			RESERVED2 = value
			redefine sequence tail
		else
			err 'invalid arguments'
		end match
	end while

	MachO.text
	if ALIGN
		align 1 shl ALIGN
	end if

	MachO.SECTION_ALIGN = 1 shl ALIGN

	MachO.SECTION_NUMBER = MachO.SECTION_NUMBER + 1
	MachO.GLOBAL_SECTION_NUMBER = MachO.GLOBAL_SECTION_NUMBER + 1

	repeat 1, s:MachO.SEGMENT_NUMBER, t:MachO.SECTION_NUMBER
		if ~ defined MachO.segment#s#_section#t.sectname
			MachO.command
			namespace MachO.segment#s#_section#t
				sectname       emit 16: ?
				segname        emit 16: ?
			if MachO.CPUTYPE and CPU_ARCH_ABI64
				addr	       dq	?
				size	       dq	?
			else
				addr	       dd	?
				size	       dd	?
			end if
				offset	       dd	?
				align.	       dd	?
				reloff	       dd	?
				nreloc	       dd	?
				flags	       dd	?
				reserved1      dd	?
				reserved2      dd	?
			if MachO.CPUTYPE and CPU_ARCH_ABI64
					       dd	?
			end if
			end namespace
			MachO.text
		end if
		store SECTNAME at MachO:MachO.segment#s#_section#t.sectname
		store SEGNAME at MachO:MachO.segment#s#_section#t.segname
		store $ at MachO:MachO.segment#s#_section#t.addr
		store $% at MachO:MachO.segment#s#_section#t.offset
		store FLAGS at MachO:MachO.segment#s#_section#t.flags
		store ALIGN at MachO:MachO.segment#s#_section#t.align
		store RESERVED1 at MachO:MachO.segment#s#_section#t.reserved1
		store RESERVED2 at MachO:MachO.segment#s#_section#t.reserved2
	end repeat

end macro

macro MachO.close_section
	MachO.text
	if MachO.SECTION_NUMBER
		local SIZE
		repeat 1, s:MachO.SEGMENT_NUMBER, t:MachO.SECTION_NUMBER
			load OFFSET from MachO:MachO.segment#s#_section#t.offset
			store $%-OFFSET at MachO:MachO.segment#s#_section#t.size
		end repeat
	end if
end macro

macro MachO.close_segment
end macro

postpone
	MachO.close_segment
	if MachO.FILETYPE <> MH_OBJECT & $%% < 1000h
		store 0:byte at $-1		; enforce minimum file size for OS X 10.10.5 and higher
	end if
end postpone

macro segment? args&
	MachO.segment args
end macro

macro section? args&
	if defined MachO.SECTION_BYPASS
		section args
	else
		MachO.section args
	end if
end macro

macro entry? regs&
	iterate reg, regs
		match name:value, reg
			MachO.thread.name? := value
		else if MachO.CPUTYPE = CPU_TYPE_I386
			MachO.thread.eip? := reg
		else if MachO.CPUTYPE = CPU_TYPE_X86_64
			MachO.thread.rip? := reg
		end if
	end iterate
	MachO.command LC_UNIXTHREAD
	if MachO.CPUTYPE = CPU_TYPE_I386
		MachO.thread.flavor dd x86_THREAD_STATE32
		iterate name, eax,ebx,ecx,edx,edi,esi,ebp,esp,ss,eflags,eip,cs,ds,es,fs,gs
			if % = 1
				MachO.thread.count dd %%
			end if
			if defined MachO.thread.name?
				dd MachO.thread.name?
			else
				dd ?
			end if
		end iterate
	else if MachO.CPUTYPE = CPU_TYPE_X86_64
		MachO.thread.flavor dd x86_THREAD_STATE64
		iterate name, rax,rbx,rcx,rdx,rdi,rsi,rbp,rsp,r8,r9,r10,r11,r12,r13,r14,r15,rip,rflags,cs,fs,gs
			if % = 1
				MachO.thread.count dd %%*2
			end if
			if defined MachO.thread.name?
				dq MachO.thread.name?
			else
				dq ?
			end if
		end iterate
	else
		err 'CPU not supported'
	end if
	MachO.text
end macro

; Upper layer: symbol tables

define macroBuilder?

macro macroBuilder? declaration&
	macro macroBuilder?.definition
		esc macro declaration
	end macro
end macro

macro macroBuilder?.line? content&
	macro macroBuilder?.definition
		macroBuilder?.definition
		content
	end macro
end macro

macro macroBuilder?.end?
	macroBuilder?.definition
	esc end macro
end macro

if MachO.FILETYPE <> MH_OBJECT

	namespace MachO
		NSYMS = 0
		LIB_NUMBER = 0
	end namespace

	macro interpreter? path
		MachO.command LC_LOAD_DYLINKER
		namespace MachO.dylinker_command
			lc_str		dd dylinker-MachO.COMMAND
			dylinker	db path,0
			if MachO.CPUTYPE and CPU_ARCH_ABI64
				align 8
			else
				align 4
			end if
		end namespace
	end macro

	macro uses? lib&
		MachO.LIB_NUMBER = MachO.LIB_NUMBER + 1
		MachO.command LC_LOAD_DYLIB
		repeat 1, l:MachO.LIB_NUMBER
		namespace MachO.dylib#l#_command
			lc_str		dd dylib-MachO.COMMAND
			timestamp	dd 2
			match path (a.b.c=,x.y.z), lib
				current_version 	dd (x and 0FFFFh) shl 16 + y shl 8 + z
				compatibility_version	dd (a and 0FFFFh) shl 16 + b shl 8 + c
				dylib			db path,0
			else
				current_version 	dd 10000h
				compatibility_version	dd 10000h
				dylib			db lib,0
			end match
			if MachO.CPUTYPE and CPU_ARCH_ABI64
				align 8
			else
				align 4
			end if
		end namespace
		end repeat
	end macro

	macro import? definitions&
		iterate <name,string>, definitions
			MachO.NSYMS = MachO.NSYMS + 1
			define MachO.nlist name
			name.imported := 1
			name.type := N_EXT
			name.desc := REFERENCE_FLAG_UNDEFINED_NON_LAZY
			define name.str string
		end iterate
	end macro

	MachO.__TEXT = 0
	MachO.__DATA = 0

	macro segment? args&
		MachO.segment args
		if MachO.NSYMS
			repeat 1, s:MachO.SEGMENT_NUMBER
				load MachO.SEGNAME from MachO:MachO.segment#s.segname
			end repeat
			if ~MachO.__TEXT & MachO.SEGNAME = '__TEXT'
				MachO.__TEXT = 1
				MachO.__stubs
			else if ~MachO.__DATA & MachO.SEGNAME = '__DATA'
				MachO.__DATA = 1
				MachO.__nl_symbol_ptr
			end if
		end if
	end macro

	postpone
	if MachO.NSYMS

		macroBuilder MachO.__stubs

			macroBuilder.line section '__stubs' flags(S_SYMBOL_STUBS+S_ATTR_SOME_INSTRUCTIONS+S_ATTR_PURE_INSTRUCTIONS) reserved1(0) reserved2(MachO.JUMP_SIZE) align 16

			irpv sym, MachO.nlist
				if sym.imported
					macroBuilder.line sym: jmp [sym.ptr]
					if % = 1
						macroBuilder.line MachO.JUMP_SIZE := $ - sym
					end if
				end if
			end irpv

		macroBuilder.end

		macroBuilder MachO.__nl_symbol_ptr

			if MachO.CPUTYPE and CPU_ARCH_ABI64
				macroBuilder.line section '__nl_symbol_ptr' flags(S_NON_LAZY_SYMBOL_POINTERS) reserved1(0) align 8
			else
				macroBuilder.line section '__nl_symbol_ptr' flags(S_NON_LAZY_SYMBOL_POINTERS) reserved1(0) align 4
			end if

			irpv sym, MachO.nlist
				if sym.imported
					if MachO.CPUTYPE and CPU_ARCH_ABI64
						macroBuilder.line sym.ptr dq 0
					else
						macroBuilder.line sym.ptr dd 0
					end if
				end if
			end irpv

		macroBuilder.end

		if ~MachO.__TEXT
			segment '__TEXT' readable executable
		end if

		if ~MachO.__DATA
			segment '__DATA' readable writable
		end if

		segment '__LINKEDIT' readable

			MachO.SYMOFF := $%
			irpv sym, MachO.nlist
				namespace MachO.nlist#%
					n_strx dd sym.strx
					n_type db sym.type
					n_sect db 0
					n_desc dw sym.desc
				if MachO.CPUTYPE and CPU_ARCH_ABI64
					n_value dq sym
				else
					n_value dd sym
				end if
				end namespace
			end irpv

			MachO.INDIRECTSYMOFF := $%
			irpv sym, MachO.nlist
				if sym.imported
					dd %-1
				end if
			end irpv

			MachO.STROFF := $%
			db 20h,0
			irpv sym, MachO.nlist
				sym.strx := $% - MachO.STROFF
				db string sym.str, 0
			end irpv
			MachO.STRSIZE := $% - MachO.STROFF

		MachO.command LC_SYMTAB
		namespace MachO.symtab_command
			symoff	dd SYMOFF
			nsyms	dd NSYMS
			stroff	dd STROFF
			strsize dd STRSIZE
		end namespace

		MachO.command LC_DYSYMTAB
		namespace MachO.dysymtab_command
			ilocalsym	dd 0
			nlocalsym	dd 0
			iextdefsym	dd 0
			nextdefsym	dd 0
			iundefsym	dd 0
			nundefsym	dd NSYMS
			tocoff		dd 0
			ntoc		dd 0
			modtaboff	dd 0
			nmodtab 	dd 0
			extrefsymoff	dd 0
			nextrefsyms	dd 0
			indirectsymoff	dd INDIRECTSYMOFF
			nindirectsyms	dd NSYMS
			extreloff	dd 0
			nextrel 	dd 0
			locreloff	dd 0
			nlocrel 	dd 0
		end namespace

	end if
	end postpone

else

	namespace MachO

		element MachO.section?
		element MachO.symbol?

		VM_ADDRESS = 0

		define nlist null
		null.value := 0
		null.type := 0
		null.sect := 0
		null.desc := 0
		null.str := ''

		NSYMS = 1
		NRELOC = 0

	end namespace

	segment 0 readable writable executable

	macro MachO.section declaration
		if MachO.SECTION_NUMBER
			org 0 scaleof (1 metadataof $) + 0 scaleof $
		end if
		MachO.section declaration
		local sym
		element sym : MachO.GLOBAL_SECTION_NUMBER * MachO.section + $
		org sym
		MachO.SECTION_OFFSET = $%
		MachO.SECTION_REL_INDEX = MachO.NRELOC
		repeat 1, s:MachO.SEGMENT_NUMBER, t:MachO.SECTION_NUMBER
			store MachO.RELOFF+MachO.SECTION_REL_INDEX*8 at MachO:MachO.segment#s#_section#t.reloff
		end repeat
	end macro

	macro MachO.close_section
		MachO.close_section
		if MachO.SECTION_NUMBER
			repeat 1, s:MachO.SEGMENT_NUMBER, t:MachO.SECTION_NUMBER
				store MachO.NRELOC-MachO.SECTION_REL_INDEX at MachO:MachO.segment#s#_section#t.nreloc
			end repeat
		end if
	end macro

	macro public? declaration*
		local name
		define MachO.nlist name
		match value =as? namestr, declaration
			name = value
			define name.str string namestr
		else
			name = declaration
			define name.str `declaration
		end match
		if name relativeto 1 elementof name & 1 elementof (1 metadataof name) eq MachO.section
			name.value := 0 scaleof (1 metadataof name) + 0 scaleof name
			name.type := N_SECT + N_EXT
			name.sect := 1 scaleof (1 metadataof name)
		else
			name.value := name
			name.type := N_ABS
			name.sect := 0
		end if
		name.desc := REFERENCE_FLAG_DEFINED
		MachO.NSYMS = MachO.NSYMS + 1
	end macro

	macro extrn? declaration*
		match namestr =as? sym, declaration
			define MachO.nlist sym
			define sym.str string namestr
		else
			define MachO.nlist declaration
			define declaration.str `declaration
		end match
		match sym, MachO.nlist
			element sym : MachO.NSYMS * MachO.symbol
			sym.type := N_EXT
			sym.sect := 0
			sym.desc := 0
			sym.value := 0
		end match
		MachO.NSYMS = MachO.NSYMS + 1
	end macro

	macro dword? value
		local v, o
		v = value
		if ~ v relativeto 0 & v relativeto 1 elementof v & (1 elementof (1 metadataof v) eq MachO.section | 1 elementof (1 metadataof v) eq MachO.symbol)
			MachO.reloc =: $% - MachO.SECTION_OFFSET + (1 scaleof (1 metadataof v)) shl 32 + 2 shl 57 + GENERIC_RELOC_VANILLA shl 60
			if 1 elementof (1 metadataof v) eq MachO.symbol
				MachO.reloc = MachO.reloc + 1 shl 59
			end if
			o = $%
			emit dword: 0 scaleof (1 metadataof v) + 0 scaleof v
			if $% > o
				MachO.NRELOC = MachO.NRELOC + 1
			else
				restore MachO.reloc
			end if
		else if ~ v relativeto 0 & (v + $) relativeto 1 elementof (v + $) & (1 elementof (1 metadataof (v + $)) eq MachO.section | 1 elementof (1 metadataof (v + $)) eq MachO.symbol)
			if MachO.CPUTYPE = CPU_TYPE_X86_64
				if 1 elementof (1 metadataof (v + $)) eq MachO.symbol
					v = v + $ + 4
				else
					v = v + 0 scaleof (1 metadataof v) - 0 scaleof (1 metadataof $)
				end if
				MachO.reloc =: $% - MachO.SECTION_OFFSET + (1 scaleof (1 metadataof (v + $))) shl 32 + 1 shl 56 + 2 shl 57 + X86_64_RELOC_SIGNED_4 shl 60
			else
				v = v + 0 scaleof (1 metadataof v) - 0 scaleof (1 metadataof $)
				MachO.reloc =: $% - MachO.SECTION_OFFSET + (1 scaleof (1 metadataof (v + $))) shl 32 + 1 shl 56 + 2 shl 57 + GENERIC_RELOC_VANILLA shl 60
			end if
			if 1 elementof (1 metadataof (v + $)) eq MachO.symbol
				MachO.reloc = MachO.reloc + 1 shl 59
			end if
			o = $%
			emit dword: 0 scaleof v
			if $% > o
				MachO.NRELOC = MachO.NRELOC + 1
			else
				restore MachO.reloc
			end if
		else
			emit dword: v
		end if
	end macro

	postpone
	if MachO.NSYMS

		MachO.close_section
		macro MachO.close_section
			MachO.text
		end macro

		rb (-$%) and 11b

		MachO.RELOFF := $%
		irpv rel, MachO.reloc
			dq rel
		end irpv

		MachO.SYMOFF := $%
		irpv sym, MachO.nlist
			namespace MachO.nlist#%
				n_strx dd sym.strx
				n_type db sym.type
				n_sect db sym.sect
				n_desc dw sym.desc
			if MachO.CPUTYPE and CPU_ARCH_ABI64
				n_value dq sym.value
			else
				n_value dd sym.value
			end if
			end namespace
		end irpv

		MachO.STROFF := $%
		irpv sym, MachO.nlist
			sym.strx := $% - MachO.STROFF
			db string sym.str, 0
		end irpv
		MachO.STRSIZE := $% - MachO.STROFF

		MachO.command LC_SYMTAB
		namespace MachO.symtab_command
			symoff	dd SYMOFF
			nsyms	dd NSYMS
			stroff	dd STROFF
			strsize dd STRSIZE
		end namespace

	end if
	end postpone

	calminstruction align? boundary,value:?
		check	$ relativeto 0 | MachO.SECTION_ALIGN mod (boundary) = 0
		jyes	allowed
		arrange value, =err 'section not aligned enough'
		assemble value
		exit
	    allowed:
		compute boundary, (boundary-1)-(0 scaleof $+boundary-1) mod boundary
		arrange value, =db boundary =dup value
		assemble value
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

end if
