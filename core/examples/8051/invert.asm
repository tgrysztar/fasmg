
; This simple example reads data from external device through P2
; and outputs it inverted through P1.

include 'hex.inc'
include '8051.inc'

ORG 0
	JMP	setup

ORG 3
	JMP	ext0_interrupt

ORG 0Bh
	JMP	timer0_interrupt

ORG 30h

setup:

	SETB	EX0
	SETB	IT0
	CLR	P0.7

	MOV	TH0,#-40
	MOV	TL0,#-40
	MOV	TMOD,#2

	SETB	TR0
	SETB	ET0
	SETB	EA

	JMP	$

timer0_interrupt:
	CLR	P3.6
	SETB	P3.6
	RETI

ext0_interrupt:
	CLR	P3.7
	MOV	A,P2
	CPL	A
	MOV	P1,A
	SETB	P3.7
	RETI
