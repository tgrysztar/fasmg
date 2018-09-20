
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

macro Elf64_Sym name:0,value:0,size:0,bind:0,type:0,other:0,shndx:0
	dd name
	db bind shl 4 + type
	db other
	dw shndx
	dq value
	dq size
end macro

virtual at 0
	Elf64_Sym
	sizeof.Elf64_Sym = $
end virtual

macro Elf64_Rel offset:0,symbol:0,type:0
	dq offset
	dq symbol shl 32 + type
end macro

virtual at 0
	Elf64_Rel
	sizeof.Elf64_Rel = $
end virtual

macro Elf64_Rela offset:0,symbol:0,type:0,addend:0
	dq offset
	dq symbol shl 32 + type
	dq addend
end macro

virtual at 0
	Elf64_Rela
	sizeof.Elf64_Rela = $
end virtual