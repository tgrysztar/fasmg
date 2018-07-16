
include 'cpu/80386.inc'
use32

MachO.Settings.FileType equ MH_OBJECT
include 'format/macho.inc'

public start
extrn writemsg

SYS_exit = 1

section '__TEXT':'__text'

  start:

	mov	esi,msg
	call	writemsg

	push	1
	mov	eax,SYS_exit
	sub	esp,4
	int	0x80

section '__TEXT':'__cstring'

  msg db "Relocated and ready!",0xA,0
