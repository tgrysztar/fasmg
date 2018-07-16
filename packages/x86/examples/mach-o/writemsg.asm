
include 'cpu/80386.inc'
use32

MachO.Settings.FileType equ MH_OBJECT
include 'format/macho.inc'

public writemsg

SYS_write = 4

section '__TEXT':'__text'

  writemsg:
	mov	edi,esi
	mov	ecx,-1
	xor	al,al
	repne	scasb
	neg	ecx
	sub	ecx,2
	push	ecx
	push	esi
	push	1
	mov	eax,SYS_write
	sub	esp,4
	int	0x80
	add	esp,4*4
	ret
