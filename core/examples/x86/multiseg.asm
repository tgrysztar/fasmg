
include '8086.inc'
include 'format/mz.inc'

entry code:start			; program entry point
stack 100h				; stack size

segment code

  start:
	mov	ax,data
	mov	ds,ax

	mov	dx,hello
	call	extra:write_text

	mov	ax,4C00h
	int	21h

segment data

  hello db 'Hello world!',24h

segment extra

  write_text:
	mov	ah,9
	int	21h
	retf
