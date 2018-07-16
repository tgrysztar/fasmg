
include '8086.inc'

	org	100h

display_text = 9

	mov	ah,display_text
	mov	dx,hello
	int	21h

	int	20h

hello db 'Hello world!',24h
