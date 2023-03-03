

include 'format/format.inc'

format MachO64

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


