
include 'elfsym.inc'

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
	irp name, definitions
		Elf32_Sym strtab.name-strtab,0,0,STB_GLOBAL,STT_FUNC,0,0
		count = count+1
	end irp
	rel:
	irp name, definitions
		Elf32_Rel name,%,R_386_32
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
	irp name, definitions
		strtab.name db `name,0
	end irp
	irpv library, needed@dynamic
		strtab.needed#% db library,0
	end irpv
	strsz = $-strtab
	ptrtab:
	irp name, definitions
		?name dq 0
	end irp
end macro
