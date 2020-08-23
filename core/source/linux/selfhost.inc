
include '../../examples/x86/include/80386.inc'

macro format?.ELF? variant
	match , variant
		format binary as 'o'
		include '../../examples/x86/include/format/elf32.inc'
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
		include '../../examples/x86/include/format/elfexe.inc'
		use32
	else
		err 'invalid argument'
	end match
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

macro cinvoke? proc*,args&
	local size
	mov	ebp,esp
	if size
		sub	esp,size
	end if
	and	esp,0FFFFFFF0h
	match any, args
		iterate arg, args
			mov	dword [esp+(%-1)*4],arg
			if % = 1
				size := %%*4
			end if
		end iterate
	else
		size := 0
	end match
	call	[proc]
	mov	esp,ebp
end macro

DT_NULL    = 0
DT_NEEDED  = 1
DT_HASH    = 4
DT_STRTAB  = 5
DT_SYMTAB  = 6
DT_RELA    = 7
DT_RELASZ  = 8
DT_RELAENT = 9
DT_STRSZ   = 10
DT_SYMENT  = 11
DT_REL	   = 17
DT_RELSZ   = 18
DT_RELENT  = 19

STB_LOCAL  = 0
STB_GLOBAL = 1
STB_WEAK   = 2

STT_NOTYPE  = 0
STT_OBJECT  = 1
STT_FUNC    = 2
STT_SECTION = 3
STT_FILE    = 4

R_386_NONE     = 0
R_386_32       = 1
R_386_PC32     = 2
R_386_GOT32    = 3
R_386_PLT32    = 4
R_386_COPY     = 5
R_386_GLOB_DAT = 6
R_386_JMP_SLOT = 7
R_386_RELATIVE = 8
R_386_GOTOFF   = 9
R_386_GOTPC    = 10

macro Elf32_Sym name:0,value:0,size:0,bind:0,type:0,other:0,shndx:0
	dd name
	dd value
	dd size
	db bind shl 4 + type
	db other
	dw shndx
end macro

virtual at 0
	Elf32_Sym
	sizeof.Elf32_Sym = $
end virtual

macro Elf32_Rel offset:0,symbol:0,type:0
	dd offset
	dd symbol shl 8 + type
end macro

virtual at 0
	Elf32_Rel
	sizeof.Elf32_Rel = $
end virtual

macro Elf32_Rela offset:0,symbol:0,type:0,addend:0
	dd offset
	dd symbol shl 8 + type
	dd addend
end macro

virtual at 0
	Elf32_Rela
	sizeof.Elf32_Rela = $
end virtual

macro interpreter library
	segment interpreter readable
	db library,0
end macro

macro needed libraries&
	irp library, libraries
		define needed@dynamic library
	end irp
end macro

macro import definitions&
	local strtab,strsz,symtab,rel,relsz,hash
	segment dynamic readable
	irpv library, needed@dynamic
		dd DT_NEEDED,strtab.needed#%-strtab
	end irpv
	dd DT_STRTAB,strtab
	dd DT_STRSZ,strsz
	dd DT_SYMTAB,symtab
	dd DT_SYMENT,sizeof.Elf32_Sym
	dd DT_REL,rel
	dd DT_RELSZ,relsz
	dd DT_RELENT,sizeof.Elf32_Rel
	dd DT_HASH,hash
	dd DT_NULL,0
	segment readable writeable
	symtab: Elf32_Sym
	local count
	count = 0
	irp <label,string>, definitions
		Elf32_Sym strtab.label-strtab,0,0,STB_GLOBAL,STT_FUNC,0,0
		count = count+1
	end irp
	rel:
	irp <label,string>, definitions
		Elf32_Rel label,%,R_386_32
	end irp
	relsz = $-rel
	hash:
	dd 1,count+1
	dd 0
	repeat count
		dd %
	end repeat
	dd 0
	strtab db 0
	irp <label,string>, definitions
		strtab.label db string,0
	end irp
	irpv library, needed@dynamic
		strtab.needed#% db library,0
	end irpv
	strsz = $-strtab
	irp <label,string>, definitions
		label dd 0
	end irp
end macro
