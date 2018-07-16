
macro ccall proc*,[arg]
{
    common
	local size,count
	mov	ebp,esp
	if size
		sub	esp,size
	end if
	and	esp,0FFFFFFF0h
	count = 0
	if ~ arg eq
    forward
	mov	dword [esp+count*4],arg
	count = count + 1
    common
	end if
	size = count*4
	call	proc
	mov	esp,ebp
}

macro cinvoke proc*,arg& { ccall [proc],arg }

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

macro Elf32_Sym name,value,size,bind,type,other,shndx
{
 dd name+0
 dd value+0
 dd size+0
 db (bind+0) shl 4 + (type+0)
 db other+0
 dw shndx+0
}

virtual at 0
 Elf32_Sym
 sizeof.Elf32_Sym = $
end virtual

macro Elf32_Rel offset,symbol,type
{
  dd offset+0
  dd (symbol+0) shl 8 + (type+0)
}

virtual at 0
 Elf32_Rel
 sizeof.Elf32_Rel = $
end virtual

macro Elf32_Rela offset,symbol,type,addend
{
  dd offset+0
  dd (symbol+0) shl 8 + (type+0)
  dd addend+0
}

virtual at 0
 Elf32_Rela
 sizeof.Elf32_Rela = $
end virtual

macro interpreter [library]
{
 segment interpreter readable
 db library,0
}

macro needed [library]
{
 local str
 match needed,needed@dynamic \{ define needed@dynamic needed,str:library \}
 match ,needed@dynamic \{ define needed@dynamic str:library \}
}
define needed@dynamic

macro import [name,string]
{
 common
  local strtab,strsz,symtab,rel,relsz,hash
  segment dynamic readable
  match needed,needed@dynamic
  \{ irp item,needed \\{ match str:library,item \\\{ dd DT_NEEDED,str-strtab \\\} \\} \}
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
 forward
  local fstr
  Elf32_Sym fstr-strtab,0,0,STB_GLOBAL,STT_FUNC,0,0
 common
  rel:
  local counter
  counter = 1
 forward
   Elf32_Rel name,counter,R_386_32
   counter = counter+1
 common
  relsz = $-rel
  hash:
  dd 1,counter
  dd 0
  repeat counter
   if %=counter
    dd 0
   else
    dd %
   end if
  end repeat
  strtab db 0
 forward
  fstr db string,0
 common
  match needed,needed@dynamic
  \{ irp item,needed \\{ match str:library,item \\\{ str db library,0 \\\} \\} \}
  strsz = $-strtab
 forward
  name dd 0
}
