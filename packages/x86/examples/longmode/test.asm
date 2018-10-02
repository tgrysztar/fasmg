
	format binary as 'com'
	include 'cpu/x64.inc'

	org	100h

	cli

	push	0
	pop	es

	mov	di,1600h
	mov	si,basecode
	mov	cx,basecode_length
	rep	movsb

	jmp	0:1600h

basecode file 'BASECODE.BIN'
basecode_length = $ - basecode