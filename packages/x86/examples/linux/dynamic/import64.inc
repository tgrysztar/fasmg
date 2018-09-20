
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
		dq DT_NEEDED,strtab.needed#%-strtab
	end irpv
	dq DT_STRTAB,strtab
	dq DT_STRSZ,strsz
	dq DT_SYMTAB,symtab
	dq DT_SYMENT,sizeof.Elf64_Sym
	dq DT_RELA,rela
	dq DT_RELASZ,relasz
	dq DT_RELAENT,sizeof.Elf64_Rela
	dq DT_HASH,hash
	dq DT_NULL,0
	segment readable writeable
	symtab: Elf64_Sym
	local count
	count = 0
	irp name, definitions
		Elf64_Sym strtab.name-strtab,0,0,STB_GLOBAL,STT_FUNC,0,0
		count = count+1
	end irp
	rela:
	irp name, definitions
		Elf64_Rela name,%,R_X86_64_64
	end irp
	relasz = $-rela
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
