
ELF.Settings.Class = ELFCLASS64
ELF.Settings.Type = ET_DYN
ELF.Settings.Machine = EM_AARCH64
ELF.Settings.LoadHeaders = 1
include '../x86/include/format/elfexe.inc'

include 'iset/aarch64.inc'

segment readable executable

	entry $

		mov	x8,64		; sys_write
		mov	x0,1
		adr	x1,msg
		mov	x2,msg.length
		svc	0

		mov	x8,93		; sys_exit
		mov	x0,0
		svc	0

segment readable writeable

	msg db "Hello, world of ARM64 assembly!",0xA
	.length = $ - .
