
element R

repeat 32 i:0
  element R#i? : R + i
end repeat

XH? = R27
XL? = R26
YH? = R29
YL? = R28
ZH? = R31
ZL? = R30

element X
element Y
element Z

iterate <instr,opcode>, ADC,000111b, ADD,000011b, AND, 001000b, EOR, 001001b, CP,000101b, CPC,000001b, CPSE,000100b, MOV,001011b, MUL,100111b, OR,001010b, SBC,000010b, SUB,000110b
	macro instr? Rd,Rr
		local value,d,r
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			value = +Rr
			if value metadata 1 relativeto R & value eq value element 1
				r = value metadata 1 - R
				dw r and 1111b + d shl 4 + (r shr 4) shl 9 + opcode shl 10
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,opcode>, ADIW,10010110b, SBIW,10010111b
	macro instr? RRd,K
		local value,d
		match Rdh:Rdl,RRd
			value = +Rdl
			if value metadata 1 relativeto R & value eq value element 1
				d = value metadata 1 - R
				value = +Rdh
				if value metadata 1 relativeto R & value eq value element 1
					if value metadata 1 - R = d + 1
						if d = 24 | d = 26 | d = 28 | d = 30
							value = +K
							if value >= 0 & value <= 63
								dw value and 1111b + ((d-24) shr 1) shl 4 + (value shr 4) shl 6 + opcode shl 8
							else
								err 'immediate value out of range'
							end if
						else
							err 'specified register not allowed for this instruction'
						end if
					else
						err 'invalid operand'
					end if
				else
					err 'invalid operand'
				end if
			else
				err 'invalid operand'
			end if
		else match Rd,RRd
			value = +Rd
			if value metadata 1 relativeto R & value eq value element 1
				d = value metadata 1 - R
				if d = 24 | d = 26 | d = 28 | d = 30
					value = +K
					if value >= 0 & value <= 63
						dw value and 1111b + ((d-24) shr 1) shl 4 + (value shr 4) shl 6 + opcode shl 8
					else
						err 'immediate value out of range'
					end if
				else
					err 'specified register not allowed for this instruction'
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand'
		end match
	end macro
end iterate

iterate <instr,opcode>, ANDI,0111b, CPI,0011b, LDI,1110b, ORI,0110b, SBCI,0100b, SBR,0110b, SUBI,0101b
	macro instr? Rd,K
		local value,d
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			if d >= 16
				value = +K
				if value >= -256 & value <= 255
					value = value and 0FFh
					dw value and 1111b + (d-16) shl 4 + (value shr 4) shl 8 + opcode shl 12
				else
					err 'immediate value out of range'
				end if
			else
				err 'specified register not allowed for this instruction'
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,opcode>, ASR,0101b, COM,0000b, DEC,1010b, INC,0011b, LSR,0110b, NEG,0001b, ROR,0111b, SWAP,0010b
	macro instr? Rd
		local value,d
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			dw opcode + d shl 4 + 1001010b shl 9
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,opcode>, BCLR,1001'0100'1000'1000b, BSET,1001'0100'0000'1000b
	macro instr? s
		local value
		value = +s
		if value >= 0 & value <= 7
			dw opcode + value shl 4
		else
			err 'bit index out of range'
		end if
	end macro
end iterate

iterate <instr,opcode>, BLD,1111100b, BST,1111101b, SBRC,1111110b, SBRS,1111111b
	macro instr? Rd,b
		local value,d
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			value = +b
			if value >= 0 & value <= 7
				dw value + d shl 4 + opcode shl 9
			else
				err 'bit index out of range'
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,opcode>, BRBC,111101b, BRBS,111100b
	macro instr? s,k
		local index,offset
		index = +s
		if index >= 0 & index <= 7
			offset = -($ shr 1 + 1) + k
			if offset >= -64 & offset <= 63
				dw index + (offset and 1111111b) shl 3 + opcode shl 10
			else
				err 'relative jump out of range'
			end if
		else
			err 'bit index out of range'
		end if
	end macro
end iterate

macro BRCC? k
	BRBC 0,k
end macro

macro BRCS? k
	BRBS 0,k
end macro

macro BREQ? k
	BRBS 1,k
end macro

macro BRGE? k
	BRBC 4,k
end macro

macro BRHC? k
	BRBC 5,k
end macro

macro BRHS? k
	BRBS 5,k
end macro

macro BRID? k
	BRBC 7,k
end macro

macro BRIE? k
	BRBS 7,k
end macro

macro BRLO? k
	BRBS 0,k
end macro

macro BRLT? k
	BRBS 4,k
end macro

macro BRMI? k
	BRBS 2,k
end macro

macro BRNE? k
	BRBC 1,k
end macro

macro BRPL? k
	BRBC 2,k
end macro

macro BRSH? k
	BRBC 0,k
end macro

macro BRTC? k
	BRBC 6,k
end macro

macro BRTS? k
	BRBS 6,k
end macro

macro BRVC? k
	BRBC 3,k
end macro

macro BRVS? k
	BRBS 3,k
end macro

macro CALL? k
	local offset
	offset = -($ shr 1 + 1) + k
	if offset >= -2048 & offset <= 2047
		dw offset and (1 shl 12 - 1) + 1101b shl 12
	else
		offset = +k
		if offset >= 0 & offset <= 1 shl 22 - 1
			dw (offset shr 16) and 1 + 111b shl 1 + (offset shr 17) shl 4 + 1001010b shl 9
			dw offset and (1 shl 16 - 1)
		else
			err 'value out of range'
		end if
	end if
end macro

iterate <instr,opcode>, CBI,10011000b, SBI,10011010b, SBIC,10011001b, SBIS,10011011b
	macro instr? A,b
		local reg,index
		reg = +A
		if reg >= 0 & reg <= 31
			index = +b
			if index >= 0 & index <= 7
				dw index + reg shl 3 + opcode shl 8
			else
				err 'bit index out of range'
			end if
		else
			err 'specified register number not allowed'
		end if
	end macro
end iterate

macro CBR? r,K
	ANDI r,$FF-(K)
end macro

macro CLC?
	dw 1001'0100'1000'1000b
end macro

macro CLH?
	dw 1001'0100'1101'1000b
end macro

macro CLI?
	dw 1001'0100'1111'1000b
end macro

macro CLN?
	dw 1001'0100'1010'1000b
end macro

macro CLR? r
	EOR r,r
end macro

macro CLS?
	dw 1001'0100'1100'1000b
end macro

macro CLT?
	dw 1001'0100'1110'1000b
end macro

macro CLV?
	dw 1001'0100'1011'1000b
end macro

macro CLZ?
	dw 1001'0100'1001'1000b
end macro

macro DES? K
	local value
	value = +K
	if value >= 0 & value <= 0x0F
		dw 1011b + value shl 4 + 10010100b shl 8
	else
		err 'value out of range'
	end if
end macro

macro EICALL?
	dw 1001'0101'0001'1001b
end macro

macro EIJMP?
	dw 1001'0100'0001'1001b
end macro

iterate <instr,opcode,opcode2,opcode3>, ELPM,1001'0101'1101'1000b,0110b,0111b, LPM,1001'0101'1100'1000b,0100b,0101b
	macro instr? args&
		local value,d
		match , args
			dw opcode
		else match Rd=, =Z?, args
			value = +Rd
			if value metadata 1 relativeto R & value eq value element 1
				d = value metadata 1 - R
				dw opcode2 + d shl 4 + 1001000b shl 9
			else
				err 'invalid operand'
			end if
		else match Rd=, =Z?+, args
			value = +Rd
			if value metadata 1 relativeto R & value eq value element 1
				d = value metadata 1 - R
				dw opcode3 + d shl 4 + 1001000b shl 9
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand'
		end match
	end macro
end iterate

iterate <instr,opcode>, FMUL,1100001000b, FMULS,1110000000b, FMULSU,1110001000b
	macro instr? Rd,Rr
		local value,d,r
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			if d >= 16 & d <= 23
				value = +Rr
				if value metadata 1 relativeto R & value eq value element 1
					r = value metadata 1 - R
					if r >= 16 & r <= 23
						dw opcode + (r-16) + (d-16) shl 4
					else
						err 'specified register not allowed for this instruction'
					end if
				else
					err 'invalid operand'
				end if
			else
				err 'specified register not allowed for this instruction'
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

macro ICALL?
	dw 1001'0101'0000'1001b
end macro

macro IJMP?
	dw 1001'0100'0000'1001b
end macro

macro IN? Rd,A
	local value,d
	value = +Rd
	if value metadata 1 relativeto R & value eq value element 1
		d = value metadata 1 - R
		value = +A
		if A >= 0 & A <= 63
			dw A and 1111b + d shl 4 + (A shr 4) shl 9 + 10110b shl 11
		else
			err 'address out of range'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro JMP? k
	local offset
	offset = -($ shr 1 + 1) + k
	if offset>=-2048 & offset<=2047
		dw offset and 111111111111b + 1100b shl 12
	else
		offset = +k
		if offset>=0 & offset<=1 shl 22 - 1
			dw (offset shr 16) and 1 + 110b shl 1 + (offset shr 17) shl 4 + 1001010b shl 9
			dw offset and (1 shl 16 - 1)
		else
			err 'value out of range'
		end if
	end if
end macro

iterate <instr,opcode>, LAC,0110b, LAS,0101b, LAT,0111b, XCH,0100b
	macro instr? Rw,Rd
		local value,d
		match =Z?, Rw
			value = +Rd
			if value metadata 1 relativeto R & value eq value element 1
				d = value metadata 1 - R
				dw opcode + d shl 4 + 1001001b shl 9
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand'
		end match
	end macro
end iterate

macro LD? Rd,Rw
	local value,d
	value = +Rd
	if value metadata 1 relativeto R & value eq value element 1
		d = value metadata 1 - R
		match =X?, Rw
			dw 1100b + d shl 4 + 1001000b shl 9
		else match =Y?, Rw
			dw 1000b + d shl 4 + 1000000b shl 9
		else match =Z?, Rw
			dw 0000b + d shl 4 + 1000000b shl 9
		else match =X?+, Rw
			dw 1101b + d shl 4 + 1001000b shl 9
		else match =Y?+, Rw
			dw 1001b + d shl 4 + 1001000b shl 9
		else match =Z?+, Rw
			dw 0001b + d shl 4 + 1001000b shl 9
		else match -=X?, Rw
			dw 1110b + d shl 4 + 1001000b shl 9
		else match -=Y?, Rw
			dw 1010b + d shl 4 + 1001000b shl 9
		else match -=Z?, Rw
			dw 0010b + d shl 4 + 1001000b shl 9
		else
			err 'invalid operand'
		end match
	else
		err 'invalid operand'
	end if
end macro

macro LDD? Rd,Rq
	local value,d,q
	value = +Rd
	if value metadata 1 relativeto R & value eq value element 1
		d = value metadata 1 - R
		value = +Rq
		if value relativeto Y
			q = value - Y
			if q >= 0 & q <= 63
				dw q and 111b + 1 shl 3 + d shl 4 + ((q shr 3) and 11b) shl 10 + (q shr 5) shl 13 + 10b shl 14
			else
				err 'value out of range'
			end if
		else if value relativeto Z
			q = value - Z
			if q >= 0 & q <= 63
				dw q and 111b + d shl 4 + ((q shr 3) and 11b) shl 10 + (q shr 5) shl 13 + 10b shl 14
			else
				err 'value out of range'
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro LDS? Rd,k
	local value,d
	value = +Rd
	if value metadata 1 relativeto R & value eq value element 1
		d = value metadata 1 - R
		value = +k
		if value >= 0 & value <= 65535
			dw d shl 4 + 1001000b shl 9
			dw value
		else
			err 'address out of range'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro LSL? r
	ADD r,r
end macro

macro MOVW? args&
	local value,d,r
	match Rdh:Rdl=,Rrh:Rrl, args
		value = +Rdl
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			value = +Rdh
			if value metadata 1 relativeto R & value eq value element 1
				if value metadata 1 - R = d + 1 & d and 1 = 0
					value = +Rrl
					if value metadata 1 relativeto R & value eq value element 1
						r = value metadata 1 - R
						value = +Rrh
						if value metadata 1 relativeto R & value eq value element 1
							if value metadata 1 - R = r + 1 & r and 1 = 0
								dw r shr 1 + (d shr 1) shl 4 + 1 shl 8
							else
								err 'invalid operand'
							end if
						else
							err 'invalid operand'
						end if
					else
						err 'invalid operand'
					end if
				else
					err 'invalid operand'
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand'
		end if
	else match Rd=,Rr,args
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			if d and 1 = 0
				value = +Rr
				if value metadata 1 relativeto R & value eq value element 1
					r = value metadata 1 - R
					if r and 1 = 0
						dw r shr 1 + (d shr 1) shl 4 + 1 shl 8
					else
						err 'invalid operand'
					end if
				else
					err 'invalid operand'
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid operand'
	end match
end macro

iterate <instr,opcode>, MULS,0010b, MULSU,0011b
	macro instr? Rd,Rr
		local value,d,r
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			if d >= 16 & d <= 31
				value = +Rr
				if value metadata 1 relativeto R & value eq value element 1
					r = value metadata 1 - R
					if r >= 16 & r <= 31
						dw (r-16) + (d-16) shl 4 + opcode shl 8
					else
						err 'specified register not allowed for this instruction'
					end if
				else
					err 'invalid operand'
				end if
			else
				err 'specified register not allowed for this instruction'
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

macro NOP?
	dw 0
end macro

macro OUT? A,Rr
	local value,r
	value = +Rr
	if value metadata 1 relativeto R & value eq value element 1
		r = value metadata 1 - R
		value = +A
		if A >= 0 & A <= 63
			dw A and 1111b + r shl 4 + (A shr 4) shl 9 + 10111b shl 11
		else
			err 'address out of range'
		end if
	else
		err 'invalid operand'
	end if
end macro

iterate <instr,opcode>, POP,000b, PUSH,001b
	macro instr? Rd
		local value,d
		value = +Rd
		if value metadata 1 relativeto R & value eq value element 1
			d = value metadata 1 - R
			dw 1111b + d shl 4 + opcode shl 9 + 1001b shl 12
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,opcode>, RCALL,1101b, RJMP,1100b
	macro instr? k
		local offset
		offset = -($ shr 1 + 1) + k
		if offset>=-2048 & offset<=2047
			dw offset and 111111111111b + opcode shl 12
		else
			err 'relative jump out of range'
		end if
	end macro
end iterate

macro RET?
	dw 1001'0101'0000'1000b
end macro

macro RETI?
	dw 1001'0101'0001'1000b
end macro

macro ROL? r
	ADC r,r
end macro

macro SEC?
	dw 1001'0100'0000'1000b
end macro

macro SEH?
	dw 1001'0100'0101'1000b
end macro

macro SEI?
	dw 1001'0100'0111'1000b
end macro

macro SEN?
	dw 1001'0100'0010'1000b
end macro

macro SER? Rd
	local value,d
	value = +Rd
	if value metadata 1 relativeto R & value eq value element 1
		d = value metadata 1 - R
		if d >= 16
			dw 1111b + (d-16) shl 4 + 11101111b shl 8
		else
			err 'specified register not allowed for this instruction'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro SES?
	dw 1001'0100'0100'1000b
end macro

macro SET?
	dw 1001'0100'0110'1000b
end macro

macro SEV?
	dw 1001'0100'0011'1000b
end macro

macro SEZ?
	dw 1001'0100'0001'1000b
end macro

macro SLEEP?
	dw 1001'0101'1000'1000b
end macro

macro SPM? args
	match , args
		dw 1001'0101'1110'1000b
	else match =Z?+,args
		dw 1001'0101'1111'1000b
	else
		err 'invalid operand'
	end match
end macro

macro ST? Rw,Rr
	local value,r
	value = +Rr
	if value metadata 1 relativeto R & value eq value element 1
		r = value metadata 1 - R
		match =X?, Rw
			dw 1100b + r shl 4 + 1001001b shl 9
		else match =Y?, Rw
			dw 1000b + r shl 4 + 1000001b shl 9
		else match =Z?, Rw
			dw 0000b + r shl 4 + 1000001b shl 9
		else match =X?+, Rw
			dw 1101b + r shl 4 + 1001001b shl 9
		else match =Y?+, Rw
			dw 1001b + r shl 4 + 1001001b shl 9
		else match =Z?+, Rw
			dw 0001b + r shl 4 + 1001001b shl 9
		else match -=X?, Rw
			dw 1110b + r shl 4 + 1001001b shl 9
		else match -=Y?, Rw
			dw 1010b + r shl 4 + 1001001b shl 9
		else match -=Z?, Rw
			dw 0010b + r shl 4 + 1001001b shl 9
		else
			err 'invalid operand'
		end match
	else
		err 'invalid operand'
	end if
end macro

macro STD? Rq,Rr
	local value,r,q
	value = +Rr
	if value metadata 1 relativeto R & value eq value element 1
		r = value metadata 1 - R
		value = +Rq
		if value relativeto Y
			q = value - Y
			if q >= 0 & q <= 63
				dw q and 111b + 1 shl 3 + r shl 4 + 1 shl 9 + ((q shr 3) and 11b) shl 10 + (q shr 5) shl 13 + 10b shl 14
			else
				err 'value out of range'
			end if
		else if value relativeto Z
			q = value - Z
			if q >= 0 & q <= 63
				dw q and 111b + r shl 4 + 1 shl 9 + ((q shr 3) and 11b) shl 10 + (q shr 5) shl 13 + 10b shl 14
			else
				err 'value out of range'
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro STS? k,Rr
	local value,r
	value = +Rr
	if value metadata 1 relativeto R & value eq value element 1
		r = value metadata 1 - R
		value = +k
		if value >= 0 & value <= 65535
			dw r shl 4 + 1001001b shl 9
			dw value
		else
			err 'address out of range'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro TST? Rd
	AND Rd,Rd
end macro

macro WDR?
	dw 1001'0101'1010'1000b
end macro

macro BREAK?
	if defined %
		break	; loop control directive
	else
		dw 1001'0101'1001'1000b
	end if
end macro

PC? equ ($ shr 1)

calminstruction (label) ? definition&
	local command
	match : command?, definition
	jno other
	match :: command?, definition
	jyes other
	arrange definition, =LABEL label =AT =$ =shr 1
	assemble definition
	assemble command
	exit
    other:
	arrange definition, label definition
	assemble definition
end calminstruction

if defined SRAM_START
	DSEG?.$ = SRAM_START
else
	DSEG?.$ = 60h
end if

DSEG? = 0

define DIRECTIVE DIRECTIVE

macro __ORG? addr*
	if ~ DSEG?
		org (addr) shl 1
	else
		org addr
	end if
end macro
DIRECTIVE.ORG? equ __ORG

macro __EQU? definition&
	match name == value, definition
		name? = value
	else
		err 'invalid definition'
	end match
end macro
DIRECTIVE.EQU? equ __EQU

macro __DSEG?
	if ~ DSEG?
		virtual at DSEG?.$
		DSEG? = 1
	end if
end macro
DIRECTIVE.DSEG? equ __DSEG

macro __CSEG?
	if DSEG?
		DSEG?.$ = $
		end virtual
		DSEG? = 0
	end if
end macro
DIRECTIVE.CSEG? equ __CSEG

calminstruction ? line&
	local command
	match .command, line
	jno pass
	transform command, DIRECTIVE
	jno pass
	assemble command
	exit
    pass:
	assemble line
end calminstruction
