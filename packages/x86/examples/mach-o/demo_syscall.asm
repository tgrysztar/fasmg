
include 'cpu/80386.inc'
use32

MachO.Settings.BaseAddress = 0x1000

include 'format/macho.inc'

segment '__TEXT' readable executable

  section '__text'

  entry $

	push	msg.length
	push	msg
	push	1
	mov	eax,4
	sub	esp,4
	int	0x80
	add	esp,4*4
	push	0
	mov	eax,1
	sub	esp,4
	int	0x80

  section '__cstring'

	msg db 'This is a simple 32-bit Mach-O executable using only syscalls.',0Ah
	.length = $ - msg



