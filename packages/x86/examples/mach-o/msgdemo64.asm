
include 'cpu/x64.inc'
use64

MachO.Settings.FileType equ MH_OBJECT
MachO.Settings.ProcessorType equ CPU_TYPE_X86_64
MachO.Settings.ProcessorSubtype equ CPU_SUBTYPE_X86_64_ALL
include 'format/macho.inc'

section '__TEXT':'__text'

public start
extrn writemsg
extrn exit

start:
	lea	rsi,[msg]
	call	writemsg
	call	exit

section '__DATA':'__data'

	msg db 'Relocated and ready!',0Ah,0


